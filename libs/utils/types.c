#include <string.h>
#include <math.h>

#include <utils.h>

char *base64_encode(const char *data) {
	char base64_alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t data_length = strlen(data);
	char m3 = (data_length % 3);
	size_t loop_limit = ((m3 == 0) ? (data_length / 3) : (floor(data_length / 3) + 1));
	short padding_count = ((m3 == 0) ? ((3 - m3) * 2) : 0);

	char *response = allocate(NULL, data_length + padding_count + 1, sizeof(char));

	for (int i = 0; i < loop_limit; ++i) {
		size_t number = 0;
		int di = i * 3;

		if (m3 == 0 || (i + 1) != loop_limit) {
			number |= data[di] << 16;
			number |= data[di + 1] << 8;
			number |= data[di + 2];

			strncat(response, &base64_alphabet[(number >> 18) & 0x3F], 1);
			strncat(response, &base64_alphabet[(number >> 12) & 0x3F], 1);
			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
		} else if (m3 == 2) {
			number |= data[di] << 8;
			number |= data[di + 1];
			number = number << 2;

			strncat(response, &base64_alphabet[(number >> 12) & 0x3F], 1);
			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
			strncat(response, "=", 2);
		} else if (m3 == 1) {
			number |= data[di];
			number = number << 4;

			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
			strncat(response, "==", 2);
		}
	}

	return response;
}
