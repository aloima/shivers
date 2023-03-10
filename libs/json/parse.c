#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <json.h>
#include <utils.h>

static void parse_v(JSONElement **element, char *text, size_t length, size_t *i);
static void parse_kv(JSONElement **parent, JSONElement **element, char *text, size_t length, size_t *i);

static void parse_v(JSONElement **element, char *text, size_t length, size_t *i) {
	size_t value_length = 0;

	while (*i < length) {
		char ch = text[*i];

		if ((*element)->type == JSON_UNSPECIFIED) {
			bool is_true = (strncmp(text + *i, "true", 4) == 0);
			bool is_false = (strncmp(text + *i, "false", 5) == 0);
			bool is_null = (strncmp(text + *i, "null", 4) == 0);

			if (ch == ' ' || ch == '\t') {
				++(*i);
				continue;
			} else if (ch == '"') {
				(*element)->type = JSON_STRING;
			} else if (ch == '{') {
				bool condition;

				(*element)->type = JSON_OBJECT;

				condition = (*i < length);
				++(*i);

				while (condition) {
					JSONElement *sub_element = NULL;
					bool sub_condition = true;

					condition = *i < length;
					ch = text[*i];

					++(*element)->size;
					(*element)->value = allocate((*element)->value, (*element)->size, sizeof(JSONElement));
					parse_kv(element, &sub_element, text, length, i);
					((JSONElement **) (*element)->value)[(*element)->size - 1] = sub_element;
					++(*i);

					while (sub_condition) {
						char sub_ch = text[*i - 1];

						if (sub_ch == ' ' || sub_ch == '\t') {
							++(*i);
						} else if (sub_ch == ',') {
							sub_condition = false;
						} else if (sub_ch == '}') {
							sub_condition = false;
							condition = false;
						} else {
							fprintf(stderr, "json_parse(): missing ending of object or comma\n");
							json_free(*element);
							break;
						}
					}
				}

				break;
			} else if (ch == '[') {
				bool condition;

				(*element)->type = JSON_ARRAY;
				condition = (*i < length);
				++(*i);

				while (condition) {
					JSONElement *sub_element = allocate(NULL, 1, sizeof(JSONElement));
					bool sub_condition = true;

					sub_element->parent = *element;
					condition = *i < length;
					ch = text[*i];

					++(*element)->size;
					(*element)->value = allocate((*element)->value, (*element)->size, sizeof(JSONElement));
					parse_v(&sub_element, text, length, i);
					((JSONElement **) (*element)->value)[(*element)->size - 1] = sub_element;
					++(*i);

					while (sub_condition) {
						char sub_ch = text[*i - 1];

						if (sub_ch == ' ' || sub_ch == '\t') {
							++(*i);
						} else if (sub_ch == ',') {
							sub_condition = false;
						} else if (sub_ch == ']') {
							sub_condition = false;
							condition = false;
						} else {
							fprintf(stderr, "json_parse(): missing ending of array or comma\n");
							json_free(*element);
							break;
						}
					}
				}

				break;
			} else if (isdigit(ch)) {
				(*element)->type = JSON_NUMBER;
				(*element)->value = allocate((*element)->value, 1, sizeof(long));
				((long *) (*element)->value)[0] = ch - 48;
			} else if (is_true || is_false) {
				(*element)->type = JSON_BOOLEAN;

				if (is_true) {
					*i += 4;
					(*element)->value = allocate((*element)->value, 1, sizeof(bool));
					((bool *) (*element)->value)[0] = 1;
					break;
				} else if (is_false) {
					*i += 5;
					(*element)->value = allocate((*element)->value, 1, sizeof(bool));
					((bool *) (*element)->value)[0] = 0;
					break;
				}
			} else if (is_null) {
				*i += 4;
				(*element)->type = JSON_NULL;
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
			} else if ((*element)->type == JSON_NUMBER) {
				if (isdigit(ch)) {
					((long *) (*element)->value)[0] = (long) ((((long *) (*element)->value)[0] * 10) + (ch - 48));
				} else {
					break;
				}
			}
		}

		++(*i);
	}
}

static void parse_kv(JSONElement **parent, JSONElement **element, char *text, size_t length, size_t *i) {
	size_t key_length = 0;
	bool parsing_key = false;
	bool parsing_value = false;

	*element = allocate(*element, 1, sizeof(JSONElement));
	(*element)->parent = parent;

	while (*i < length) {
		char ch = text[*i];

		if (!parsing_key && !parsing_value && (ch == ' ' || ch == '\t')) {
			++(*i);
			continue;
		} else if (!parsing_key && (*element)->key == NULL) {
			if (ch == '"') {
				parsing_key = true;
			} else {
				fprintf(stderr, "json_parse(): invalid starting of key\n");
				json_free(*element);
				break;
			}
		} else if (parsing_key) {
			if (ch == '"') {
				parsing_key = false;
			} else {
				++key_length;
				(*element)->key = allocate((*element)->key, key_length + 1, sizeof(char));
				strncat((*element)->key, &ch, 1);
			}
		} else {
			if (ch == ' ' || ch == '\t') {
				++(*i);
				continue;
			} else if (ch == ':') {
				parsing_value = true;
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
