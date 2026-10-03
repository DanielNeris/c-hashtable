#ifndef HASHFUNCTIONS_H
#define HASHFUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

// every function here matches the hashfunction type from hashtable.h

// naive, for comparison only
uint64_t hash(const char *data, size_t length);
uint64_t hash_checksum(const char *data, size_t length);

// classic simple hashes
uint64_t hash_djb2(const char *data, size_t length);
uint64_t hash_fnv1(const char *data, size_t length);
uint64_t hash_fnv1a(const char *data, size_t length);

// modern fast hashes (Cassandra, Elasticsearch, zstd, LZ4...)
uint64_t hash_murmur3(const char *data, size_t length);
uint64_t hash_xxh64(const char *data, size_t length);

// keyed hash that resists hash flooding (Python, Rust)
uint64_t hash_siphash24(const char *data, size_t length);

#endif
