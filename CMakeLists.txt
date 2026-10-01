cmake_minimum_required(VERSION 3.20)
project(c_hashtable C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

# Collisions handled with linear probing
add_executable(hashtable main.c)

# Collisions handled with linked lists (separate chaining)
add_executable(hashtable_linked_list main_linked_list.c)
