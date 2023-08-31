#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include <json.h>
#include <utils.h>

static void parse_v(jsonelement_t *element, const char *text, size_t length, size_t *i);
static void parse_kv(jsonelement_t *parent, jsonelement_t **element, const char *text, size_t length, size_t *i);

static void parse_v(jsonelement_t *element, const char *text, size_t length, size_t *i) {
	size_t value_length = 0;
	bool escaping = false;

	while (*i < length) {
		char ch = text[*i];

		if (element->type == JSON_UNSPECIFIED) {
			bool is_true = (strncmp(text + *i, "true", 4) == 0);
			bool is_false = (strncmp(text + *i, "false", 5) == 0);
			bool is_null = (strncmp(text + *i, "null", 4) == 0);

			if (ch == ' ' || ch == '\t' || ch == '\n') {
				++(*i);
				continue;
			} else if (ch == '"') {
				element->type = JSON_STRING;
			} else if (ch == '{') {
				++(*i);
				element->type = JSON_OBJECT;
				bool condition = (*i < length);

				while (condition) {
					jsonelement_t *sub_element = NULL;
					bool sub_condition = true;

					condition = (*i < length);

					parse_kv(element, &sub_element, text, length, i);
					++(*i);

					while (sub_condition) {
						char sub_ch = text[*i - 1];

						if (sub_ch == ' ' || sub_ch == '\t' || sub_ch == '\n') {
							++(*i);
						} else if (sub_ch == ',') {
							sub_condition = false;
						} else if (sub_ch == '}') {
							sub_condition = false;
							condition = false;
						} else {
							json_free(element);
							throw("json_parse(): expected value ',' or '}', but received '%c'", sub_ch);
						}
					}
				}

				break;
			} else if (ch == '[') {
				++(*i);
				element->type = JSON_ARRAY;
				bool condition = (*i < length);
				bool waiting_new_element = false;

				while (condition) {
					jsonelement_t *sub_element = allocate(NULL, 0, 1, sizeof(jsonelement_t));
					bool sub_condition = true;

					sub_element->parent = element;
					condition = (*i < length);

					if (text[*i] != ']') {
						++element->size;
						element->value = allocate(element->value, element->size - 1, element->size, sizeof(jsonelement_t));
						((jsonelement_t **) element->value)[element->size - 1] = sub_element;
						parse_v(sub_element, text, length, i);
					} else {
						free(sub_element);
						condition = false;

						if (waiting_new_element) {
							json_free(element);
							throw("json_parse(): expected element, but received ']'");
						}

						++(*i);
						break;
					}

					++(*i);

					while (sub_condition) {
						char sub_ch = text[*i - 1];

						if (sub_ch == ' ' || sub_ch == '\t' || sub_ch == '\n') {
							++(*i);
						} else if (sub_ch == ',') {
							waiting_new_element = true;
							sub_condition = false;
						} else if (sub_ch == ']') {
							sub_condition = false;
							condition = false;
						} else {
							json_free(sub_element);
							throw("json_parse(): expected value ',' or ']', but received '%c'", sub_ch);
						}
					}
				}

				break;
			} else if (isdigit(ch)) {
				element->type = JSON_NUMBER;
				element->size = 1;
				element->value = allocate(element->value, 0, 1, sizeof(long));
				((long *) element->value)[0] = (ch - 48);
			} else if (is_true || is_false) {
				element->type = JSON_BOOLEAN;

				if (is_true) {
					*i += 4;
					element->value = allocate(element->value, 0, 1, sizeof(bool));
					((bool *) element->value)[0] = true;
					break;
				} else if (is_false) {
					*i += 5;
					element->value = allocate(element->value, 0, 1, sizeof(bool));
					((bool *) element->value)[0] = false;
					break;
				}
			} else if (is_null) {
				*i += 4;
				element->type = JSON_NULL;
				break;
			} else {
				json_free(element);
				throw("json_parse(): expected element, but received '%c'", ch);
			}
		} else {
			if (element->type == JSON_STRING) {
				if (ch != '"' || escaping) {
					++value_length;
					element->value = allocate(element->value, value_length, value_length + 1, sizeof(char));
					strncat(element->value, &ch, 1);

					if (ch == '\\') {
						escaping = true;
					} else {
						escaping = false;
					}
				} else {
					++(*i);
					break;
				}
			} else if (element->type == JSON_NUMBER) {
				if (isdigit(ch)) {
					((long *) element->value)[0] = ((((long *) element->value)[0] * 10) + (ch - 48));
				} else if (element->size != 2 && ch == '.') {
					element->size = 2;
					element->value = allocate(element->value, 1, 2, sizeof(long));
					++(*i);

					bool condition = (*i < length);

					while (condition) {
						ch = text[*i];

						if (isdigit(ch)) {
							++(*i);
							((long *) element->value)[1] = (long) ((((long *) element->value)[1] * 10) + (ch - 48));
							condition = (*i < length);
						} else {
							condition = false;
						}
					}

					break;
				} else {
					break;
				}
			}
		}

		++(*i);
	}
}

static void parse_kv(jsonelement_t *parent, jsonelement_t **element, const char *text, size_t length, size_t *i) {
	size_t key_length = 0;
	bool parsing_key = false;
	bool parsing_value = false;

	*element = allocate(NULL, 0, 1, sizeof(jsonelement_t));
	(*element)->parent = parent;

	while (*i < length) {
		char ch = text[*i];

		if (!parsing_key && !parsing_value && (ch == ' ' || ch == '\t' || ch == '\n')) {
			++(*i);
			continue;
		} else if (!parsing_key && (*element)->key == NULL) {
			if (ch == '"') {
				parsing_key = true;
			} else if (ch == '}') {
				free(*element);
				break;
			} else {
				free(*element);
				json_free(parent);
				throw("json_parse(): expected value '\"', but received '%c'", ch);
			}
		} else if (parsing_key) {
			if (ch == '"') {
				parsing_key = false;
			} else {
				++key_length;
				(*element)->key = allocate((*element)->key, key_length, key_length + 1, sizeof(char));
				strncat((*element)->key, &ch, 1);
			}
		} else {
			if (ch == ' ' || ch == '\t' || ch == '\n') {
				++(*i);
				continue;
			} else if (ch == ':') {
				parsing_value = true;
			} else if (parsing_value) {
				++parent->size;
				parent->value = allocate(parent->value, parent->size - 1, parent->size, sizeof(jsonelement_t));
				((jsonelement_t **) parent->value)[parent->size - 1] = *element;
				parse_v(*element, text, length, i);
				break;
			} else {
				free((*element)->key);
				json_free(parent);
				throw("json_parse(): expected value ':', but received '%c'", ch);
			}
		}

		++(*i);
	}
}

jsonelement_t *json_parse(const char *text) {
	jsonelement_t *result = allocate(NULL, 0, 1, sizeof(jsonelement_t));
	size_t length = strlen(text);
	size_t i = 0;

	parse_v(result, text, length, &i);

	return result;
}
