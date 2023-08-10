#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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
	config.headers = allocate(NULL, 0, 2 + (body ? 1 : 0), sizeof(Header));

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
		config.body = allocate(NULL, 0, body_length + 1, sizeof(char));
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

	if (embed.title) {
		add_json_element(&embed_data, "title", embed.title, JSON_STRING);
	}

	if (embed.description) {
		add_json_element(&embed_data, "description", embed.description, JSON_STRING);
	}

	if (embed.color) {
		add_json_element(&embed_data, "color", &(embed.color), JSON_NUMBER);
	}

	if (embed.image_url) {
		JSONElement *image = create_empty_json_element(false);
		add_json_element(&image, "url", embed.image_url, JSON_STRING);
		add_json_element(&embed_data, "image", image, JSON_OBJECT);

		json_free(image);
	}

	if (embed.thumbnail_url) {
		JSONElement *thumbnail = create_empty_json_element(false);
		add_json_element(&thumbnail, "url", embed.thumbnail_url, JSON_STRING);
		add_json_element(&embed_data, "thumbnail", thumbnail, JSON_OBJECT);

		json_free(thumbnail);
	}

	if (embed.author.name) {
		JSONElement *author = create_empty_json_element(false);

		add_json_element(&author, "name", embed.author.name, JSON_STRING);
		if (embed.author.url) add_json_element(&author, "url", embed.author.url, JSON_STRING);
		if (embed.author.icon_url) add_json_element(&author, "icon_url", embed.author.icon_url, JSON_STRING);

		add_json_element(&embed_data, "author", author, JSON_OBJECT);
		json_free(author);
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

	char *body = allocate(NULL, 0, strlen(embed_text) + 128, sizeof(char));
	sprintf(body, "{\"embeds\":[%s]}", embed_text);

	Response response = api_request(client.token, path, "POST", body);
	response_free(&response);

	json_free(embed_data);
	free(embed_text);
	free(body);
}

void add_field_to_embed(Embed *embed, char *name, char *value, bool inline_mode) {
	++embed->field_size;
	embed->fields = allocate(embed->fields, embed->field_size - 1, embed->field_size, sizeof(EmbedField));

	embed->fields[embed->field_size - 1] = (EmbedField) {
		.name = name,
		.value = value,
		.inline_mode = inline_mode
	};
}

void set_embed_author(Embed *embed, char *name, char *url, char *icon_url) {
	embed->author = (EmbedAuthor) {
		.name = name,
		.url = url,
		.icon_url = icon_url
	};
}

bool check_snowflake(char *snowflake) {
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
