#include <stdio.h>
#include <string.h>

#include <discord.h>
#include <network.h>

const unsigned int get_all_intents() {
	unsigned int result = 0;

	for (int i = 0; i <= 21; ++i) {
		result |= (1 << i);
	}

	return (const unsigned int) result;
}

Response api_request(char *token, char *path, char *method, char *body) {
	char url[256] = {0};
	sprintf(url, "https://discord.com/api/v10%s", path);

	char authorization[128] = {0};
	sprintf(authorization, "Bot %s", token);

	RequestConfig config;
	memset(&config, 0, sizeof(RequestConfig));

	config.url = url;
	config.method = method;
	config.headers = allocate(NULL, 2 + (body ? 1 : 0), sizeof(Header));

	config.headers[0] = (Header) {
		.name = "Authorization",
		.value = authorization
	};

	config.headers[1] = (Header) {
		.name = "Content-Type",
		.value = "application/json"
	};

	if (body != NULL) {
		size_t body_length = strlen(body);
		config.body = allocate(NULL, body_length + 1, sizeof(char));
		strcpy(config.body, body);

		char length[5] = {0};
		sprintf(length, "%ld", body_length);

		config.header_size = 3;
		config.headers[2] = (Header) {
			.name = "Content-Length",
			.value = length
		};
	} else {
		config.header_size = 2;
	}

	return request(config);
}

void send_content(Client client, const char *channel_id, const char *content) {
	char path[64] = {0};
	sprintf(path, "/channels/%s/messages", channel_id);

	char body[2048] = {0};
	sprintf(body, "{\"content\":\"%s\"}", content);

	api_request(client.token, path, "POST", body);
}
