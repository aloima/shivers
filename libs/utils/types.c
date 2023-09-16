#include <string.h>
#include <ctype.h>
#include <math.h>

#include <utils.h>

char *base64_encode(const char *data, const size_t data_length) {
	const char base64_alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const size_t m3 = (data_length % 3);
	const size_t d3 = ((data_length - m3) / 3);
	const size_t loop_limit = ((m3 == 0) ? d3 : (d3 + 1));

	size_t response_length;

	switch (m3) {
		case 0:
			response_length = (d3 * 4);
			break;

		case 1:
			response_length = ((d3 + 1) * 4);
			break;

		case 2:
			response_length = ((d3 + 1) * 4);
			break;
	}

	char *response = allocate(NULL, 0, response_length + 1, sizeof(char));

	for (size_t i = 0; i < loop_limit; ++i) {
		size_t number = 0;
		const int di = i * 3;

		if (m3 == 0 || (i + 1) != loop_limit) {
			number |= (unsigned char) data[di] << 16;
			number |= (unsigned char) data[di + 1] << 8;
			number |= (unsigned char) data[di + 2];

			strncat(response, &base64_alphabet[(number >> 18) & 0x3F], 1);
			strncat(response, &base64_alphabet[(number >> 12) & 0x3F], 1);
			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
		} else if (m3 == 2) {
			number |= (unsigned char) data[di] << 8;
			number |= (unsigned char) data[di + 1];
			number = number << 2;

			strncat(response, &base64_alphabet[(number >> 12) & 0x3F], 1);
			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
			strncat(response, "=", 1);
		} else if (m3 == 1) {
			number |= (unsigned char) data[di];
			number = number << 4;

			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
			strncat(response, "==", 2);
		}
	}

	return response;
}

size_t ahtoi(const char *data) {
	const char hex_alphabet[17] = "0123456789ABCDEF";
	const size_t size = strlen(data);
	size_t result = 0;

	for (int i = 0; i < size; ++i) {
		size_t base = pow(16, (size - i - 1));
		result |= (char_at(hex_alphabet, toupper(data[i])) * ((base == 0) ? 1 : base));
	}

	return result;
}
