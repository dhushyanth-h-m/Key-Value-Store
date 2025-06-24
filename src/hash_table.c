/**
 * Implements a hash table using open addressng with linear probing
 * Dynamic resizing, tombstone deletion, and FNV-1a hash function 
 * prioritizes simplicity while maintaining good performance
 */

#include "hash_table.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// default init capacity for new HT (power of 2 for optimal performane)
#define DEFAULT_CAPACITY 16

// Load factor threshold for trigerring resize 
// (size + tombstones) / capacity exceeds -> resize
#define LOAD_FACTOR_THRESHOLD 0.75

//Growth factor for resizing
#define GROWTH_FACTOR 2

/**
 * FNV-1a hash Impl
 * Simple, fast hash func with good distribution properties
 * for integer keys. Non-cryptographic but suitable for hash tables
 */
static size_t hash_key(int key) {
    //FNV-1a constants
    uint32_t hash = 2166136261u; 
    const uint8_t* data = (const uint8_t*)&key;

    //process each byte of the key
    for (size_t i = 0; i < sizeof(int); i++) {
        hash ^= data[i];    // xor with current byte
        hash *= 16777619u;  // multiply by FNV prime
    }

    return (size_t)hash;
}

// Find a slot for a key -> implement linear probing to handle collisions
static size_t find_slot(hash_table_t* table, int key, bool for_insertion) {
    if (table->capacity == 0) {
        return SIZE_MAX;
    }

    size_t index = hash_key(key) % table->capacity;
    size_t original_index = index;
    size_t first_tombstone = SIZE_MAX; 

    do {
        ht_entry_t* entry = &table->entries[index];

        if (!entry->occupied) {
            // Empty slot
            if (for_insertion && first_tombstone != SIZE_MAX) {
                // return the first tombstone we found for insertion
                return first_tombstone;
            }
            return index;
        }

        if (entry->key == DELETED_KEY) {
            // Tombstone found
            if (for_insertion && first_tombstone == SIZE_MAX) {
                first_tombstone = index; 
            }
            //continue probing for lookups in case key exists later
        } else if (entry->key == key) {
            // Key found
            return index;
        }

        index = (index + 1) % table->capacity;
    } while (index != original_index);

    if (for_insertion && first_tombstone != SIZE_MAX) {
        return first_tombstone;
    }

    return SIZE_MAX;
}

/**
 * Resize the hash table to a new capacity 
 * creates a new table and re-inserts all existing entries
 */
static bool resize_table (hash_table_t* table, size_t new_capacity) {
    //validate new capacity
    if (new_capacity <= table->capacity) {
        return false;
    }

    //save old entries array
    ht_entry_t* old_entries = table->entries;
    size_t old_capacity = table->capacity;

    //Allocate new entries array
    table->entries = calloc(new_capacity, sizeof(ht_entry_t));
    if (!table->entries) {
        //restore old entries on failure
        table->entries = old_entries;
        kvs_set_error(KVS_ERROR_MEMORY);
        return false;
    }

    //udpate table metadata
    table->capacity = new_capacity;
    table->size = 0;
    table->tombstones = 0;

    // Re-insert all non-deleted entries
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].occupied && old_entries[i].key != DELETED_KEY) {
            // Re-insert the entry (this will update the table->size)
            if (!ht_set(table, old_entries[i].key, old_entries[i].value)){
                // Insertion failed
                free(table->entries);
                table->entries = old_entries;
                table->capacity = old_capacity;

                return false;
            }
        }

        // Free the old value string
        if (old_entries[i].value) {
            free(old_entries[i].value);
        }
    }

    free(old_entries);
    return true;
}

/**
 * Create a new hash table
 * allocates memory and initializes the hash table structure
 */
hash_table_t* ht_create(size_t initial_capacity) {
    // Use default capacity if none specified
    if (initial_capacity == 0) {
        initial_capacity = DEFAULT_CAPACITY;
    }

    // Allocate the hash table structure
    hash_table_t* table = malloc(sizeof(hash_table_t));
    if (!table) {
        kvs_set_error(KVS_ERROR_MEMORY);
        return NULL;
    }

    // Allocate the entries array (initialized to zero)
    table->entries = calloc(initial_capacity, sizeof(ht_entry_t));
    if (!table->entries) {
        free(table);
        kvs_set_error(KVS_ERROR_MEMORY);
        return NULL;
    }

    // Initialize metadata
    table->capacity = initial_capacity;
    table->size = 0;
    table->tombstones = 0;

    kvs_clear_error();
    return table;
}


/**
 * Insert or update a key-value pair
 * if the key exists, its value is updated, otherwise a new entry is created
 */
bool ht_set(hash_table_t* table, int key, const char* value) {
    // validate params
    if (!table || !value || key == DELETED_KEY) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // check if resize is needed before insertion
    double load_factor = (double)(table->size + table->tombstones) / table->capacity;
    if (load_factor >= LOAD_FACTOR_THRESHOLD) {
        if (!resize_table(table, table->capacity * GROWTH_FACTOR)){
            return false; 
        }
    }

    // find slot for the key
    size_t index = find_slot(table, key, true); 
    if (index == SIZE_MAX) {
        kvs_set_error(KVS_ERROR_MEMORY);
        return false;
    }

    ht_entry_t* entry = &table->entries[index];

    // copy the value string
    char* value_copy = malloc(strlen(value) + 1);
    if (!value_copy) {
        kvs_set_error(KVS_ERROR_MEMORY);
        return false;
    }
    strcpy(value_copy, value);

    if (entry->occupied && entry->key == key && entry->key != DELETED_KEY) {
        // reusing tombstones
        table->tombstones--;
    }

    entry->key = key;
    entry->value = value_copy;
    entry->occupied = true;
    table->size++;
}

/**
 * Retrieve a value by key 
 * performs a lookup in the hashtable using linear probing
 */
const char* ht_get(hash_table_t* table, int key) {
    // validate param
    if (!table || key == DELETED_KEY) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return NULL;
    }

    // Find the key 
    size_t index = find_slot(table, key, false);
    if (index == SIZE_MAX) {
        kvs_set_error(KVS_ERROR_KEY_NOT_FOUND);
        return NULL;
    }

    ht_entry_t* entry = &table->entries[index];
    if (!entry->occupied || entry->key != key || entry->key == DELETED_KEY) {
        kvs_set_error(KVS_ERROR_KEY_NOT_FOUND);
        return NULL;
    }

    kvs_clear_error();
    return entry->value;
}

/**
 * Delete a key-value pair
 * uses tombstone deletion to maintain probe sequence
 */
bool ht_delete(hash_table_t* table, int key) {
    // validate params
    if (!table || key == DELETED_KEY) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // Find the key
    size_t index = find_slot(table, key, false);
    if (index == SIZE_MAX) {
        kvs_set_error(KVS_ERROR_KEY_NOT_FOUND);
        return false;
    }

    ht_entry_t* entry = &table->entries[index];
    if (!entry->occupied || entry->key != key || entry->key == DELETED_KEY) {
        kvs_set_error(KVS_ERROR_KEY_NOT_FOUND);
        return false;
    }

    // Mark as deleted 
    free(entry->value);
    entry->value = NULL;
    entry->key = DELETED_KEY;
    // leave occupied to be true to maintain probe sequence

    table->size--;
    table->tombstones++;

    kvs_clear_error();
    return true;
}

/**
 * Get the number of key-value stores
 * Count of active entries
 */
size_t ht_size(hash_table_t* table) {
    if (!table) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return 0;
    }
    return table->size;
}

/**
 * Get the table capacity 
 */
size_t ht_capacity(hash_table_t* table) {
    if (!table) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return 0;
    }
    return table->capacity;
}

/**
 * Initialize an iterator for the hash table
 * sets up an iterator to traverse all key-value pairs
 */
ht_iterator_t ht_iterator_init(hash_table_t* table) {
    ht_iterator_t iter;
    iter.table = table;
    iter.index = 0;
    return iter;
}

/**
 * Get the next key-value pair from the iterator
 * Advances the iterator and returns the next valid entry
 */
bool ht_iterator_next(ht_iterator_t* iter, int* key, const char** value) {
    //validate params
    if (!iter || !iter->table || !key || !value) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // Find the next occupied non-deleted entry
    while (iter->index < iter->table->capacity) {
        ht_entry_t* entry = &iter->table->entries[iter->index];
        iter->index++; // move to the next slot for next call

        if (entry->occupied && entry->key != DELETED_KEY) {
            *key = entry->key;
            *value = entry->value;
            return true;
        }
    }

    return false;
}

/**
 * Destroy the hash_table and free all memory 
 */
void ht_destroy(hash_table_t* table) {
    if (!table) {
        return;
    }

    //Free all value strings
    if (table->entries) {
        for (size_t i = 0; i < table->capacity; i++){
            if (table->entries[i].value) {
                free(table->entries[i].value);
            }
        }
        free(table->entries);
    }

    free(table);
}

