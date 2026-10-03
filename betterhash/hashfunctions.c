#include "hashfunctions.h"
#include <string.h>

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

static inline uint64_t rotl64(uint64_t x, int r) {
  return (x << r) | (x >> (64 - r));
}

// memcpy avoids unaligned reads; assumes a little-endian CPU (x86, ARM)
static inline uint64_t read64(const unsigned char *p) {
  uint64_t value;
  memcpy(&value, p, sizeof(value));
  return value;
}

static inline uint32_t read32(const unsigned char *p) {
  uint32_t value;
  memcpy(&value, p, sizeof(value));
  return value;
}

// ---------------------------------------------------------------------------
// naive hashes
// ---------------------------------------------------------------------------

uint64_t hash(const char *data, size_t length) {
  uint64_t hash_value = 0;
  for (size_t i = 0; i < length; i++) {
    hash_value += data[i];
    hash_value = hash_value * data[i];
  }

  return hash_value;
}

uint64_t hash_checksum(const char *data, size_t length) {
  uint64_t hash_value = 0;
  for (size_t i = 0; i < length; i++) {
    hash_value += (unsigned char)data[i];
  }

  return hash_value;
}

// ---------------------------------------------------------------------------
// djb2 (Dan Bernstein): hash * 33 + c
// ---------------------------------------------------------------------------

uint64_t hash_djb2(const char *data, size_t length) {
  uint64_t hash_value = 5381;
  for (size_t i = 0; i < length; i++) {
    hash_value = (hash_value << 5) + hash_value + (unsigned char)data[i];
  }

  return hash_value;
}

// ---------------------------------------------------------------------------
// FNV (Fowler-Noll-Vo)
// ---------------------------------------------------------------------------

constexpr uint64_t FNV_PRIME = 0x100000001b3;
constexpr uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325;

uint64_t hash_fnv1(const char *data, size_t length) {
  uint64_t hash_value = FNV_OFFSET_BASIS;
  for (size_t i = 0; i < length; i++) {
    hash_value *= FNV_PRIME;
    hash_value ^= (unsigned char)data[i];
  }

  return hash_value;
}

uint64_t hash_fnv1a(const char *data, size_t length) {
  uint64_t hash_value = FNV_OFFSET_BASIS;
  for (size_t i = 0; i < length; i++) {
    hash_value ^= (unsigned char)data[i];
    hash_value *= FNV_PRIME;
  }

  return hash_value;
}

// ---------------------------------------------------------------------------
// MurmurHash3 x64_128 (Austin Appleby), seed 0, returns the low 64 bits
// ---------------------------------------------------------------------------

static inline uint64_t murmur3_fmix64(uint64_t k) {
  k ^= k >> 33;
  k *= 0xff51afd7ed558ccd;
  k ^= k >> 33;
  k *= 0xc4ceb9fe1a85ec53;
  k ^= k >> 33;
  return k;
}

uint64_t hash_murmur3(const char *data, size_t length) {
  constexpr uint64_t c1 = 0x87c37b91114253d5;
  constexpr uint64_t c2 = 0x4cf5ad432745937f;
  const unsigned char *bytes = (const unsigned char *)data;
  const size_t nblocks = length / 16;

  uint64_t h1 = 0;
  uint64_t h2 = 0;

  // body: 16 bytes at a time
  for (size_t i = 0; i < nblocks; i++) {
    uint64_t k1 = read64(bytes + i * 16);
    uint64_t k2 = read64(bytes + i * 16 + 8);

    k1 *= c1; k1 = rotl64(k1, 31); k1 *= c2; h1 ^= k1;
    h1 = rotl64(h1, 27); h1 += h2; h1 = h1 * 5 + 0x52dce729;

    k2 *= c2; k2 = rotl64(k2, 33); k2 *= c1; h2 ^= k2;
    h2 = rotl64(h2, 31); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
  }

  // tail: the last 0..15 bytes
  const unsigned char *tail = bytes + nblocks * 16;
  uint64_t k1 = 0;
  uint64_t k2 = 0;
  switch (length & 15) {
    case 15: k2 ^= (uint64_t)tail[14] << 48; [[fallthrough]];
    case 14: k2 ^= (uint64_t)tail[13] << 40; [[fallthrough]];
    case 13: k2 ^= (uint64_t)tail[12] << 32; [[fallthrough]];
    case 12: k2 ^= (uint64_t)tail[11] << 24; [[fallthrough]];
    case 11: k2 ^= (uint64_t)tail[10] << 16; [[fallthrough]];
    case 10: k2 ^= (uint64_t)tail[9] << 8;   [[fallthrough]];
    case 9:
      k2 ^= (uint64_t)tail[8];
      k2 *= c2; k2 = rotl64(k2, 33); k2 *= c1; h2 ^= k2;
      [[fallthrough]];
    case 8: k1 ^= (uint64_t)tail[7] << 56; [[fallthrough]];
    case 7: k1 ^= (uint64_t)tail[6] << 48; [[fallthrough]];
    case 6: k1 ^= (uint64_t)tail[5] << 40; [[fallthrough]];
    case 5: k1 ^= (uint64_t)tail[4] << 32; [[fallthrough]];
    case 4: k1 ^= (uint64_t)tail[3] << 24; [[fallthrough]];
    case 3: k1 ^= (uint64_t)tail[2] << 16; [[fallthrough]];
    case 2: k1 ^= (uint64_t)tail[1] << 8;  [[fallthrough]];
    case 1:
      k1 ^= (uint64_t)tail[0];
      k1 *= c1; k1 = rotl64(k1, 31); k1 *= c2; h1 ^= k1;
  }

  // finalization: mixes every input bit into every output bit
  h1 ^= length;
  h2 ^= length;
  h1 += h2;
  h2 += h1;
  h1 = murmur3_fmix64(h1);
  h2 = murmur3_fmix64(h2);
  h1 += h2;

  return h1;
}

// ---------------------------------------------------------------------------
// xxHash64 (Yann Collet), seed 0
// ---------------------------------------------------------------------------

constexpr uint64_t XXH_PRIME1 = 0x9E3779B185EBCA87;
constexpr uint64_t XXH_PRIME2 = 0xC2B2AE3D27D4EB4F;
constexpr uint64_t XXH_PRIME3 = 0x165667B19E3779F9;
constexpr uint64_t XXH_PRIME4 = 0x85EBCA77C2B2AE63;
constexpr uint64_t XXH_PRIME5 = 0x27D4EB2F165667C5;

static inline uint64_t xxh64_round(uint64_t acc, uint64_t input) {
  acc += input * XXH_PRIME2;
  acc = rotl64(acc, 31);
  return acc * XXH_PRIME1;
}

static inline uint64_t xxh64_merge_round(uint64_t acc, uint64_t value) {
  acc ^= xxh64_round(0, value);
  return acc * XXH_PRIME1 + XXH_PRIME4;
}

uint64_t hash_xxh64(const char *data, size_t length) {
  constexpr uint64_t seed = 0;
  const unsigned char *p = (const unsigned char *)data;
  const unsigned char *end = p + length;
  uint64_t h;

  if (length >= 32) {
    // four independent lanes, so the CPU can work on them in parallel
    uint64_t v1 = seed + XXH_PRIME1 + XXH_PRIME2;
    uint64_t v2 = seed + XXH_PRIME2;
    uint64_t v3 = seed;
    uint64_t v4 = seed - XXH_PRIME1;
    const unsigned char *limit = end - 32;
    do {
      v1 = xxh64_round(v1, read64(p));      p += 8;
      v2 = xxh64_round(v2, read64(p));      p += 8;
      v3 = xxh64_round(v3, read64(p));      p += 8;
      v4 = xxh64_round(v4, read64(p));      p += 8;
    } while (p <= limit);

    h = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
    h = xxh64_merge_round(h, v1);
    h = xxh64_merge_round(h, v2);
    h = xxh64_merge_round(h, v3);
    h = xxh64_merge_round(h, v4);
  } else {
    h = seed + XXH_PRIME5;
  }

  h += length;

  // remaining bytes: 8, then 4, then 1 at a time
  for (; p + 8 <= end; p += 8) {
    h ^= xxh64_round(0, read64(p));
    h = rotl64(h, 27) * XXH_PRIME1 + XXH_PRIME4;
  }
  if (p + 4 <= end) {
    h ^= (uint64_t)read32(p) * XXH_PRIME1;
    h = rotl64(h, 23) * XXH_PRIME2 + XXH_PRIME3;
    p += 4;
  }
  for (; p < end; p++) {
    h ^= (*p) * XXH_PRIME5;
    h = rotl64(h, 11) * XXH_PRIME1;
  }

  // avalanche
  h ^= h >> 33;
  h *= XXH_PRIME2;
  h ^= h >> 29;
  h *= XXH_PRIME3;
  h ^= h >> 32;
  return h;
}

// ---------------------------------------------------------------------------
// SipHash-2-4 (Aumasson & Bernstein)
// ---------------------------------------------------------------------------

// fixed key (bytes 00..0f, the one from the paper's test vectors).
// a real hash table picks a random key at startup, so attackers cannot
// predict which keys collide.
static const unsigned char SIPHASH_KEY[16] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

#define SIPROUND                                              \
  do {                                                        \
    v0 += v1; v1 = rotl64(v1, 13); v1 ^= v0; v0 = rotl64(v0, 32); \
    v2 += v3; v3 = rotl64(v3, 16); v3 ^= v2;                  \
    v0 += v3; v3 = rotl64(v3, 21); v3 ^= v0;                  \
    v2 += v1; v1 = rotl64(v1, 17); v1 ^= v2; v2 = rotl64(v2, 32); \
  } while (0)

uint64_t hash_siphash24(const char *data, size_t length) {
  const unsigned char *p = (const unsigned char *)data;
  const uint64_t k0 = read64(SIPHASH_KEY);
  const uint64_t k1 = read64(SIPHASH_KEY + 8);

  uint64_t v0 = 0x736f6d6570736575 ^ k0;
  uint64_t v1 = 0x646f72616e646f6d ^ k1;
  uint64_t v2 = 0x6c7967656e657261 ^ k0;
  uint64_t v3 = 0x7465646279746573 ^ k1;

  // 2 compression rounds per 8-byte block
  const size_t nblocks = length / 8;
  for (size_t i = 0; i < nblocks; i++) {
    uint64_t m = read64(p + i * 8);
    v3 ^= m;
    SIPROUND;
    SIPROUND;
    v0 ^= m;
  }

  // last block: remaining bytes plus the length in the top byte
  uint64_t b = (uint64_t)length << 56;
  const unsigned char *tail = p + nblocks * 8;
  for (size_t i = 0; i < (length & 7); i++) {
    b |= (uint64_t)tail[i] << (8 * i);
  }
  v3 ^= b;
  SIPROUND;
  SIPROUND;
  v0 ^= b;

  // 4 finalization rounds
  v2 ^= 0xff;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  return v0 ^ v1 ^ v2 ^ v3;
}

#undef SIPROUND
