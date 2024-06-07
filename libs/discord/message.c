#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <discord.h>
#include <network.h>
#include <json.h>

unsigned short send_message(const struct Client client, const struct Message message) {
	const struct MessagePayload message_payload = message.payload;
	jsonelement_t *payload = create_empty_json_element(false);

	char path[512];

	if (message.target_type == TARGET_CHANNEL) {
		sprintf(path, "/channels/%s/messages", message.target.channel_id);
	} else {
		const struct InteractionCommand interaction_command = message.target.interaction_command;
		sprintf(path, "/interactions/%s/%s/callback", interaction_command.id, interaction_command.token);
	}

	if (message_payload.content) {
		json_set_val(payload, "content", message_payload.content, JSON_STRING);
	}

	if (message_payload.embed_size != 0) {
		jsonelement_t *embeds_payload = create_empty_json_element(true);

		for (unsigned int i = 0; i < message_payload.embed_size; ++i) {
			struct Embed embed = message_payload.embeds[i];
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

	if (message.payload.ephemeral && message.target_type == TARGET_INTERACTION_COMMAND) {
		double value = (1 << 6);
		json_set_val(payload, "flags", &value, JSON_NUMBER);
	}

	struct Response response;
	char *body = NULL;

	if (message_payload.file_size != 0 && payload->size != 0) {
		struct FormData formdata = {
			.boundary = "deneme"
		};

		jsonelement_t *attachments = create_empty_json_element(true);

		for (unsigned int i = 0; i < message_payload.file_size; ++i) {
			const struct File file = message_payload.files[i];
			jsonelement_t *attachment = create_empty_json_element(false);
			char *field_name = allocate(NULL, -1, 12, sizeof(char));
			sprintf(field_name, "files[%u]", i);

			double n = i;
			json_set_val(attachment, "id", &n, JSON_NUMBER);
			json_set_val(attachments, NULL, attachment, JSON_OBJECT);

			add_field_to_formdata(&formdata, field_name, file.data, file.size, file.name);
			add_header_to_formdata_field(&formdata, field_name, "Content-Type", file.type);

			json_free(attachment, true);
			free(field_name);
		}

		json_set_val(payload, "attachments", attachments, JSON_ARRAY);

		body = json_stringify(payload, 5);
		const unsigned long long body_length = strlen(body);

		if (message.target_type == TARGET_INTERACTION_COMMAND) {
			body = allocate(body, -1, body_length + 19, sizeof(char));
			memcpy(body + 17, body, body_length);
			memcpy(body, "{\"type\":4,\"data\":", 17);
			body[body_length + 17] = '}';
			body[body_length + 18] = 0;

			add_field_to_formdata(&formdata, "payload_json", body, body_length + 18, NULL);
		} else {
			add_field_to_formdata(&formdata, "payload_json", body, body_length, NULL);
		}

		add_header_to_formdata_field(&formdata, "payload_json", "Content-Type", "application/json");

		response = api_request(client.token, path, "POST", NULL, &formdata);
		json_free(attachments, false);
		free_formdata(formdata);
	} else if (message_payload.file_size != 0) {
		struct FormData formdata = {
			.boundary = "deneme"
		};

		for (unsigned int i = 0; i < message_payload.file_size; ++i) {
			const struct File file = message_payload.files[i];
			char *field_name = allocate(NULL, -1, 12, sizeof(char));
			sprintf(field_name, "files[%u]", i);

			add_field_to_formdata(&formdata, field_name, file.data, file.size, file.name);
			add_header_to_formdata_field(&formdata, field_name, "Content-Type", file.type);

			free(field_name);
		}

		if (message.target_type == TARGET_INTERACTION_COMMAND) {
			body = allocate(body, -1, 21, sizeof(char));
			memcpy(body, "{\"type\":4,\"data\":{}}", 20);

			add_field_to_formdata(&formdata, "payload_json", body, 20, NULL);
			add_header_to_formdata_field(&formdata, "payload_json", "Content-Type", "application/json");
		}

		response = api_request(client.token, path, "POST", NULL, &formdata);
		free_formdata(formdata);
	} else if (payload->size != 0) {
		body = json_stringify(payload, 5);

		if (message.target_type == TARGET_INTERACTION_COMMAND) {
			const unsigned int body_length = strlen(body);
			body = allocate(body, -1, body_length + 19, sizeof(char));
			memcpy(body + 17, body, body_length);
			memcpy(body, "{\"type\":4,\"data\":", 17);
			body[body_length + 17] = '}';
			body[body_length + 18] = 0;
		}

		response = api_request(client.token, path, "POST", body, NULL);
	} else {
		throw("cannot send empty message");
	}

	const unsigned short status = response.status.code;

	response_free(response);
	json_free(payload, true);
	free(body);

	return status;
}

void add_field_to_embed(struct Embed *embed, const char *name, const char *value, const bool is_inline) {
	++embed->field_size;
	embed->fields = allocate(embed->fields, -1, embed->field_size, sizeof(struct EmbedField));

	embed->fields[embed->field_size - 1] = (struct EmbedField) {
		.name = (char *) name,
		.value = (char *) value,
		.is_inline = is_inline
	};
}

void set_embed_author(struct Embed *embed, const char *name, const char *url, const char *icon_url) {
	embed->author = (struct EmbedAuthor) {
		.name = (char *) name,
		.url = (char *) url,
		.icon_url = (char *) icon_url
	};
}

void set_embed_footer(struct Embed *embed, const char *text, const char *icon_url) {
	embed->footer = (struct EmbedFooter) {
		.text = (char *) text,
		.icon_url = (char *) icon_url
	};
}

void add_embed_to_message_payload(const struct Embed embed, struct MessagePayload *message_payload) {
	++message_payload->embed_size;
	message_payload->embeds = allocate(message_payload->embeds, -1, message_payload->embed_size, sizeof(struct Embed));
	message_payload->embeds[message_payload->embed_size - 1] = embed;
}

void free_message_payload(struct MessagePayload message_payload) {
	if (message_payload.embed_size != 0) {
		free(message_payload.embeds);
	}

	if (message_payload.file_size != 0) {
		for (unsigned char i = 0; i < message_payload.file_size; ++i) {
			struct File file = message_payload.files[i];

			free(file.name);
			free(file.data);
			free(file.type);
		}

		free(message_payload.files);
	}
}

void add_file_to_message_payload(struct MessagePayload *message_payload, const char *name, const char *data, const unsigned long size, const char *type) {
	++message_payload->file_size;
	message_payload->files = allocate(message_payload->files, -1, message_payload->file_size, sizeof(struct File));

	const unsigned int name_size = (strlen(name) + 1);
	const unsigned int type_size = (strlen(type) + 1);

	struct File *file_in_payload = &(message_payload->files[message_payload->file_size - 1]);

	memcpy(file_in_payload, &((struct File) {
		.name = allocate(NULL, -1, name_size, sizeof(char)),
		.data = allocate(NULL, -1, size, sizeof(char)),
		.type = allocate(NULL, -1, type_size, sizeof(char)),
		.size = size
	}), sizeof(struct File));

	memcpy(file_in_payload->name, name, name_size);
	memcpy(file_in_payload->data, data, size);
	memcpy(file_in_payload->type, type, type_size);
}
