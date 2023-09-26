#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <discord.h>
#include <network.h>

unsigned int get_all_intents() {
	unsigned int result = 0;

	for (int i = 0; i <= 21; ++i) {
		result |= (1 << i);
	}

	return result;
}

struct Response api_request(const char *token, const char *path, const char *method, const char *body) {
	char url[256] = {0};
	sprintf(url, "https://discord.com/api/v10%s", path);

	char authorization[128] = {0};
	sprintf(authorization, "Bot %s", token);

	struct RequestConfig config = {
		.url = url,
		.method = (char *) method,
		.headers = allocate(NULL, 0, 2 + !!body, sizeof(struct Header))
	};

	config.headers[0] = (struct Header) {
		.name = "Authorization",
		.value = authorization
	};

	config.headers[1] = (struct Header) {
		.name = "Content-Type",
		.value = "application/json"
	};

	if (body != NULL) {
		size_t body_length = strlen(body);
		config.body = allocate(NULL, 0, body_length + 1, sizeof(char));
		strcpy(config.body, body);

		char length[5] = {0};
		sprintf(length, "%ld", body_length);

		config.header_size = 3;
		config.headers[2] = (struct Header) {
			.name = "Content-Length",
			.value = length
		};
	} else {
		config.header_size = 2;
	}

	struct Response response = request(config);
	free(config.headers);

	if (body != NULL) {
		free(config.body);
	}

	return response;
}

bool check_snowflake(const char *snowflake) {
	size_t length = strlen(snowflake);

	if (length != 18) {
		return false;
	} else {
		bool result = true;

		for (short i = 0; i < length; ++i) {
			if (!isdigit(snowflake[i])) {
				result = false;
				break;
			}
		}

		return result;
	}
}
