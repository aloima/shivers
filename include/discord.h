#include <stddef.h>

#include <json.h>
#include <network.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	void connect_gateway(const char *token);

	typedef struct {
		char **data;
		size_t size;
	} Cache;

	typedef struct {
		JSONElement *user;
		char *token;
	} Client;

	typedef struct {
		char *name;
		char *value;
		bool inline_mode;
	} EmbedField;

	typedef struct {
		char *title;
		char *description;
		unsigned int color;
		char *image_url;
		EmbedField *fields;
		short field_size;
	} Embed;

	void create_caches();
	void clear_cache(Cache *cache);
	void add_to_cache(Cache *cache, const char *data);
	size_t get_cache_size(Cache *cache);
	void remove_from_cache_index(Cache *cache, size_t index);
	const char *get_cache_data_from_index(Cache *cache, size_t index);

	Cache *get_guilds_cache();

	const unsigned int get_all_intents();
	Response api_request(char *token, char *path, char *method, char *body);
	void send_content(Client client, const char *channel_id, const char *content);
	void send_embed(Client client, const char *channel_id, Embed embed);
	bool check_snowflake(char *snowflake);
#endif
