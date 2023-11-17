#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <discord.h>
#include <network.h>
#include <json.h>

void send_message(const struct Client client, const char *channel_id, const struct Message message) {
	jsonelement_t *payload = create_empty_json_element(false);

	char path[64];
	sprintf(path, "/channels/%s/messages", channel_id);

	if (message.content) {
		json_set_val(payload, "content", message.content, JSON_STRING);
	}

	if (message.embed_size != 0) {
		jsonelement_t *embeds_payload = create_empty_json_element(true);

		for (unsigned char i = 0; i < message.embed_size; ++i) {
			struct Embed embed = message.embeds[i];
			jsonelement_t *embed_payload = create_empty_json_element(false);

			if (embed.author.name) {
				jsonelement_t *embed_author_payload = create_empty_json_element(false);
				json_set_val(embed_author_payload, "name", embed.author.name, JSON_STRING);

				if (embed.author.url) {
					json_set_val(embed_author_payload, "url", embed.author.url, JSON_STRING);
				}

				if (embed.author.icon_url) {
					json_set_val(embed_author_payload, "icon_url", embed.author.icon_url, JSON_STRING);
				}

				json_set_val(embed_payload, "author", embed_author_payload, JSON_OBJECT);
				json_free(embed_author_payload, false);
			}

			if (embed.title) {
				json_set_val(embed_payload, "title", embed.title, JSON_STRING);
			}

			if (embed.description) {
				json_set_val(embed_payload, "description", embed.description, JSON_STRING);
			}

			if (embed.color) {
				json_set_val(embed_payload, "color", &(embed.color), JSON_NUMBER);
			}

			if (embed.image_url) {
				jsonelement_t *embed_image_payload = create_empty_json_element(false);
				json_set_val(embed_image_payload, "url", embed.image_url, JSON_STRING);

				json_set_val(embed_payload, "image", embed_image_payload, JSON_OBJECT);
				json_free(embed_image_payload, false);
			}

			if (embed.thumbnail_url) {
				jsonelement_t *thumbnail = create_empty_json_element(false);
				json_set_val(thumbnail, "url", embed.thumbnail_url, JSON_STRING);

				json_set_val(embed_payload, "thumbnail", thumbnail, JSON_OBJECT);
				json_free(thumbnail, false);
			}

			if (embed.footer.text) {
				jsonelement_t *embed_footer_payload = create_empty_json_element(false);
				json_set_val(embed_footer_payload, "text", embed.footer.text, JSON_STRING);

				if (embed.footer.icon_url) {
					json_set_val(embed_footer_payload, "icon_url", embed.footer.icon_url, JSON_STRING);
				}

				json_set_val(embed_payload, "footer", embed_footer_payload, JSON_OBJECT);
				json_free(embed_footer_payload, false);
			}


			if (embed.field_size != 0) {
				jsonelement_t *fields_payload = create_empty_json_element(true);

				for (unsigned int f = 0; f < embed.field_size; ++f) {
					struct EmbedField field = embed.fields[f];
					jsonelement_t *field_payload = create_empty_json_element(false);

					json_set_val(field_payload, "name", field.name, JSON_STRING);
					json_set_val(field_payload, "value", field.value, JSON_STRING);

					if (field.is_inline) {
						json_set_val(field_payload, "inline", &(field.is_inline), JSON_BOOLEAN);
					}

					json_set_val(fields_payload, NULL, field_payload, JSON_OBJECT);
					json_free(field_payload, false);
				}

				json_set_val(embed_payload, "fields", fields_payload, JSON_ARRAY);
				json_free(fields_payload, false);
			}

			json_set_val(embeds_payload, NULL, embed_payload, JSON_OBJECT);
			json_free(embed_payload, false);
		}

		json_set_val(payload, "embeds", embeds_payload, JSON_ARRAY);
		json_free(embeds_payload, false);
	}

	struct Response response;
	char *body = json_stringify(payload, 5);

	if (message.file_size != 0 && payload->size != 0) {
		struct FormData formdata = {
			.boundary = "deneme"
		};

		add_field_to_formdata(&formdata, "payload_json", body, strlen(body), NULL);
		add_header_to_formdata_field(&formdata, "payload_json", "Content-Type", "application/json");

		for (size_t i = 0; i < message.file_size; ++i) {
			struct File file = message.files[i];
			char *field_name = allocate(NULL, -1, 12, sizeof(char));
			sprintf(field_name, "files[%ld]", i);

			add_field_to_formdata(&formdata, field_name, file.data, file.size, file.name);
			add_header_to_formdata_field(&formdata, field_name, "Content-Type", file.type);
			free(field_name);
		}

		response = api_request(client.token, path, "POST", NULL, &formdata);
		free_formdata(formdata);
	} else if (message.file_size != 0) {
		struct FormData formdata = {
			.boundary = "deneme"
		};

		for (size_t i = 0; i < message.file_size; ++i) {
			struct File file = message.files[i];
			char *field_name = allocate(NULL, -1, 12, sizeof(char));
			sprintf(field_name, "files[%ld]", i);

			add_field_to_formdata(&formdata, field_name, file.data, file.size, file.name);
			add_header_to_formdata_field(&formdata, field_name, "Content-Type", file.type);
			free(field_name);
		}

		response = api_request(client.token, path, "POST", NULL, &formdata);
		free_formdata(formdata);
	} else if (payload->size != 0) {
		response = api_request(client.token, path, "POST", body, NULL);
	} else {
		throw("cannot send empty message");
	}

	response_free(&response);
	json_free(payload, true);
	free(body);
}

void add_field_to_embed(struct Embed *embed, char *name, char *value, bool is_inline) {
	++embed->field_size;
	embed->fields = allocate(embed->fields, -1, embed->field_size, sizeof(struct EmbedField));

	embed->fields[embed->field_size - 1] = (struct EmbedField) {
		.name = name,
		.value = value,
		.is_inline = is_inline
	};
}

void set_embed_author(struct Embed *embed, char *name, char *url, char *icon_url) {
	embed->author = (struct EmbedAuthor) {
		.name = name,
		.url = url,
		.icon_url = icon_url
	};
}

void set_embed_footer(struct Embed *embed, char *text, char *icon_url) {
	embed->footer = (struct EmbedFooter) {
		.text = text,
		.icon_url = icon_url
	};
}

void add_embed_to_message(const struct Embed embed, struct Message *message) {
	++message->embed_size;
	message->embeds = allocate(message->embeds, -1, message->embed_size, sizeof(struct Embed));
	message->embeds[message->embed_size - 1] = embed;
}

void free_message(struct Message message) {
	if (message.embed_size != 0) {
		free(message.embeds);
	}

	if (message.file_size != 0) {
		free(message.files);
	}
}

void add_file_to_message(struct Message *message, const char *name, const char *data, const size_t size, const char *type) {
	++message->file_size;
	message->files = allocate(message->files, -1, message->file_size, sizeof(struct File));
	message->files[message->file_size - 1] = (struct File) {
		.name = (char *) name,
		.data = (char *) data,
		.type = (char *) type,
		.size = size
	};
}
