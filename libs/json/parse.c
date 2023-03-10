#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <json.h>
#include <utils.h>

static void parse_v(JSONElement **element, char *text, size_t length, size_t *i);
static void parse_kv(JSONElement **parent, JSONElement **element, char *text, size_t length, size_t *i);

static void parse_v(JSONElement **element, char *text, size_t length, size_t *i) {
	size_t value_length = 0;
	bool_t parsing = 0;
	bool_t condition;

	while (*i < length) {
		char ch = text[*i];

		if (!parsing) {
			if (ch == '"') {
				parsing = 1;
				(*element)->type = JSON_STRING;
			} else if (ch == '{') {
				parsing = 1;
				(*element)->type = JSON_OBJECT;

				condition = *i < length;
				++(*i);

				while (condition) {
					JSONElement *sub_element = NULL;

					condition = *i < length;
					ch = text[*i];

					++(*element)->size;
					(*element)->value = allocate((*element)->value, (*element)->size, sizeof(JSONElement));
					parse_kv(element, &sub_element, text, length, i);
					((JSONElement **) (*element)->value)[(*element)->size - 1] = sub_element;
					++(*i);

					if (text[*i - 1] == '}') {
						condition = 0;
					} else if (text[*i - 1] != ',') {
						fprintf(stderr, "json_parse(): missing ending of object or comma\n");
						json_free(*element);
						break;
					}
				}

				(*element)->type = JSON_OBJECT;
				break;
			} else {
				fprintf(stderr, "json_parse(): invalid value\n");
				json_free(*element);
				break;
			}
		} else {
			if ((*element)->type == JSON_STRING) {
				if (ch != '"') {
					++value_length;
					(*element)->value = allocate((*element)->value, value_length + 1, sizeof(char));
					strncat((*element)->value, &ch, 1);
				} else {
					++(*i);
					break;
				}
			}
		}

		++(*i);
	}
}

static void parse_kv(JSONElement **parent, JSONElement **element, char *text, size_t length, size_t *i) {
	size_t key_length = 0;
	bool_t parsing_key = 0;
	bool_t parsing_value = 0;

	*element = allocate(*element, 1, sizeof(JSONElement));
	(*element)->parent = parent;

	while (*i < length) {
		char ch = text[*i];

		if (!parsing_key && !parsing_value && (ch == ' ' || ch == '\t')) {
			continue;
		} else if (!parsing_key && (*element)->key == NULL) {
			if (ch == '"') {
				parsing_key = 1;
			} else {
				fprintf(stderr, "json_parse(): invalid starting of key\n");
				json_free(*element);
				break;
			}
		} else if (parsing_key) {
			if (ch == '"') {
				parsing_key = 0;
			} else {
				++key_length;
				(*element)->key = allocate((*element)->key, key_length + 1, sizeof(char));
				strncat((*element)->key, &ch, 1);
			}
		} else {
			if (ch == ':') {
				parsing_value = 1;
			} else if (parsing_value) {
				parse_v(element, text, length, i);
				break;
			} else {
				fprintf(stderr, "json_parse(): missing colons\n");
				json_free(*element);
				break;
			}
		}

		++(*i);
	}
}

JSONElement *json_parse(char *text) {
	JSONElement *result = allocate(NULL, 1, sizeof(JSONElement));
	size_t length = strlen(text);
	size_t i = 0;

	parse_v(&result, text, length, &i);

	if (i != length) {
		fprintf(stderr, "json_parse(): invalid ending of object\n");
	}

	return result;
}
