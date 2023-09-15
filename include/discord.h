#include <stddef.h>

#include <json.h>
#include <network.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	typedef struct {
		char **data;
		size_t size;
	} Cache;

	typedef struct {
		jsonelement_t *user;
		unsigned long ready_at;
		char *token;
	} Client;

	typedef struct {
		char *name;
		char *value;
		bool inline_mode;
	} EmbedField;

	typedef struct {
		char *name;
		char *url;
		char *icon_url;
	} EmbedAuthor;

	typedef struct {
		char *title;
		char *description;
		unsigned int color;
		char *image_url;
		char *thumbnail_url;
		EmbedAuthor author;
		EmbedField *fields;
		short field_size;
	} Embed;

	void connect_gateway(const char *token);
	int get_latency();

	void create_caches();
	void clear_cache(Cache *cache);
	void add_to_cache(Cache *cache, const char *data);
	void remove_from_cache_index(Cache *cache, const size_t index);

	Cache *get_guilds_cache();

	const unsigned int get_all_intents();
	Response api_request(char *token, char *path, char *method, char *body);

	void send_content(Client client, const char *channel_id, const char *content);
	void send_embed(Client client, const char *channel_id, Embed embed);
	void add_field_to_embed(Embed *embed, char *name, char *value, bool inline_mode);
	void set_embed_author(Embed *embed, char *name, char *url, char *icon_url);

	bool check_snowflake(char *snowflake);
#endif
