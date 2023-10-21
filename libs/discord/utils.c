#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <discord.h>
#include <network.h>

unsigned int get_all_intents() {
	unsigned int result = 0;

	for (unsigned char i = 0; i <= 21; ++i) {
		result |= (1 << i);
	}

	return result;
}

struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata) {
	char url[256] = {0};
	sprintf(url, "https://discord.com/api/v10%s", path);

	char authorization[128] = {0};
	sprintf(authorization, "Bot %s", token);

	struct RequestConfig config = {
		.url = url,
		.method = (char *) method,
		.headers = allocate(NULL, -1, 1 + !!formdata + !!body, sizeof(struct Header))
	};

	config.headers[0] = (struct Header) {
		.name = "Authorization",
		.value = authorization
	};

	config.header_size = 1;

	if (body != NULL && formdata == NULL) {
		size_t body_length = strlen(body);
		config.body.is_formdata = false;
		config.body.payload.data = allocate(NULL, 0, body_length + 1, sizeof(char));
		strcpy(config.body.payload.data, body);

		char length[5] = {0};
		sprintf(length, "%ld", body_length);

		config.header_size = 2;

		config.headers[1] = (struct Header) {
			.name = "Content-Type",
			.value = "application/json"
		};
	} else if (body == NULL && formdata != NULL && formdata->field_size != 0) {
		config.body.is_formdata = true;
		config.body.payload.formdata = *formdata;
	}

	struct Response response = request(config);
	free(config.headers);

	if (body != NULL) {
		free(config.body.payload.data);
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
