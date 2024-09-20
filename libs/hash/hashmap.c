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
  free(map);
}

void expand_hashmap(struct HashMap *map) {
  struct HashMap *new = create_hashmap(map->size * 2);

  for (unsigned int i = 0; i < map->size; ++i) {
    struct Node *node = map->nodes[i];
    struct Node *next = node->next;
    node->next = NULL;

    while (node) {
      const unsigned int index = hash(node->key, new->size);
      struct Node *area = new->nodes[index];
      ++new->length;

      if (area == NULL) {
        ++new->count;
        new->nodes[index] = node;
      } else {
        while (area->next != NULL) {
          area = area->next;
        }

        area->next = node;
      }

      node = next;

      if (node != NULL) {
        next = node->next;
        node->next = NULL;
      }
    }
  }

  free(map->nodes);

  map->nodes = new->nodes;
  map->count = new->count;
  map->length = new->length;
  map->size = new->size;

  free(new);
}

struct Node *get_node(const struct HashMap *map, const char *key) {
  const unsigned int index = hash(key, map->size);
  struct Node *node = map->nodes[index];

  while (node && !streq(key, node->key)) {
    node = node->next;
  }

  if (node && streq(key, node->key)) {
    return node;
  } else {
    return NULL;
  }
}

void insert_node(struct HashMap *map, const char *key, void *value, const unsigned int size) {
  const unsigned int index = hash(key, map->size);
  struct Node *area = map->nodes[index];

  if (area != NULL && map->count == map->size) {
    expand_hashmap(map);
  }

  const unsigned int key_size = strlen(key) + 1;

  struct Node *node = allocate(NULL, -1, 1, sizeof(struct Node));
  node->key = allocate(NULL, -1, key_size, sizeof(char));
  node->value = allocate(NULL, -1, 1, size);
  node->size = size;
  node->next = NULL;

  memcpy(node->key, key, key_size);
  memcpy(node->value, value, size);

  if (area == NULL) {
    ++map->count;
    ++map->length;
    map->nodes[index] = node;
  } else {
    ++map->length;

    while (area->next != NULL) {
      area = area->next;
    }

    area->next = node;
  }
}

void delete_node(struct HashMap *map, const char *key) {
  const unsigned int index = hash(key, map->size);
  struct Node **node = &map->nodes[index];
  struct Node *prev = NULL;

  if (*node != NULL) {
    while (!streq((*node)->key, key)) {
      prev = *node;
      node = &(*node)->next;
    }

    if (streq((*node)->key, key)) {
      struct Node *last = NULL;
      while ((*node)->next) last = (*node)->next;

      if (last) {
        last->next = prev->next->next;
        prev->next = last;
      }

      --map->count;
    }

    --map->length;
    free((*node)->key);
    free((*node)->value);
    free(*node);
    *node = NULL;
  }
}
