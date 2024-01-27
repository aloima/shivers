#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <discord.h>
#include <network.h>
#include <json.h>

unsigned int get_all_intents() {
	unsigned int result = 0;

	for (unsigned char i = 0; i <= 21; ++i) {
		result |= (1 << i);
	}

	return result;
}

struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata) {
	char url[256];
	sprintf(url, "https://discord.com/api/v10%s", path);

	char authorization[100];
	sprintf(authorization, "Bot %s", token);

	struct Header headers[2] = {
		(struct Header) {
			.name = "Authorization",
			.value = authorization
		}
	};

	struct RequestConfig config = {
		.url = url,
		.method = (char *) method,
		.headers = headers,
		.header_size = 1
	};

	if (body != NULL && formdata == NULL) {
		const unsigned long body_length = strlen(body);
		config.body.is_formdata = false;
		config.body.payload.data = allocate(NULL, 0, body_length + 1, sizeof(char));
		strcpy(config.body.payload.data, body);

		headers[1] = (struct Header) {
			.name = "Content-Type",
			.value = "application/json"
		};

		config.headers = headers;
		config.header_size = 2;
	} else if (body == NULL && formdata != NULL && formdata->field_size != 0) {
		config.body.is_formdata = true;
		config.body.payload.formdata = *formdata;
	}

	struct Response response = request(config);

	if (body != NULL) {
		free(config.body.payload.data);
	}

	return response;
}

bool check_snowflake(const char *snowflake) {
	const unsigned long length = strlen(snowflake);

	if (length != 18 && length != 19) {
		return false;
	} else {
		bool result = true;

		for (unsigned char i = 0; i < length; ++i) {
			if (!isdigit(snowflake[i])) {
				result = false;
				break;
			}
		}

		return result;
	}
}

void get_avatar_url(char *url, const char *token, const char *user_id, const char *discriminator, const char *hash, const bool force_png, const short size) {
	if (discriminator == NULL) {
		char path[26] = "/users/";
		strcat(path, user_id);

		struct Response response = api_request(token, path, "GET", NULL, NULL);
		jsonelement_t *user = json_parse((const char *) response.data);
		const jsonresult_t avatar = json_get_val(user, "avatar");

		if (avatar.exist && avatar.type != JSON_NULL) {
			const char *avatar_hash = avatar.value.string;
			const char *extension = ((!force_png && (strncmp(avatar_hash, "a_", 2) == 0)) ? "gif" : "png");

			sprintf(url, AVATAR_URL "?size=%d", user_id, avatar_hash, extension, size);
		} else {
			const char *_discriminator = json_get_val(user, "discriminator").value.string;

			sprintf(url, DEFAULT_AVATAR_URL "?size=%d", atoi(_discriminator) % 5, size);
		}

		json_free(user, false);
		response_free(&response);
	} else {
		if (hash != NULL) {
			const char *extension = ((!force_png && (strncmp(hash, "a_", 2) == 0)) ? "gif" : "png");

			sprintf(url, AVATAR_URL "?size=%d", user_id, hash, extension, size);
		} else {
			sprintf(url, DEFAULT_AVATAR_URL "?size=%d", atoi(discriminator) % 5, size);
		}
	}
}
