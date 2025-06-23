/**
 * file persistence operations
 * 
 * functions for saving and loading the hash table
 * to and from disk. Implements a custom binary format 
 * for efficient storage and retrieval
 */


#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "hash_table.h"
#include <stdbool.h>
#include <cstdint>

/**
 * Magic number for file format identification
 * Helps verify that we're reading a valid kvstore file
 */
#define KVS_MAGIC_NUMBER 0x4B565301 // "KVS" + version 1

/**
 * current file format version
 * allows for future format upgrades 
 */
#define KVS_FILE_VERSION 1

/**
 * File header structure
 * Contains metadata about the saved data
 */
typedef struct {
    uint32_t magic;         // Magic number for format identification
    uint32_t version;       // file format version
    uint32_t entry_count;   // Number of key-value pairs in the file
    uint32_t reserved;      //Reserved for future use
} kvs_file_header_t;

/**
 * Save the hash table contents to a file
 */
bool kvs_save_to_file(hash_table_t* table, const char* filename);

/**
 * load hash table contents from a file
 * 
 */
bool kvs_load_from_file(hash_table_t* table,  const char* filename);

/**
 * check if a file exists and is redable
 */
bool kvs_file_exists(const char* filename);

#endif