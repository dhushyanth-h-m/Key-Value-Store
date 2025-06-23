/**
 * Hashtable impl header
 * 
 * defines the hash table data structure and operations
 * HT is the core DS that provides fast O(1) average-case 
 * lookups, insertions, and deletions
 */


#ifndef HASH_TABLE_H
#define HASH_TABLE_H    

#include <stddef.h>
#include <stdbool.h>

/**
 * Special key value used to mark deleted entries (tombstones)
 * this allows us to maintain the probe sequence during deletion
 */
#define DELETED_KEY (-2147483648)


/**
 * Hash table entry structure
 * each entry stores a key-value pair and occupation status
 */
typedef struct {
    int key;        // The integer key
    char* value;    // The string value (dynamically allocated)
    bool occupied;   // whether this slot is occupied
} ht_entry_t;

/**
 * Hash table structure
 * Contains the array of entries and metadata about the table
 */
typedef struct {
    ht_entry_t* entries;       // Array of hash table entries
    size_t capacity;           // total number of slots in the table
    size_t size;               // number of occupied slots (excluding tombstones)
    size_t tombstones;         // number of deleted slots (tombstones)
} hash_table_t;

/**
 * Create a new hash table
 * @param initial_capacity Initial number of slots (must be > 0)
 * @return Pointer to the new hash_table or NULL on failure
 */
hash_table_t* ht_create(size_t initial_capacity);

/**
 * Insert or update a key-value pair
 * @param table Pointer to the hash table
 * @param key The integer key
 * @param value The string value (will be copied)
 * @return true on success, false on failure
 */
bool ht_set(hash_table_t* table, int key, const char* value);

/**
 * Retrieve a value by a key
 * @param table Pointer to the hash table
 * @param key The integer key
 * @return pointer to the value string, or NULL if not found
 */
const char* ht_get(hash_table_t* table, int key);

/**
 * Delete a key-value pair
 * @param table pointer to the hash table
 * @param key The integer key
 * @return bool
 */
bool ht_delete(hash_table_t* table, int key);

/**
 * Get the number of key-value pairs
 * @param table pointer to the hash table
 * @return number of entries in the table
 */
size_t ht_size(hash_table_t* table);

/**
 * Get the table capacity
 * @param table pointer to the hash table
 * @return total capacity of the table
 */
size_t ht_capacity(hash_table_t* table);

/**
 * Destroy the hash table and free all memory 
 * @param table pointer to the hash table to destroy 
 */
void ht_destroy(hash_table_t* table);

/**
 * Iterator structure for traversing the hash table
 */
typedef struct {
    hash_table_t* table;        // pointer to the table being traversed
    size_t index;               // current position in the entries array
} ht_iterator_t;

/**
 * Init an iterator for the hash table
 */
ht_iterator_t ht_iterator_init(hash_table_t* table);

/**
 * Get the next key-value pair from the iterator
 * @param iter pointer to the iterator
 * @param key pointer to store the key
 * @param value pointer to store the value
 * @return bool
 */
bool ht_iterator_next(ht_iterator_t* iter, int* key, const char** value);


#endif