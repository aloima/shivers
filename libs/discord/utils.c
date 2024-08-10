#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata) {
	char url[1024];
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
		const unsigned long body_size = strlen(body) + 1;
		config.body.is_formdata = false;
		config.body.payload.data = allocate(NULL, -1, body_size, sizeof(char));
		memcpy(config.body.payload.data, body, body_size);

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

struct Guild *get_guild(const struct Client client, const char *id) {
	return (struct Guild *) get_node(client.guilds, id)->value;
}

bool check_snowflake(const char *snowflake) {
	unsigned int i = 0;

	while (snowflake[i] != '\0') {
		if (!isdigit(snowflake[i])) {
			return false;
		}

		++i;
	}

	return (i == 18 || i == 19);
}

void get_avatar_url(char *url, const char *user_id, const char *discriminator, const char *hash, const bool force_png, const short size) {
	if (hash && hash[0] != 0) {
		const char *extension = ((!force_png && (strncmp(hash, "a_", 2) == 0)) ? "gif" : "png");
		sprintf(url, AVATAR_URL "?size=%d", user_id, hash, extension, size);
	} else {
		unsigned int index;

		if (!streq(discriminator, "0")) {
			index = (atoi_s(discriminator, 4) % 5);
		} else {
			index = ((atoi_s(user_id, -1) >> 22) % 6);
		}

		sprintf(url, DEFAULT_AVATAR_URL "?size=%d", index, size);
	}
}
