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

	Response response = request(config);
	free(config.headers);

	if (body != NULL) {
		free(config.body);
	}

	return response;
}

void send_content(Client client, const char *channel_id, const char *content) {
	char path[64] = {0};
	sprintf(path, "/channels/%s/messages", channel_id);

	char body[2048] = {0};
	sprintf(body, "{\"content\":\"%s\"}", content);

	Response response = api_request(client.token, path, "POST", body);
	response_free(&response);
}

void send_embed(Client client, const char *channel_id, Embed embed) {
	char path[64] = {0};
	sprintf(path, "/channels/%s/messages", channel_id);

	char description[4096] = {0};
	char color[128] = {0};
	char fields[25 * 2048] = {0};

	memset(&fields, 0, sizeof(fields));

	if (embed.description) {
		sprintf(description, "\"description\":\"%s\"", embed.description);
	}

	if (embed.color) {
		sprintf(color, "\"color\":%ld", embed.color);
	}

	if (embed.field_size != 0) {
		strcat(fields, "\"fields\":[");
		char field[2048] = {0};

		for (short i = 0; i < embed.field_size; ++i) {
			if (embed.field_size == (i + 1)) {
				sprintf(field, "{\"name\":\"%s\",\"value\":\"%s\",\"inline\":%s}", embed.fields[i].name, embed.fields[i].value, embed.fields[i].inline_mode ? "true" : "false");
			} else {
				sprintf(field, "{\"name\":\"%s\",\"value\":\"%s\",\"inline\":%s},", embed.fields[i].name, embed.fields[i].value, embed.fields[i].inline_mode ? "true" : "false");
			}

			strcat(fields, field);
			memset(field, 0, sizeof(field));
		}

		strcat(fields, "]");
	}

	char *elements[3] = {color, description, fields};
	char join_size = ((color[0] != 0) + (description[0] != 0) + (fields[0] != 0));

	char *embed_text = allocate(NULL, calculate_join(elements, join_size, ",") + 1, sizeof(char));
	join(elements, embed_text, join_size, ",");

	char *body = allocate(NULL, strlen(embed_text) + 128, sizeof(char));
	sprintf(body, "{\"embeds\":[{%s}]}", embed_text);

	Response response = api_request(client.token, path, "POST", body);
	response_free(&response);

	free(embed_text);
	free(body);
}
