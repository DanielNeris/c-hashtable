# c-hashtable

Hash tables in C23, standard library only. Built step by step, from a fixed array of
10 slots to a reusable library with a benchmark.

| Path                 | What it is                                                        |
| -------------------- | ----------------------------------------------------------------- |
| `main.c`             | fixed table, collisions handled with linear probing and tombstones |
| `main_linked_list.c` | same table, collisions handled with linked lists                  |
| `betterhash/`        | reusable hash table library plus a benchmark                      |

Requires CMake 3.21+ and a C23 compiler (recent clang or gcc).

---

## First versions

```bash
cmake -B build
cmake --build build
./build/hashtable
./build/hashtable_linked_list
```

Both store `Person` structs by name in a 10-slot table and run the same demo: insert,
look up, delete.

- **Linear probing** moves a colliding name to the next free slot. Deleting leaves a
  tombstone (`DELETED_NODE`) so later lookups keep searching past the gap. The table
  holds at most 10 people.
- **Linked lists** keep every name that lands on a slot in a list there. No tombstones,
  no size limit.

---

## betterhash

A general-purpose hash table: string keys, `void *` values, and the hash function chosen
by the caller.

```bash
cd betterhash
cmake -B build
cmake --build build
./build/test words.txt 1000000    # word list, number of random lookups
```

It builds `libhashtable.a`, `libhashtable.dylib` (`.so` on Linux), and `test`.

```c
hash_table *table = hash_table_create(1 << 20, hash_fnv1a, nullptr);
hash_table_insert(table, "apple", obj);
void *found = hash_table_lookup(table, "apple");
hash_table_delete(table, "apple");
hash_table_destroy(table);
```

- **Opaque type.** `hashtable.h` only declares `hash_table`; its fields live in
  `hashtable.c`, so callers cannot reach inside.
- **Chaining.** Each slot holds a linked list. Delete uses a pointer to pointer, so
  removing the head and removing from the middle are the same code.
- **Ownership.** The table copies keys and frees them itself. Values are freed through a
  cleanup function passed to `create` (`nullptr` means `free`).
- **Collision counter.** Updated on every insert and delete, so
  `hash_table_collisions()` is instant.

### Benchmark

`test` loads every word into a table of 1,048,576 slots, makes random lookups, and
reports collisions and time for each hash function in `hashfunctions.c`.

With the 235,976-word list, a perfectly random hash would give about 24,700 collisions.
FNV-1a lands right there. The first hand-written `hash` gets 29,412, and simply summing
the characters gets 233,846, because short words can only add up to a few thousand
different values.

Memory is checked with `leaks --atExit -- ./build/test words.txt 1000` on macOS
(`valgrind` on Linux): 0 leaks.

---

## Background

- C reference: https://en.cppreference.com/w/c
- Hash table: https://en.wikipedia.org/wiki/Hash_table
- List of hash functions: https://en.wikipedia.org/wiki/List_of_hash_functions
