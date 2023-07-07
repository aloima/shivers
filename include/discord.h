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

	void create_caches();
	void clear_cache(Cache *cache);
	void add_to_cache(Cache *cache, const char *data);
	size_t get_cache_size(Cache *cache);
	void remove_from_cache_index(Cache *cache, size_t index);
	const char *get_cache_data_from_index(Cache *cache, size_t index);

	Cache *get_guilds_cache();

	const unsigned int get_all_intents();
	Response api_request(char *token, char *path, char *method, char *content);

	void on_ready(Client client);
	void on_message_create(Client client, JSONElement *message);
#endif
