#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <json.h>
#include <utils.h>

char *json_stringify(const jsonelement_t *element, const unsigned int fractional_limit) {
  char *result = NULL;

  if (!element || element->type == JSON_UNSPECIFIED) {
    result = allocate(NULL, -1, 8, sizeof(char));
    memcpy(result, "<empty>", 8);
  } else if (element->type == JSON_STRING) {
    const unsigned int length = strlen(element->value);
    result = allocate(NULL, -1, length + 3, sizeof(char));
    result[0] = '"';
    memcpy(result + 1, element->value, length);
    result[length + 1] = '"';
    result[length + 2] = '\0';
  } else if (element->type == JSON_NUMBER) {
    const double number = *((double *) element->value);
    double integer;
    double fractional = (modf(number, &integer) * pow(10, fractional_limit));

    const unsigned int digit_count = ((integer == 0.0) ? 1 : (floor(log10(integer)) + 1));
    char formatter[13];

    if (fractional != 0.0) {
      result = allocate(NULL, -1, digit_count + fractional_limit + 2, sizeof(char));
      sprintf(formatter, "%%.0f.%%%d.0f", fractional_limit);
      sprintf(result, formatter, integer, fractional);
    } else {
      result = allocate(NULL, -1, (digit_count + 1), sizeof(char));
      sprintf(result, "%.0f", integer);
    }
  } else if (element->type == JSON_BOOLEAN) {
    const bool value = *((bool *) element->value);

    if (value) {
      result = allocate(NULL, -1, 5, sizeof(char));
      memcpy(result, "true", 5);
    } else {
      result = allocate(NULL, -1, 6, sizeof(char));
      memcpy(result, "false", 6);
    }
  } else if (element->type == JSON_NULL) {
    result = allocate(NULL, -1, 5, sizeof(char));
    memcpy(result, "null", 5);
  } else if (element->type == JSON_OBJECT) {
    unsigned int result_length = 2;
    unsigned int i;

    result = allocate(NULL, -1, result_length + 1, sizeof(char));
    result[0] = 0;
    strcat(result, "{");

    for (i = 0; i < element->size; ++i) {
      const jsonelement_t *data = ((jsonelement_t **) element->value)[i];
      char *value = json_stringify(data, fractional_limit);
      const unsigned int key_length = strlen(data->key);
      const unsigned int value_length = strlen(value);
      const bool has_comma = (element->size != (i + 1));

      const unsigned int length_addition = key_length + 3 + value_length + has_comma;
      result_length += length_addition;
      result = allocate(result, result_length - length_addition + 1, result_length + 1, sizeof(char));
      strcat(result, "\"");
      strcat(result, data->key);
      strcat(result, "\":");
      strcat(result, value);

      if (has_comma) {
        strcat(result, ",");
      }

      free(value);
    }

    strcat(result, "}");
  } else if (element->type == JSON_ARRAY) {
    unsigned int result_length = 2;
    unsigned int i;

    result = allocate(NULL, -1, result_length + 1, sizeof(char));
    result[0] = 0;
    strcat(result, "[");

    for (i = 0; i < element->size; ++i) {
      const jsonelement_t *data = ((jsonelement_t **) element->value)[i];
      char *value = json_stringify(data, fractional_limit);
      const unsigned int value_length = strlen(value);
      const bool has_comma = (element->size != (i + 1));

      const unsigned int length_addition = value_length + has_comma;
      result_length += length_addition;
      result = allocate(result, result_length - length_addition + 1, result_length + 1, sizeof(char));
      strcat(result, value);

      if (has_comma) {
        strcat(result, ",");
      }

      free(value);
    }

    strcat(result, "]");
  }

  return result;
}
