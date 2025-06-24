/**
 * Main key-value store implementation
 * 
 * Implements the high-level API that wraps the hash table
 * and provides additional functionality like persistence and statistics
 */


#include "kvstore.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Default initial capacity for new stores
#define DEFAULT_INITIAL_CAPACITY 16

/**
 * Create a new key-value store
 * Allocates and init a new kvstore structure
 */
kvstore_t* kvs_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = DEFAULT_INITIAL_CAPACITY;
    }

    // Allocate the KVstore structure
    kvstore_t* kvs = malloc(sizeof(kvstore_t));
    if (!kvs) {
        kvs_set_error(KVS_ERROR_MEMORY);
        return NULL;
    }

    // create underlying hash table
    kvs->table = ht_create(initial_capacity);
    if (!kvs->table) {
        free(kvs);
        return NULL;
    }

    kvs->filename = NULL;

    kvs_clear_error();
    return kvs;
}


/**
 * Set a key-value pair in the store
 * wrapper around the hash table set operation
 */
bool kvs_set(kvstore_t* kvs, int key, const char* value) {
    // validate params
    if (!kvs || !kvs->table) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    return ht_set(kvs->table, key, value);
}


/**
 * Get a value by key
 * wrapper aroudn the hash table get operationm
 */
const char* kvs_get(kvstore_t* kvs, int key) {
    // validate params
    if (!kvs || !kvs->table) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return NULL;
    }

    return ht_get(kvs->table, key);
}

/**
 * delete a key-value pair
 */
bool kvs_delete(kvstore_t* kvs, int key) {
    // validate params
    if (!kvs || !kvs->table) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    return ht_delete(kvs->table, key);
}

/**
 * Get the number of key-value pairs
 */
size_t kvs_count(kvstore_t* kvs) {
        // validate params
    if (!kvs || !kvs->table) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return 0;
    }

    return ht_size(kvs->table);
}

/**
 * Save the store contents to a file
 */
bool kvs_save(kvstore_t* kvs, const char* filename) {
        // validate params
    if (!kvs || !kvs->table || !filename) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // save to file
    if (!kvs_save_to_file(kvs->table, filename)) {
        return false;
    }

    // update stored filename
    free(kvs->filename);
    kvs->filename = malloc(strlen(filename) + 1);
    if (kvs->filename) {
        strcpy(kvs->filename, filename);
    }

    return true;
}

/** 
 * Load store contents from a file
 */
bool kvs_load(kvstore_t* kvs, const char* filename) {
        // validate params
    if (!kvs || !kvs->table || !filename) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // load from file
    if (!kvs_load_from_file(kvs->table, filename)) {
        return false;
    }

    // update stored filename 
    free(kvs->filename);
    kvs->filename = malloc(strlen(filename) + 1);
    if (kvs->filename) {
        strcpy(kvs->filename, filename);
    }

    return true;
}



void kvs_print_stats(kvstore_t* kvs) {
    if (!kvs || !kvs->table) {
        printf("Invalid key-value store");
        return;
    }

    size_t size = ht_size(kvs->table);
    size_t capacity = ht_capacity(kvs->table);
    double load_factor = capacity > 0 ? (double)size / capacity : 0.0;

    printf("Key-Value Store Statistics:\n");
    printf("  Entries: %zu\n", size);
    printf("  Capacity: %zu\n", capacity);
    printf("  Load Factor: %.2f%%\n", load_factor * 100);
    
    if (kvs->filename) {
        printf(" Associated file: %s\n", kvs->filename);
    } else {
        printf(" Associated file: None\n");
    }
}   

/**
 * Print all key-value pairs 
 */
void kvs_print_all(kvstore_t* kvs) {
    if (!kvs || !kvs->table) {
        printf("Invalid key-value store");
        return;
    }

    size_t count = ht_size(kvs->table);
    if (count == 0) {
        printf("Key-value store is empty");
        return;
    }

    printf("key value store contents (%zu entries):\n", count);

    // use iterator to traverse all entries
    ht_iterator_t iter = ht_iterator_init(kvs->table);
    int key;
    const char* value;

    while (ht_iterator_next(&iter, &key, &value)) {
        printf("  %d: \"%s\"\n", key, value);
    }
}


/**
 * Destroy the key-value store and free all memory
 */
void kvs_destroy(kvstore_t* kvs) {
    if (!kvs) {
        return;
    }

    // Destroy the hash table
    if (kvs->table) {
        ht_destroy(kvs->table);
    }

    // free the filename string
    free(kvs->filename);

    // free kvstore structure
    free(kvs);
}