#include "hashtable.h"
#include "hashfunctions.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

constexpr int MAX_LINE_LENGTH = 4096;

static void generate_random_word(char *buffer, size_t length) {
  for (size_t i = 0; i < length-1; i++) {
    buffer[i] = 'a' + (rand() % 26);
  }
  buffer[length-1] = 0;
}

// static void mycleanup(void *p) {
//   printf("destroying %p\n", p);
//   free(p);
// }

typedef struct {
  const char *name;
  hashfunction *function;
} hash_candidate;

typedef struct {
  uint32_t words;
  uint64_t collisions;
  uint32_t good_guesses;
  double seconds;
} benchmark_result;

static double now_in_seconds(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e9;
}

// loads every word into a fresh table, then makes num_guesses random lookups
static bool run_benchmark(const char *filename, hashfunction *hf, uint32_t num_guesses, benchmark_result *result) {
  const int tablesize = (1<<20);
  double start = now_in_seconds();

  hash_table *table = hash_table_create(tablesize, hf, nullptr);
  if (table == nullptr) {
    fprintf(stderr, "could not create the hash table\n");
    return false;
  }

  FILE *fp = fopen(filename, "r");
  if (fp == nullptr) {
    fprintf(stderr, "could not open %s\n", filename);
    hash_table_destroy(table);
    return false;
  }

  char buffer[MAX_LINE_LENGTH];
  uint32_t numwords = 0;
  while (fgets(buffer, MAX_LINE_LENGTH, fp) != nullptr) {
    buffer[strcspn(buffer, "\n\r")] = 0;
    char *newentry = strdup(buffer);
    if (newentry == nullptr) continue;

    // duplicates are rejected, so the table never takes ownership of them
    if (!hash_table_insert(table, newentry, newentry)) {
      free(newentry);
      continue;
    }
    numwords++;
  }
  fclose(fp);

  // same seed for every function, so they all get the same guesses
  srand(1);
  uint32_t good_guesses = 0;
  const int shortest_guess = 2;
  const int longest_guess = 15;
  for (uint32_t i = 0; i < num_guesses; i++) {
    generate_random_word(buffer, shortest_guess + (rand() % (longest_guess - shortest_guess)));
    if (hash_table_lookup(table, buffer)) {
      good_guesses++;
    }
  }

  result->words = numwords;
  result->collisions = hash_table_collisions(table);
  result->good_guesses = good_guesses;

  hash_table_destroy(table);
  result->seconds = now_in_seconds() - start;
  return true;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s <wordlist filename> <num guesses>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *filename = argv[1];
  uint32_t num_guesses = atol(argv[2]);

  const hash_candidate candidates[] = {
    {"hash",           hash},
    {"hash_checksum",  hash_checksum},
    {"hash_djb2",      hash_djb2},
    {"hash_fnv1",      hash_fnv1},
    {"hash_fnv1a",     hash_fnv1a},
    {"hash_murmur3",   hash_murmur3},
    {"hash_xxh64",     hash_xxh64},
    {"hash_siphash24", hash_siphash24},
  };
  constexpr size_t num_candidates = sizeof(candidates) / sizeof(candidates[0]);

  for (size_t i = 0; i < num_candidates; i++) {
    benchmark_result result;
    if (!run_benchmark(filename, candidates[i].function, num_guesses, &result)) {
      return EXIT_FAILURE;
    }
    if (i == 0) {
      printf("Loaded %u words, %u guesses\n\n", result.words, num_guesses);
      printf("%-15s %12s %12s %10s\n", "Function", "Collisions", "Hits", "Time");
    }
    printf("%-15s %12" PRIu64 " %12u %9.2fs\n",
           candidates[i].name, result.collisions, result.good_guesses, result.seconds);
  }

  return EXIT_SUCCESS;
}
