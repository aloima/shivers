#include <string.h>
#include <stdlib.h>

#include <discord.h>
#include <utils.h>

static struct Cache guilds = {0};

void clear_cache(struct Cache *cache) {
	if (cache != NULL) {
		for (size_t i = 0; i < cache->size; ++i) {
			free(cache->data[i]);
		}

		free(cache->data);
		cache->size = 0;
	}
}

void add_to_cache(struct Cache *cache, const char *data) {
	++cache->size;
	cache->data = allocate(cache->data, -1, cache->size, sizeof(char *));
	cache->data[cache->size - 1] = allocate(NULL, -1, strlen(data) + 1, sizeof(char));
	strcpy(cache->data[cache->size - 1], data);
}

void remove_from_cache_index(struct Cache *cache, const size_t index) {
	for (size_t i = (index + 1); i < cache->size; ++i) {
		const size_t length = strlen(cache->data[i]);
		cache->data[i - 1] = allocate(cache->data[i - 1], -1, length + 1, sizeof(char));
		strcpy(cache->data[i - 1], cache->data[i]);
	}

	free(cache->data[cache->size - 1]);
	--cache->size;
	cache->data = allocate(cache->data, -1, cache->size, sizeof(char *));
}

struct Cache *get_guilds_cache() {
	return &guilds;
}
