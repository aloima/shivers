#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <discord.h>
#include <json.h>

void send_message(const struct Client client, const char *channel_id, const struct Message message) {
	jsonelement_t *payload = create_empty_json_element(false);

	char path[64] = {0};
	sprintf(path, "/channels/%s/messages", channel_id);

	if (message.content) add_json_element(&payload, "content", message.content, JSON_STRING);

	if (message.embed_size != 0) {
		jsonelement_t *embeds_payload = create_empty_json_element(true);

		for (size_t i = 0; i < message.embed_size; ++i) {
			struct Embed embed = message.embeds[i];
			jsonelement_t *embed_payload = create_empty_json_element(false);

			if (embed.author.name) {
				jsonelement_t *embed_author_payload = create_empty_json_element(false);
				if (embed.author.name) add_json_element(&embed_author_payload, "name", embed.author.name, JSON_STRING);
				if (embed.author.url) add_json_element(&embed_author_payload, "url", embed.author.url, JSON_STRING);
				if (embed.author.icon_url) add_json_element(&embed_author_payload, "icon_url", embed.author.icon_url, JSON_STRING);

				add_json_element(&embed_payload, "author", embed_author_payload, JSON_OBJECT);
				json_free(embed_author_payload);
			}

			if (embed.title) add_json_element(&embed_payload, "title", embed.title, JSON_NUMBER);
			if (embed.description) add_json_element(&embed_payload, "description", embed.description, JSON_STRING);
			if (embed.color) add_json_element(&embed_payload, "color", &(embed.color), JSON_NUMBER);

			if (embed.image_url) {
				jsonelement_t *embed_image_payload = create_empty_json_element(false);
				add_json_element(&embed_image_payload, "url", embed.image_url, JSON_STRING);

				add_json_element(&embed_payload, "image", embed_image_payload, JSON_OBJECT);
				json_free(embed_image_payload);
			}

			if (embed.thumbnail_url) {
				jsonelement_t *thumbnail = create_empty_json_element(false);
				add_json_element(&thumbnail, "url", embed.thumbnail_url, JSON_STRING);

				add_json_element(&embed_payload, "thumbnail", thumbnail, JSON_OBJECT);
				json_free(thumbnail);
			}

			if (embed.field_size != 0) {
				jsonelement_t *fields_payload = create_empty_json_element(true);

				for (size_t e = 0; e < embed.field_size; ++e) {
					struct EmbedField field = embed.fields[e];
					jsonelement_t *field_payload = create_empty_json_element(false);

					if (field.name) add_json_element(&field_payload, "name", field.name, JSON_STRING);
					if (field.value) add_json_element(&field_payload, "value", field.value, JSON_STRING);
					if (field.is_inline) add_json_element(&field_payload, "inline", &(field.is_inline), JSON_BOOLEAN);

					add_json_element(&fields_payload, NULL, field_payload, JSON_OBJECT);
					json_free(field_payload);
				}

				add_json_element(&embed_payload, "fields", fields_payload, JSON_ARRAY);
				json_free(fields_payload);
			}

			add_json_element(&embeds_payload, NULL, embed_payload, JSON_OBJECT);
			json_free(embed_payload);
		}

		add_json_element(&payload, "embeds", embeds_payload, JSON_ARRAY);
		json_free(embeds_payload);
	}

	char *body = json_stringify(payload);

	Response response = api_request(client.token, path, "POST", body);
	response_free(&response);
	json_free(payload);
	free(body);
}

void add_field_to_embed(struct Embed *embed, char *name, char *value, bool is_inline) {
	++embed->field_size;
	embed->fields = allocate(embed->fields, embed->field_size - 1, embed->field_size, sizeof(struct EmbedField));

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

void add_embed_to_message(const struct Embed embed, struct Message *message) {
	++message->embed_size;
	message->embeds = allocate(message->embeds, message->embed_size - 1, message->embed_size, sizeof(struct Embed));
	message->embeds[message->embed_size - 1] = embed;
}

void free_message(struct Message message) {
	free(message.embeds);
}