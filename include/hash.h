#ifndef HASH_H_
	#define HASH_H_

	struct Node {
		char *key;
		void *value;
		unsigned int size;
		struct Node *next;
	};

	struct HashMap {
		struct Node **nodes;
		unsigned int size; // max size of HashMap
		unsigned int length; // Total node count (includes node->next) of HashMap
		unsigned int count; // Node count of HashMap.nodes
	};

	unsigned int hash(const char *key, unsigned int size);
	struct HashMap *create_hashmap(unsigned int size);
	void free_hashmap(struct HashMap *map);
	void free_hashmap_nodes(struct HashMap *map);
	void expand_hashmap(struct HashMap *map);
	struct Node *get_node(const struct HashMap *map, const char *key);
	void insert_node(struct HashMap *map, const char *key, void *value, const unsigned int size);
	void delete_node(struct HashMap *map, const char *key);
#endif
