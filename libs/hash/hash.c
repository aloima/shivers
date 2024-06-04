#include <stdlib.h>
#include <string.h>

#include <hash.h>
#include <utils.h>

unsigned int hash(const char *key, const unsigned int size) {
	unsigned long hash = 5381;
	char c;

	while ((c = *key++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash % size;
}

struct HashMap *create_hashmap(const unsigned int size) {
	struct HashMap *map = allocate(NULL, -1, 1, sizeof(struct HashMap));
	map->nodes = allocate(NULL, 0,size, sizeof(struct Node));
	map->size = size;
	map->count = 0;
	map->length = 0;

	return map;
}

void free_hashmap(struct HashMap *map) {
	free_hashmap_nodes(map);
	free(map);
}

void free_hashmap_nodes(struct HashMap *map) {
	for (unsigned int i = 0; i < map->size; ++i) {
		struct Node *node = map->nodes[i];

		while (node) {
			struct Node *temp = node->next;
			free(node->key);
			free(node->value);
			free(node);
			node = temp;
		}
	}

	free(map->nodes);
}

void expand_hashmap(struct HashMap *map) {
	struct HashMap *new = create_hashmap(map->size * 2);

	for (unsigned int i = 0; i < map->size; ++i) {
		struct Node *node = map->nodes[i];

		while (node) {
			insert_node(new, node->key, node->value, node->size);
			node = node->next;
		}
	}

	free_hashmap_nodes(map);
	map->nodes = new->nodes;
	map->count = new->count;
	map->length = new->length;
	map->size = new->size;

	free(new);
}

struct Node *get_node(const struct HashMap *map, const char *key) {
	const unsigned int index = hash(key, map->size);
	struct Node *node = map->nodes[index];

	while (!strsame(key, node->key)) {
		node = node->next;
	}

	return node;
}

void insert_node(struct HashMap *map, const char *key, void *value, const unsigned int size) {
	if (map->count == map->size) {
		expand_hashmap(map);
	}

	const unsigned int index = hash(key, map->size);
	const unsigned int key_size = strlen(key) + 1;

	struct Node *node = allocate(NULL, -1, 1, sizeof(struct Node));
	node->key = allocate(NULL, -1, key_size, sizeof(char));
	node->value = allocate(NULL, -1, 1, size);
	node->size = size;

	memcpy(node->key, key, key_size);
	memcpy(node->value, value, size);

	struct Node *tnode = map->nodes[index];

	if (tnode == NULL) {
		++map->count;
		++map->length;
		map->nodes[index] = node;
	} else {
		++map->length;

		while (tnode->next != NULL) {
			tnode = tnode->next;
		}

		tnode->next = node;
	}
}

void delete_node(struct HashMap *map, const char *key) {
	const unsigned int index = hash(key, map->size);
	struct Node *node = map->nodes[index];
	struct Node *prev = NULL;

	while (!strsame(node->key, key)) {
		prev = node;
		node = node->next;
	}

	if (strsame(node->key, key)) {
		struct Node *last = NULL;
		while (node->next) last = node->next;

		if (last) {
			last->next = prev->next->next;
			prev->next = last;
		}

		--map->count;
	}

	--map->length;
	free(node->key);
	free(node->value);
	free(node);
}