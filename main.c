#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_NAME_LENGTH 256
#define TABLE_SIZE 10

typedef struct {
  int id;
  char name[100];
  int age;
} Person;

#define DELETED_NODE ((Person*)UINTPTR_MAX)

Person *hash_table[TABLE_SIZE];

unsigned int hash(char *name) {
  int length = strnlen(name, MAX_NAME_LENGTH);
  unsigned int hash_value = 0;
  for (int i = 0; i < length; i++) {
    hash_value += name[i];
    hash_value = hash_value * name[i] % TABLE_SIZE;
  }

  return hash_value;
}

bool init_hash_table() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    hash_table[i] = NULL;
  }
  return true;
}

void print_table() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    if (hash_table[i] == NULL) {
      printf("\t%i\t---\n", i);
    } else if (hash_table[i] == DELETED_NODE) {
      printf("\t%i\t<deleted>\n", i);
    } else {
      printf("\t%i\t%s\n", i, hash_table[i]->name);
    }
  }
}

bool insert_into_hash_table(Person *person) {
  if (person == NULL) return false;
  int index = hash(person->name);
  for (int i = 0; i < TABLE_SIZE; i++) {
    int try = (i + index) % TABLE_SIZE;
    if (hash_table[try] == NULL || hash_table[try] == DELETED_NODE) {
      hash_table[try] = person;
      printf("Inserted %s at index %i\n", person->name, try);
      return true;
    }
  }
  printf(" !!! Table is full %s !!!\n", person->name);
  printf("--------------------------------\n");
  return false;
}

Person *hash_table_delete(char *name) {
  int index = hash(name);
  for (int i = 0; i < TABLE_SIZE; i++) {
    int try = (i + index) % TABLE_SIZE;
    if (hash_table[try] == NULL) return NULL;
    if (hash_table[try] == DELETED_NODE) continue;
    if (strncmp(hash_table[try]->name, name, MAX_NAME_LENGTH) == 0) {
      Person *tmp = hash_table[try];
      hash_table[try] = DELETED_NODE;
      return tmp;
    }
  }
  return NULL;
}

Person *hash_table_lookup(char *name) {
  int index = hash(name);
  for (int i = 0; i < TABLE_SIZE; i++) {
    int try = (i + index) % TABLE_SIZE;
    if (hash_table[try] == NULL) return false;
    if (hash_table[try] == DELETED_NODE) continue;
    if (strncmp(hash_table[try]->name, name, MAX_NAME_LENGTH) == 0) {
      return hash_table[try];
    }
  }
  return NULL;
}

void print_lookup_result(Person *person) {
  if (person == NULL) {
    printf("Not found\n");
  } else {
    printf("Found %s\n", person->name);
  }
  printf("--------------------------------\n");
}


int main() {
  init_hash_table();
  Person person1 = {.id = 1, .name = "John", .age = 20};
  Person person2 = {.id = 2, .name = "Jane", .age = 21};
  Person person3 = {.id = 3, .name = "Jim", .age = 91};
  Person person4 = {.id = 4, .name = "Jill", .age = 73};
  Person person5 = {.id = 5, .name = "Jack", .age = 24};
  Person person6 = {.id = 6, .name = "Test", .age = 68};
  Person person7 = {.id = 7, .name = "Daniel", .age = 27};
  Person person8 = {.id = 8, .name = "Sarah", .age = 13};
  Person person9 = {.id = 9, .name = "Adam", .age = 47};
  Person person10 = {.id = 10, .name = "Olivia", .age = 50};
  Person person11 = {.id = 11, .name = "Tega", .age = 11};
  insert_into_hash_table(&person1);
  insert_into_hash_table(&person2);
  insert_into_hash_table(&person3);
  insert_into_hash_table(&person4);
  insert_into_hash_table(&person5);
  insert_into_hash_table(&person6);
  insert_into_hash_table(&person7);
  insert_into_hash_table(&person8);
  insert_into_hash_table(&person9);
  insert_into_hash_table(&person10);
  insert_into_hash_table(&person11);
  printf("--------------------------------\n");
  print_table();
  printf("--------------------------------\n");

  Person *tmp = hash_table_lookup("John");
  print_lookup_result(tmp);

  tmp = hash_table_lookup("Nobody");
  print_lookup_result(tmp);

  tmp = hash_table_delete("Jane");
  print_lookup_result(tmp);
  Person *tmp2 = hash_table_lookup("Jane");
  print_lookup_result(tmp2);

  print_table();
  return 0;
}