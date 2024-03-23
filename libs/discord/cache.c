#include <string.h>
#include <stdlib.h>

#include <discord.h>
#include <utils.h>

static struct Cache guilds = {0};

void clear_cache(struct Cache *cache) {
	if (cache != NULL) {
		for (unsigned int i = 0; i < cache->size; ++i) {
			free(cache->data[i]);
		}

		free(cache->data);
		cache->size = 0;
	}
}

void add_to_cache(struct Cache *cache, const char *data) {
	const unsigned long data_size = (strlen(data) + 1);

	++cache->size;
	cache->data = allocate(cache->data, -1, cache->size, sizeof(char *));
	cache->data[cache->size - 1] = allocate(NULL, -1, data_size, sizeof(char));
	memcpy(cache->data[cache->size - 1], data, data_size);
}

void remove_from_cache_index(struct Cache *cache, const unsigned int index) {
	for (unsigned int i = (index + 1); i < cache->size; ++i) {
		const unsigned int size = (strlen(cache->data[i]) + 1);
		cache->data[i - 1] = allocate(cache->data[i - 1], -1, size, sizeof(char));
		memcpy(cache->data[i - 1], cache->data[i], size);
	}

	free(cache->data[cache->size - 1]);
	--cache->size;
	cache->data = allocate(cache->data, -1, cache->size, sizeof(char *));
}

struct Cache *get_guilds_cache() {
	return &guilds;
}
