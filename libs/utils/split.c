#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <utils.h>

void split_free(struct Split value) {
  for (unsigned int i = 0; i < value.size; ++i) {
    free(value.data[i].data);
  }

  free(value.data);
}

struct Split split(const char *text, const unsigned int length, const char *separator) {
  const unsigned int separator_length = strlen(separator);

  struct Split result = {
    .data = allocate(NULL, 0, 1, sizeof(struct SplitData)),
    .size = 1
  };

  for (unsigned int i = 0; i < length; ++i) {
    char **data = (char **) &(result.data[result.size - 1].data);
    struct SplitData *splitdata = &result.data[result.size - 1];
    const char ch = text[i];

    if (ch != separator[0]) {
      ++splitdata->length;
      *data = allocate(splitdata->length == 1 ? NULL : *data, -1, splitdata->length + 1, sizeof(char));

      (*data)[splitdata->length - 1] = ch;

      if ((length - 1) == i) {
        (*data)[splitdata->length] = 0;
      }
    } else {
      bool completed = true;

      for (unsigned int _i = 1; _i < separator_length; ++_i) {
        if (separator[_i] != text[i + _i]) {
          completed = false;
          _i = separator_length;
        }
      }

      if (completed) {
        if (splitdata->length == 0) {
          result.data[result.size - 1].data = allocate(NULL, 0, 1, sizeof(char));
        }

        i += separator_length - 1;
        (*data)[splitdata->length] = 0;
        ++result.size;
        result.data = allocate(result.data, result.size - 1, result.size, sizeof(struct SplitData));

        if (length == (i + 1)) {
          result.data[result.size - 1].data = allocate(NULL, 0, 1, sizeof(char));
        }
      } else {
        ++splitdata->length;
        *data = allocate(*data, -1, splitdata->length + 1, sizeof(char));

        (*data)[splitdata->length - 1] = ch;

        if ((length - 1) == i) {
          (*data)[splitdata->length] = 0;
        }
      }
    }
  }

  return result;
}
