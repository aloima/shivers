#include <stddef.h>
#include <stdbool.h>

#include <json.h>
#include <network.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	#define AVATAR_URL "https://cdn.discordapp.com/avatars/%s/%s.%s?size=1024"
	#define DEFAULT_AVATAR_URL "https://cdn.discordapp.com/embed/avatars/%d.png?size=1024"

	struct Cache {
		char **data;
		size_t size;
	};

	struct Client {
		jsonelement_t *user;
		unsigned long ready_at;
		char *token;
	};

	struct EmbedField {
		char *name;
		char *value;
		bool is_inline;
	};

	struct EmbedAuthor {
		char *name;
		char *url;
		char *icon_url;
	};

	struct EmbedFooter {
		char *text;
		char *icon_url;
	};

	struct Embed {
		char *title;
		char *description;
		double color;
		char *image_url;
		char *thumbnail_url;
		struct EmbedAuthor author;
		struct EmbedFooter footer;
		struct EmbedField *fields;
		short field_size;
	};

	struct File {
		char *name;
		char *data;
		char *type;
		size_t size;
	};

	struct Message {
		char *content;
		struct Embed *embeds;
		size_t embed_size;
		struct File *files;
		size_t file_size;
	};

	void connect_gateway(const char *token);
	int get_latency();
	void set_presence(const char *name, const char type, const char *status);

	void clear_cache(struct Cache *cache);
	void add_to_cache(struct Cache *cache, const char *data);
	void remove_from_cache_index(struct Cache *cache, const size_t index);

	struct Cache *get_guilds_cache();

	unsigned int get_all_intents();
	struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata);
	void get_avatar_url(char *url, const char *token, const char *user_id, const char *discriminator, const char *hash, const bool force_png);

	void send_message(const struct Client client, const char *channel_id, const struct Message message);
	void free_message(struct Message message);
	void add_field_to_embed(struct Embed *embed, const char *name, const char *value, const bool is_inline);
	void set_embed_author(struct Embed *embed, const char *name, const char *url, const char *icon_url);
	void set_embed_footer(struct Embed *embed, const char *text, const char *icon_url);
	void add_embed_to_message(const struct Embed embed, struct Message *message);
	void add_file_to_message(struct Message *message, const char *name, const char *data, const size_t size, const char *type);

	bool check_snowflake(const char *snowflake);
#endif
