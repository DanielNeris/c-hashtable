#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdlib.h>
#include <stdint.h>

typedef uint64_t (hashfunction) (const char*, size_t);
typedef void cleanupfunction(void *);
typedef struct hash_table hash_table;

hash_table *hash_table_create(uint32_t size, hashfunction *hf, cleanupfunction *cf);
void hash_table_destroy(hash_table *ht);
void hash_table_print(const hash_table *ht);
bool hash_table_insert(hash_table *ht, const char *key, void *obj);
void *hash_table_lookup(const hash_table *ht, const char *key);
void *hash_table_delete(hash_table *ht, const char *key);
uint64_t hash_table_collisions(const hash_table *ht);

#endif
