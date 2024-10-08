#include <string.h>
#include <stdbool.h>

#include <json.h>
#include <utils.h>

void json_del_val(jsonelement_t *element, const char *search) {
  struct Split splitter = split(search, strlen(search), ".");
  bool exist = true;

  const unsigned int splitter_size = splitter.size;

  for (unsigned int ki = 0; ki < splitter_size; ++ki) {
    if (element->type == JSON_ARRAY) {
      int index = atoi_s(splitter.data[ki].data, splitter.data[ki].length);

      if (element->size > index) {
        element = ((jsonelement_t **) element->value)[index];
      } else {
        exist = false;
        break;
      }
    } else if (element->type == JSON_OBJECT) {
      const unsigned int size = element->size;

      if (size == 0) {
        exist = false;
        break;
      } else {
        const unsigned int last_index = (size - 1);

        for (unsigned int i = 0; i < size; ++i) {
          jsonelement_t *data = ((jsonelement_t **) element->value)[i];

          if (streq(data->key, splitter.data[ki].data)) {
            element = data;
            break;
          } else if (i == last_index) {
            exist = false;
            ki = splitter.size;
            break;
          }
        }
      }
    } else {
      exist = false;
      break;
    }
  }

  if (exist) {
    jsonelement_t *parent = element->parent;
    const unsigned int size = parent->size;
    const unsigned int replacingBound = (size - 1);

    switch (parent->type) {
      case JSON_OBJECT:
        for (unsigned int i = 0; i < size; ++i) {
          jsonelement_t *check = ((jsonelement_t **) parent->value)[i];

          if (streq(check->key, element->key)) {
            for (unsigned int j = i; j < replacingBound; ++j) {
              jsonelement_t *current = ((jsonelement_t **) parent->value)[j];
              json_free(current, false);
              ((jsonelement_t **) parent->value)[j] = clone_json_element(((jsonelement_t **) parent->value)[j + 1]);
            }

            break;
          }
        }

        break;

      case JSON_ARRAY: {
        const unsigned int at = atoi_s(splitter.data[splitter.size - 1].data, -1);

        for (unsigned int i = at; i < replacingBound; ++i) {
          jsonelement_t *current = ((jsonelement_t **) parent->value)[i];
          json_free(current, false);
          ((jsonelement_t **) parent->value)[i] = clone_json_element(((jsonelement_t **) parent->value)[i + 1]);
        }

        break;
      }

      default:
        break;
    }

    --parent->size;
    json_free(((jsonelement_t **) parent->value)[parent->size], false);
    parent->value = allocate(parent->value, -1, parent->size, sizeof(jsonelement_t *));
  }

  split_free(splitter);
}
