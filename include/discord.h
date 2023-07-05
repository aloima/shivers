#include <stddef.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	void connect_gateway(const char *token);

	typedef struct {
		char **data;
		size_t size;
	} Cache;

	void create_caches();
	void add_to_cache(Cache *cache, const char *data);
	size_t get_cache_size(Cache *cache);
	void remove_from_cache_index(Cache *cache, size_t index);
	const char *get_cache_data_from_index(Cache *cache, size_t index);

	Cache *get_guilds_cache();
#endif
