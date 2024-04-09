#include <string.h>
#include <ctype.h>
#include <math.h>

#include <utils.h>

char *base64_encode(const char *data, const unsigned long data_length) {
	const char base64_alphabet[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const unsigned char m3 = (data_length % 3);
	const unsigned long d3 = ((data_length - m3) / 3);
	const unsigned long loop_limit = ((m3 == 0) ? d3 : (d3 + 1));

	unsigned long response_length;

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

	for (unsigned long i = 0; i < loop_limit; ++i) {
		unsigned long number = 0;
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
			strcat(response, "=");
		} else if (m3 == 1) {
			number |= (unsigned char) data[di];
			number = number << 4;

			strncat(response, &base64_alphabet[(number >> 6) & 0x3F], 1);
			strncat(response, &base64_alphabet[number & 0x3F], 1);
			strcat(response, "==");
		}
	}

	return response;
}

unsigned long ahtoi(const char *data) {
	const char hex_alphabet[17] = "0123456789ABCDEF";
	const unsigned long size = strlen(data);
	unsigned long result = 0;

	for (int i = 0; i < size; ++i) {
		unsigned long base = pow(16, (size - i - 1));
		result |= (char_at(hex_alphabet, toupper(data[i]), 16) * ((base == 0) ? 1 : base));
	}

	return result;
}

int atoi_s(const char *str, long long length) {
	if (length == -1) {
		length = strlen(str);
	}

	int result = 0;

	for (long long i = 0; i < length; ++i) {
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
