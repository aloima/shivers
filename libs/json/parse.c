#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <json.h>
#include <utils.h>

static void parse_v(jsonelement_t *element, const char *text, const unsigned int length, unsigned int *i);
static void parse_kv(jsonelement_t *parent, jsonelement_t **element, const char *text, const unsigned int length, unsigned int *i);

#define IS_EMPTY(ch) ((ch) == ' ' || (ch) == '\n' || (ch) == '\t' || (ch) == 13)

static void parse_v(jsonelement_t *element, const char *text, const unsigned int length, unsigned int *i) {
  bool escaping = false;

  while (*i < length) {
    char ch = text[*i];

    if (element->type == JSON_UNSPECIFIED) {
      const bool is_true = (memcmp(text + *i, "true", 4) == 0);
      const bool is_false = (memcmp(text + *i, "false", 5) == 0);
      const bool is_null = (memcmp(text + *i, "null", 4) == 0);

      if (is_true || is_false) {
        element->type = JSON_BOOLEAN;
        element->value = allocate(element->value, -1, 1, sizeof(bool));

        if (is_true) {
          *i += 4;
          ((bool *) element->value)[0] = true;
        } else if (is_false) {
          *i += 5;
          ((bool *) element->value)[0] = false;
        }

        break;
      } else if (is_null) {
        *i += 4;
        element->type = JSON_NULL;
        break;
      } else if (IS_EMPTY(ch)) {
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

            if (IS_EMPTY(sub_ch)) {
              ++(*i);
            } else if (sub_ch == ',') {
              sub_condition = false;
            } else if (sub_ch == '}') {
              sub_condition = false;
              condition = false;
            } else {
              json_free(element, true);
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
            element->value = allocate(element->value, -1, element->size, sizeof(jsonelement_t));
            ((jsonelement_t **) element->value)[element->size - 1] = sub_element;
            parse_v(sub_element, text, length, i);
          } else {
            free(sub_element);
            condition = false;

            if (waiting_new_element) {
              json_free(element, true);
              throw("json_parse(): expected element, but received ']'");
            }

            ++(*i);
            break;
          }

          ++(*i);

          while (sub_condition) {
            const char sub_ch = text[*i - 1];

            if (IS_EMPTY(sub_ch)) {
              ++(*i);
            } else if (sub_ch == ',') {
              waiting_new_element = true;
              sub_condition = false;
            } else if (sub_ch == ']') {
              sub_condition = false;
              condition = false;
            } else {
              json_free(sub_element, true);
              throw("json_parse(): expected value ',' or ']', but received '%c'", sub_ch);
            }
          }
        }

        break;
      } else if (isdigit(ch)) {
        element->type = JSON_NUMBER;
        element->size = 1;
        element->value = allocate(element->value, -1, 1, sizeof(double));
        ((double *) element->value)[0] = (ch - 48);
      } else {
        json_free(element, true);
        throw("json_parse(): expected element, but received '%c'", ch);
      }
    } else {
      if (element->type == JSON_STRING) {
        if (ch != '"' || escaping) {
          ++element->size;
          element->value = allocate(element->value, element->size - 1, element->size + 1, sizeof(char));
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
          ((double *) element->value)[0] = ((((double *) element->value)[0] * 10) + (ch - 48));
        } else if (ch == '.') {
          ++(*i);
          unsigned int starts_at = *i;

          bool condition = (*i < length);

          while (condition) {
            ch = text[*i];

            if (isdigit(ch)) {
              ++(*i);
              ((double *) element->value)[0] += (ch - 48) / pow(10, *i - starts_at + 1);
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

static void parse_kv(jsonelement_t *parent, jsonelement_t **element, const char *text, const unsigned int length, unsigned int *i) {
  unsigned int key_length = 0;
  bool parsing_key = false;
  bool parsing_value = false;

  *element = allocate(NULL, 0, 1, sizeof(jsonelement_t));
  (*element)->parent = parent;

  while (*i < length) {
    const char ch = text[*i];

    if (!parsing_key && !parsing_value && IS_EMPTY(ch)) {
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
        json_free(parent, true);
        throw("json_parse(): expected value '\"', but received '%c'", ch);
      }
    } else if (parsing_key) {
      if (ch == '"') {
        parsing_key = false;
      } else {
        ++key_length;
        (*element)->key = allocate((*element)->key, key_length - 1, key_length + 1, sizeof(char));
        strncat((*element)->key, &ch, 1);
      }
    } else {
      if (IS_EMPTY(ch)) {
        ++(*i);
        continue;
      } else if (ch == ':') {
        parsing_value = true;
      } else if (parsing_value) {
        ++parent->size;
        parent->value = allocate(parent->value, -1, parent->size, sizeof(jsonelement_t));
        ((jsonelement_t **) parent->value)[parent->size - 1] = *element;
        parse_v(*element, text, length, i);
        break;
      } else {
        free((*element)->key);
        json_free(parent, true);
        throw("json_parse(): expected value ':', but received '%c'", ch);
      }
    }

    ++(*i);
  }
}

jsonelement_t *json_parse(const char *text) {
  jsonelement_t *result = allocate(NULL, 0, 1, sizeof(jsonelement_t));
  const unsigned int length = strlen(text);
  unsigned int i = 0;

  parse_v(result, text, length, &i);
  return result;
}
