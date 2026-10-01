#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 256
#define TABLE_SIZE 10

typedef struct Person {
  int id;
  char name[100];
  int age;
  struct Person *next;
} Person;

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
    } else {
      printf("\t%i\t", i);
      Person *tmp = hash_table[i];
      while (tmp != NULL) {
        printf("%s -> ", tmp->name);
        tmp = tmp->next;
      }
      printf("NULL\n");
    }
  }
}

bool insert_into_hash_table(Person *person) {
  if (person == NULL) return false;
  int index = hash(person->name);
  person->next = hash_table[index];
  hash_table[index] = person;
  printf("Inserted %s at index %i\n", person->name, index);
  return true;
}

Person *hash_table_lookup(char *name) {
  int index = hash(name);
  Person *tmp = hash_table[index];
  while (tmp != NULL && strncmp(tmp->name, name, MAX_NAME_LENGTH) != 0) {
    tmp = tmp->next;
  }
  return tmp;
}

Person *hash_table_delete(char *name) {
  int index = hash(name);
  Person *tmp = hash_table[index];
  Person *prev = NULL;
  while (tmp != NULL && strncmp(tmp->name, name, MAX_NAME_LENGTH) != 0) {
    prev = tmp;
    tmp = tmp->next;
  }
  if (tmp == NULL) return NULL;
  if (prev == NULL) {
    // deleting the head of the list
    hash_table[index] = tmp->next;
  } else {
    // deleting from the middle or the end
    prev->next = tmp->next;
  }
  tmp->next = NULL;
  return tmp;
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
