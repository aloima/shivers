#include <string.h>
#include <stdlib.h>

#include <discord.h>
#include <utils.h>

static Cache *guilds = NULL;

void create_caches() {
	guilds = allocate(NULL, 0, 1, sizeof(Cache));
}

void clear_cache(Cache *cache) {
	if (cache != NULL) {
		for (size_t i = 0; i < cache->size; ++i) {
			free(cache->data[i]);
		}

		free(cache->data);
		cache->size = 0;
		free(cache);
	}
}

void add_to_cache(Cache *cache, const char *data) {
	++cache->size;
	cache->data = allocate(cache->data, cache->size - 1, cache->size, sizeof(char *));
	cache->data[cache->size - 1] = allocate(NULL, 0, strlen(data) + 1, sizeof(char));
	strcpy(cache->data[cache->size - 1], data);
}

void remove_from_cache_index(Cache *cache, const size_t index) {
	for (size_t i = (index + 1); i < cache->size; ++i) {
		cache->data[i - 1] = allocate(cache->data[i - 1], strlen(cache->data[i - 1]) + 1, strlen(cache->data[i]) + 1, sizeof(char));
		strcpy(cache->data[i - 1], cache->data[i]);
	}

	--cache->size;
	cache->data = allocate(cache->data, cache->size + 1, cache->size, sizeof(char *));
}

Cache *get_guilds_cache() {
	return guilds;
}
