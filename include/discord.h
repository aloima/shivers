#include <stddef.h>

#include <json.h>
#include <network.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	typedef struct {
		char **data;
		size_t size;
	} Cache;

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

	struct Embed {
		char *title;
		char *description;
		unsigned int color;
		char *image_url;
		char *thumbnail_url;
		struct EmbedAuthor author;
		struct EmbedField *fields;
		short field_size;
	};

	struct Message {
		char *content;
		struct Embed *embeds;
		size_t embed_size;
	};

	void connect_gateway(const char *token);
	int get_latency();
	void set_presence(const char *name, const char type, const char *status);

	void create_caches();
	void clear_cache(Cache *cache);
	void add_to_cache(Cache *cache, const char *data);
	void remove_from_cache_index(Cache *cache, const size_t index);

	Cache *get_guilds_cache();

	unsigned int get_all_intents();
	struct Response api_request(const char *token, const char *path, const char *method, const char *body);

	void send_message(const struct Client client, const char *channel_id, const struct Message message);
	void free_message(struct Message message);
	void add_field_to_embed(struct Embed *embed, char *name, char *value, bool is_inline);
	void set_embed_author(struct Embed *embed, char *name, char *url, char *icon_url);
	void add_embed_to_message(const struct Embed embed, struct Message *message);

	bool check_snowflake(const char *snowflake);
#endif
