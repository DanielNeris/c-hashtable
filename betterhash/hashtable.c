#include "hashtable.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct entry {
  char *key;
  void *object;
  struct entry *next;
} entry;

struct hash_table {
  uint32_t size;
  hashfunction *hash;
  cleanupfunction *cleanup;
  entry **elements;
  uint64_t collisions;
};

static size_t hash_table_index(const hash_table *ht, const char *key) {
  return ht->hash(key, strlen(key)) % ht->size;
}

// pass NULL cf for default free behavior
hash_table *hash_table_create(uint32_t size, hashfunction *hf, cleanupfunction *cf) {
  hash_table *ht = malloc(sizeof(*ht));
  if (ht == nullptr) return nullptr;

  ht->size = size;
  ht->hash = hf;
  ht->cleanup = cf ? cf : free;
  ht->collisions = 0;
  // note that calloc zeros out the memory
  ht->elements = calloc(ht->size, sizeof(entry*));
  if (ht->elements == nullptr) {
    free(ht);
    return nullptr;
  }
  return ht;
}

void hash_table_destroy(hash_table *ht) {
  if (ht == nullptr) return;

  //clean up individual elements
  for (uint32_t i = 0; i < ht->size; i++) {
    while (ht->elements[i] != nullptr) {
      entry *tmp = ht->elements[i];
      ht->elements[i] = tmp->next;
      free(tmp->key);
      ht->cleanup(tmp->object);
      free(tmp);
    }
  }
  free(ht->elements);
  free(ht);
}

void hash_table_print(const hash_table *ht) {
  printf("Start Table\n");
  for (uint32_t i = 0; i < ht->size; i++) {
    if (ht->elements[i] == nullptr) continue;

    printf("\t%u\t", i);
    for (entry *e = ht->elements[i]; e != nullptr; e = e->next) {
      printf("\"%s\"(%p) -> ", e->key, e->object);
    }
    printf("\n");
  }
  printf("End Table\n");
}

bool hash_table_insert(hash_table *ht, const char *key, void *obj) {
  if (ht == nullptr || key == nullptr || obj == nullptr) return false;
  if (hash_table_lookup(ht, key) != nullptr) return false;

  //create a new entry
  entry *e = malloc(sizeof(*e));
  if (e == nullptr) return false;

  e->key = strdup(key);
  if (e->key == nullptr) {
    free(e);
    return false;
  }
  e->object = obj;

  //insert entry at the head of the list
  size_t index = hash_table_index(ht, key);
  if (ht->elements[index] != nullptr) ht->collisions++;
  e->next = ht->elements[index];
  ht->elements[index] = e;
  return true;
}

void *hash_table_lookup(const hash_table *ht, const char *key) {
  if (ht == nullptr || key == nullptr) return nullptr;

  size_t index = hash_table_index(ht, key);
  for (entry *e = ht->elements[index]; e != nullptr; e = e->next) {
    if (strcmp(e->key, key) == 0) return e->object;
  }
  return nullptr;
}

void *hash_table_delete(hash_table *ht, const char *key) {
  if (ht == nullptr || key == nullptr) return nullptr;

  size_t index = hash_table_index(ht, key);
  // link points at the pointer that leads to the current entry:
  // either the slot itself or the previous entry's next
  entry **link = &ht->elements[index];
  while (*link != nullptr && strcmp((*link)->key, key) != 0) {
    link = &(*link)->next;
  }
  if (*link == nullptr) return nullptr;

  entry *found = *link;
  *link = found->next;
  // the slot still has entries, so the removed one was a collision
  if (ht->elements[index] != nullptr) ht->collisions--;

  void *result = found->object;
  free(found->key);
  free(found);
  return result;
}

// counts every entry that is not the first in its slot
uint64_t hash_table_collisions(const hash_table *ht) {
  if (ht == nullptr) return 0;
  return ht->collisions;
}
