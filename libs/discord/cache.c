#include <string.h>
#include <stdlib.h>

#include <discord.h>
#include <utils.h>

static Cache *guilds;

void create_caches() {
	guilds = allocate(NULL, sizeof(Cache), 1);
}

void clear_cache(Cache *cache) {
	for (size_t i = 0; i < cache->size; ++i) {
		free(cache->data[i]);
	}

	free(cache->data);
	cache->size = 0;
}

void add_to_cache(Cache *cache, const char *data) {
	++cache->size;
	cache->data = allocate(cache->data, cache->size, sizeof(char *));
	cache->data[cache->size - 1] = allocate(NULL, strlen(data) + 1, sizeof(char));
	strcpy(cache->data[cache->size - 1], data);
}

void remove_from_cache_index(Cache *cache, size_t index) {
	for (size_t i = (index + 1); i < get_cache_size(cache); ++i) {
		cache->data[i - 1] = allocate(cache->data[i - 1], strlen(cache->data[i]) + 1, sizeof(char));
		strcpy(cache->data[i - 1], cache->data[i]);
	}

	--cache->size;
	cache->data = allocate(cache->data, cache->size, sizeof(char *));
}

size_t get_cache_size(Cache *cache) {
	return cache->size;
}

const char *get_cache_data_from_index(Cache *cache, size_t index) {
	return cache->data[index];
}

Cache *get_guilds_cache() {
	return guilds;
}
