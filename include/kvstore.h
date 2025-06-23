/**
 * main kv store API
 * 
 * 
 * provides high-level interface for the key-value store
 * It wraps the hash table operations and adds additional functionality
 * like persistence and statistics
 * 
 */

#ifndef KVSTORE_H
#define KVSTORE_H

#include "hash_table.h"
#include "error.h"
#include <stdbool.h>

/**
 * Key-value store structure
 */
typedef struct {
    hash_table_t* table;        
    char* filename;
} kvstore_t;

/**
 * create a new key-value store
 */
kvstore_t* kvs_create(size_t initial_capacity);

/**
 * set a key-value pair in the store
 */
bool kvs_set(kvstore_t* kvs, int key, const char* value);

/**
 * Get a value by key
 */
const char* kvs_get(kvstore_t* kvs, int key);

/**
 * get the number of key-value pairs in the store
 */
size_t kvs_count(kvstore_t* kvs);

/**
 * save the store contents to a file
 */
bool kvs_save(kvstore_t* kvs, const char* filename);

/**
 * Load contents from a file
 */
bool kvs_load(kvstore_t* kvs, const char* filename);

/**
 * print stats
 */
void kvs_print_stats(kvstore_t* kvs);

/**
 * Destroy a kv store and free all memory 
 */
void kvs_destroy(kvstore_t* kvs);

/**
 * print all
 */
void kvs_print_all(kvstore_t* kvs);

#endif