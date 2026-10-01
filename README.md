# c-hashtable

A hash table in C, standard library only, built twice to compare the two classic ways
of handling collisions: linear probing and linked lists (separate chaining). Each piece
was added to answer a concrete question: how does a name become an index, what does an
empty slot look like, and what happens when two names want the same slot.

```bash
cmake -B build                    # configure, once
cmake --build build               # compile
./build/hashtable                 # linear probing
./build/hashtable_linked_list     # separate chaining
```

---

## The two versions

| File                 | Executable              | Collisions                                       |
| -------------------- | ----------------------- | ------------------------------------------------ |
| `main.c`             | `hashtable`             | linear probing: try the next slot until one is free |
| `main_linked_list.c` | `hashtable_linked_list` | each slot holds a linked list of every person that hashed there |

Both store the same `Person` (`id`, `name`, `age`) in a table of `TABLE_SIZE` (10) slots,
and both run the same demo: insert 11 people, look up "John" and "Nobody", delete "Jane",
then look her up again.

---

## How it evolved

### 1. A hash function

`unsigned int hash(char *name)` walks the name and mixes each character into the result,
keeping it in range with `% TABLE_SIZE`.

The return type is `unsigned` on purpose. The value becomes an array index, so it must
never be negative, and unsigned overflow wraps around in a defined way, while signed
overflow is undefined behavior. It is printed with `%u`: `%d` would read the same bits
as a signed `int`, and any value above 2,147,483,647 would come out negative.

### 2. A table of pointers

`Person *hash_table[TABLE_SIZE]` is an array of 10 pointers to `Person`, not 10 `Person`s.
`NULL` means an empty slot, which a plain struct has no way to express, and inserting stores
an address instead of copying the struct.

Being global, the array starts zeroed, so every slot is already `NULL`. `init_hash_table()`
sets them anyway, so the table can be reset.

### 3. Lookups, and the segfault they exposed

The first version of `print_lookup_result` printed `person->name` before checking
`person == NULL`. Looking up "Nobody" returns `NULL`, and reading a field through it
crashed with a segmentation fault. The `NULL` check has to come before the first `->`.

### 4. Collisions: linear probing (`main.c`)

At first, a second name landing on an occupied slot was simply rejected ("Collision
detected for Jill"). With probing, insert tries `(index + i) % TABLE_SIZE` for each `i`
until it finds a free slot, and lookup follows the same path until it finds the name or
hits `NULL`.

The table now fills up: 10 slots hold 10 people, and the 11th ("Tega") is refused with
"Table is full".

### 5. Deleting with a tombstone

Setting a deleted slot back to `NULL` breaks lookups. If John and Jill both hash to 0 and
Jill was pushed to 1, deleting John leaves slot 0 empty, and a lookup for Jill stops there
before ever reaching her.

The fix is a marker that means "something was here":

```c
#define DELETED_NODE ((Person*)UINTPTR_MAX)
```

`UINTPTR_MAX` is an address no real `Person` will ever have. The cast to `Person *` matters:
without it the macro is an integer, and assigning it to a slot does not compile. Lookup and
delete skip tombstones and keep going, insert reuses them, and `print_table` shows them as
`<deleted>`. Every place that uses `->name` now has to rule out both `NULL` and `DELETED_NODE`.

The first attempt wrote `hash_table[try] == DELETED_NODE;`. That is a comparison whose
result is thrown away, so delete silently did nothing and "Jane" was still found afterwards.
It needed `=`, the assignment.

### 6. Collisions: linked lists (`main_linked_list.c`)

`Person` gains a `struct Person *next` field, and each slot becomes the head of a list.
The struct needs a tag (`typedef struct Person { ... }`) because inside its own definition
the name `Person` does not exist yet.

- **Insert** adds at the head, in constant time. That is why the latest insert is listed first.
- **Lookup** follows `next` until the name matches or the list ends.
- **Delete** tracks the previous node and links around the removed one. Removing the head
  updates the slot itself instead.

No tombstones are needed, and the table never fills: all 11 people fit, with some slots
holding chains like `Test -> Jill -> Jim -> NULL`.

---

## Probing vs linked lists

|            | Linear probing                    | Linked lists                       |
| ---------- | --------------------------------- | ---------------------------------- |
| Capacity   | at most `TABLE_SIZE` people       | unlimited                          |
| Collision  | moves on to the next free slot    | joins the list in the same slot    |
| Delete     | needs a tombstone (`DELETED_NODE`) | relinks the neighbouring pointers  |
| Memory     | the array only                    | one extra `next` pointer per person |

---

## Known limitations

| Limitation                            | Why it matters                                                                                                                       |
| ------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| Weak hash function                    | With 10 slots, 11 names already produce chains of three. A real table would use a well-studied hash such as FNV-1a.                 |
| Fixed size, no resizing               | `TABLE_SIZE` is a compile-time constant. Probing stops accepting people at 10, and chains only get longer.                            |
| Tombstones are never cleaned up       | After many deletes, lookups in `main.c` walk past tombstones and degrade towards scanning the whole table.                           |
| Duplicate names are not detected      | Inserting the same name twice stores both, and lookup returns only one of them.                                                       |
| The table does not own its people     | It stores pointers to `Person` variables in `main`. If those went out of scope, the table would point at dead memory.                 |
| Name length limits disagree           | `Person.name` holds 100 chars, but `hash` and `strncmp` read up to `MAX_NAME_LENGTH` (256).                                           |
| Parameters are `char *`, not `const`  | The functions never modify the names they receive, so `const char *` would document that and let the compiler enforce it.            |

---

## Background

- C reference (standard library): https://en.cppreference.com/w/c
- Hash table (collision resolution): https://en.wikipedia.org/wiki/Hash_table#Collision_resolution
- CMake tutorial: https://cmake.org/cmake/help/latest/guide/tutorial/index.html
