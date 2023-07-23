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

	JSONElement *embed_data = create_empty_json_element(false);

	if (embed.description) {
		add_json_element(&embed_data, "description", embed.description, JSON_STRING);
	}

	if (embed.color) {
		add_json_element(&embed_data, "color", &(embed.color), JSON_NUMBER);
	}

	if (embed.fields) {
		JSONElement *fields = create_empty_json_element(true);

		for (short i = 0; i < embed.field_size; ++i) {
			JSONElement *field_data = create_empty_json_element(false);
			EmbedField field = embed.fields[i];

			add_json_element(&field_data, "name", field.name, JSON_STRING);
			add_json_element(&field_data, "value", field.value, JSON_STRING);
			add_json_element(&field_data, "inline", &(field.inline_mode), JSON_BOOLEAN);

			add_json_element(&fields, NULL, field_data, JSON_OBJECT);

			json_free(field_data);
		}

		add_json_element(&embed_data, "fields", fields, JSON_ARRAY);
		json_free(fields);
	}

	char *embed_text = json_stringify(embed_data);

	char *body = allocate(NULL, strlen(embed_text) + 128, sizeof(char));
	sprintf(body, "{\"embeds\":[%s]}", embed_text);

	Response response = api_request(client.token, path, "POST", body);
	response_free(&response);

	json_free(embed_data);
	free(embed_text);
	free(body);
}
