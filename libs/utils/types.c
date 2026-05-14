#include <shivers.h>

char *base64_encode(const char *data, const uint64_t data_length) {
  const char base64_alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const uint8_t m3 = (data_length % 3);         // surplus length
  const uint64_t d3 = ((data_length - m3) / 3); // full parts
  const uint64_t loop_limit = ((m3 == 0) ? d3 : (d3 + 1));

  const uint64_t response_length = ((d3 + ((uint64_t) (m3 != 0))) * 4);
  uint64_t response_index = 0;

  char *response = allocate(NULL, 0, response_length + 1, sizeof(char));
  if (response == NULL)
    return NULL;

  for (uint64_t i = 0; i < loop_limit; ++i) {
    uint64_t number = 0;
    const uint64_t di = i * 3;

    if (m3 == 0 || (i + 1) != loop_limit) {
      number |= (uint8_t) data[di] << 16;
      number |= (uint8_t) data[di + 1] << 8;
      number |= (uint8_t) data[di + 2];

      response[response_index++] = base64_alphabet[(number >> 18) & 0x3F];
      response[response_index++] = base64_alphabet[(number >> 12) & 0x3F];
      response[response_index++] = base64_alphabet[(number >> 6) & 0x3F];
      response[response_index++] = base64_alphabet[number & 0x3F];
    } else if (m3 == 2) {
      number |= (uint8_t) data[di] << 8;
      number |= (uint8_t) data[di + 1];
      number = number << 2;

      response[response_index++] = base64_alphabet[(number >> 12) & 0x3F];
      response[response_index++] = base64_alphabet[(number >> 6) & 0x3F];
      response[response_index++] = base64_alphabet[number & 0x3F];
      response[response_index++] = '=';
    } else if (m3 == 1) {
      number |= (uint8_t) data[di];
      number = number << 4;

      response[response_index++] = base64_alphabet[(number >> 6) & 0x3F];
      response[response_index++] = base64_alphabet[number & 0x3F];
      response[response_index++] = '=';
      response[response_index++] = '=';
    }
  }

  ASSERT(response_length, ==, response_index);
  return response;
}

unsigned long ahtoi(const char *data) {
  char hex_alphabet[17] = "0123456789ABCDEF";
  const unsigned int size = strlen(data);
  unsigned long result = 0;

  for (unsigned int i = 0; i < size; ++i) {
    unsigned long base = pow(16, (size - i - 1));
    result |= (char_at(hex_alphabet, toupper(data[i])) * ((base == 0) ? 1 : base));
  }

  return result;
}

int atoi_s(const char *str, short length) {
  if (length == -1) {
    length = strlen(str);
  }

  int result = 0;

  for (short i = 0; i < length; ++i) {
    const char ch = str[i];

    if (isdigit(ch)) {
      result += ((ch - '0') * pow(10.0, (length - i - 1)));
    } else {
      result = -1;
      break;
    }
  }

  return result;
}
