#include <shivers.h>

uint64_t calculate_join(const Join *value, uint16_t size, const char *separator) {
  uint64_t source_length = (strlen(separator) * (size - 1));

  for (uint16_t i = 0; size != 0; ++i) {
    const uint64_t string_length = value[i].length;

    if (string_length != 0) {
      source_length += string_length;
      --size;
    }
  }

  return source_length;
}

uint64_t join(const Join *value, char *source, uint16_t size, const char *separator) {
  const uint64_t separator_length = strlen(separator);
  uint64_t source_length = 0;

  for (unsigned short i = 0; size != 0; ++i) {
    const Join join_element = value[i];
    const bool has_separator = (size != 1);

    if (join_element.length != 0) {
      ASSERT(memcpy(source + source_length, join_element.data, join_element.length), !=, NULL);

      if (has_separator) {
        ASSERT(memcpy(source + source_length + join_element.length, separator, separator_length), !=, NULL);
      }

      --size;
      source_length += (join_element.length + (has_separator ? separator_length : 0));
    }
  }

  source[source_length] = 0;
  return source_length;
}
