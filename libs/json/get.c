#include <shivers.h>

jsonresult_t json_get_val(jsonelement_t *element, const char *search) {
  struct Split splitter = split(search, strlen(search), ".");
  jsonresult_t result = {
    .exist = true
  };

  const uint32_t splitter_size = splitter.size;

  for (uint32_t ki = 0; ki < splitter_size; ++ki) {
    struct SplitData split_data = splitter.data[ki];
    const uint32_t size = element->size;
    const enum JSONType type = element->type;

    if (type == JSON_ARRAY) {
      const uint32_t index = (uint32_t) atoi_s(split_data.data, split_data.length);
      if (size <= index)
        goto BREAK_LOOP;

      jsonelement_t **elements = (jsonelement_t **) element->value;
      element = elements[index];
    } else if (type == JSON_OBJECT) {
      if (size == 0)
        goto BREAK_LOOP;

      const uint32_t last_index = (size - 1);

      for (uint32_t i = 0; i < size; ++i) {
        jsonelement_t *data = ((jsonelement_t **) element->value)[i];

        if (streq(data->key, split_data.data)) {
          element = data;
          break;
        } else if (i == last_index)
          goto BREAK_LOOP;
      }
    }
    
    BREAK_LOOP: {
      result.exist = false;
      break;
    }
  }

  split_free(splitter);

  if (!result.exist) {
    return (jsonresult_t) {0};
  }

  result.element = element;

  switch (element->type) {
    case JSON_NUMBER:
      result.value.number = *((double *) element->value);
      break;

    case JSON_STRING:
      result.value.string = (char *) element->value;
      break;

    case JSON_BOOLEAN:
      result.value.boolean = *((bool *) element->value);
      break;

    default:
      break;
  }

  return result;
}
