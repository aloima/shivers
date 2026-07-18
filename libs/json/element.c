#include <shivers.h>

#define ALLOCATE_VALUE(result, count, value_type) do {               \
  (result)->value = allocate(NULL, -1, (count), sizeof(value_type)); \
  if ((result)->value == NULL)                                       \
    goto FREE_ELEMENT;                                               \
} while (0)

jsonelement_t *create_empty_json_element(const bool is_array) {
  jsonelement_t *element = allocate(NULL, 0, 1, sizeof(jsonelement_t));
  if (element == NULL)
    return NULL;

  element->type = (is_array ? JSON_ARRAY : JSON_OBJECT);
  return element;
}

jsonelement_t *clone_json_element(jsonelement_t *element) {
  jsonelement_t *result = allocate(NULL, 0, 1, sizeof(jsonelement_t));
  if (result == NULL)
    return NULL;

  result->type = element->type;
  result->parent = element->parent;

  if (element->key != NULL) {
    const uint32_t key_size = (strlen(element->key) + 1);

    result->key = allocate(NULL, -1, key_size, sizeof(char));
    if (result->key == NULL)
      goto FREE_ELEMENT;

    ASSERT(memcpy(result->key, element->key, key_size), !=, NULL);
  }

  if (element->type == JSON_ARRAY || element->type == JSON_OBJECT) {
    const uint32_t size = element->size;

    result->size = size;
    ALLOCATE_VALUE(result, size, jsonelement_t);

    for (uint32_t i = 0; i < size; ++i) {
      jsonelement_t **elements = (jsonelement_t **) result->value;
      jsonelement_t *data = ((jsonelement_t **) element->value)[i];

      elements[i] = clone_json_element(data);
      if (elements[i] == NULL)
        goto FREE_ELEMENT;
    }
  } else {
    switch (result->type) {
      case JSON_NUMBER:
        ALLOCATE_VALUE(result, 1, double);
        ((double *) result->value)[0] = *((double *) element->value);
        break;

      case JSON_STRING:
        result->size = element->size;
        ALLOCATE_VALUE(result, result->size + 1, char);
        ASSERT(memcpy(result->value, element->value, result->size + 1), !=, NULL);
        break;

      case JSON_BOOLEAN:
        ALLOCATE_VALUE(result, 1, bool);
        ((bool *) result->value)[0] = *((bool *) element->value);
        break;

      default:
        result->value = NULL;
        break;
    }
  }

  return result;

  FREE_ELEMENT: {
    if (result == NULL)
      return NULL;

    if (result->key != NULL)
      free(result->key);

    if (result->value != NULL)
      free(result->value);

    free(result);
    return NULL;
  }
}
