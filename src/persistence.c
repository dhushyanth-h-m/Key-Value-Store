/**
 * File persistence implementation
 * 
 * Implements saving and loading hash table data to/from files
 * It creates a custom binary format with a header containing metadata 
 * followed by key-value pairs
 */

 #include "persistence.h"
 #include "error.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>

 /**
  * Save the hash table contents to a file
  * File format consists of:
  * 1. File header (magic number, version, entry count)
  * 2. Series of entries (key length, key, value length, value)
  */
bool kvs_save_to_file (hash_table_t* table, const char* filename) {
    // validate params
    if (!table || !filename) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // open file for binary writing
    FILE* file = fopen(filename, "wb");
    if (!file) {
        kvs_set_error(KVS_ERROR_FILE_IO);
        return false;
    }

    // Prepare and write file header
    kvs_file_header_t header;
    header.magic = KVS_MAGIC_NUMBER;
    header.version = KVS_FILE_VERSION;
    header.entry_count = (uint32_t)ht_size(table);
    header.reserved = 0;

    if (fwrite(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        kvs_set_error(KVS_ERROR_FILE_IO);
        return false;
    }

    // Write each key-value pair
    ht_iterator_t iter = ht_iterator_init(table);
    int key;
    const char* value;

    while (ht_iterator_next(&iter, &key, &value)) {
        // Write key
        if (fwrite(&key, sizeof(key), 1, file) != 1) {
            fclose(file);
            kvs_set_error(KVS_ERROR_FILE_IO);
            return false;
        }

        // write value length
        uint32_t value_len = (uint32_t)strlen(value);
        if (fwrite(&value_len, sizeof(value_len), 1, file) != 1) {
            fclose(file);
            kvs_set_error(KVS_ERROR_FILE_IO);
            return false;
        }

        // Write value string
        if (fwrite(value, 1, value_len, file) != value_len) {
            fclose(file);
            kvs_set_error(KVS_ERROR_FILE_IO);
            return false;
        }
    }

    fclose(file);
    kvs_clear_error();
    return true;
}


/**
 * Load hash table from a file
 * reads the file format and populates the hash table
 */
bool kvs_load_from_file(hash_table_t* table, const char* filename) {
    // validate params
    if (!table || !filename) {
        kvs_set_error(KVS_ERROR_INVALID_PARAM);
        return false;
    }

    // open file for binary reading
    FILE* file = fopen(filename, "rb");
    if (!file) {
        kvs_set_error(KVS_ERROR_FILE_IO);
        return false;
    }

    // read and validate file header
    kvs_file_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        kvs_set_error(KVS_ERROR_FILE_IO);
        return false;
    }

    // validate magic number
    if (header.magic != KVS_MAGIC_NUMBER) {
        fclose(file);
        kvs_set_error(KVS_ERROR_CORRUPTION);
        return false;
    }

    // check version compatibility
    if (header.version != KVS_FILE_VERSION) {
        fclose(file);
        kvs_set_error(KVS_ERROR_CORRUPTION);
        return false;
    }

    // Read each key-value pair
    for (uint32_t i = 0; i < header.entry_count; i++) {
        int key;
        if (fread(&key, sizeof(key), 1, file) != 1) {
            fclose(file);
            kvs_set_error(KVS_ERROR_FILE_IO);
            return false;
        }

        // read value length
        uint32_t value_len;
        if (fread(&value_len, sizeof(value_len), 1, file) != 1) {
            fclose(file);
            kvs_set_error(KVS_ERROR_FILE_IO);
            return false;
        }

        //validate value length 
        if (value_len > 100000) {
            fclose(file);
            kvs_set_error(KVS_ERROR_CORRUPTION);
            return false;
        }

        char* value = malloc(value_len + 1);
        if (!value) {
            fclose(file);
            kvs_set_error(KVS_ERROR_MEMORY);
            return false;
        }

        // Read value string
        if (fread(value, 1, value_len, file) != value_len) {
            free(value);
            fclose(file);
            kvs_set_error(KVS_ERROR_FILE_IO);
            return false;
        }

        // null-terminate the string
        value[value_len] = '\0';

        // insert into hash table
        if (!ht_set(table, key, value)) {
            free(value);
            fclose(file);
            return false;
        }

        free(value);
    }

    fclose(file);
    kvs_clear_error();
    return true;
}


/**
 * check if a file exists and is readable
 */
bool kvs_file_exists(const char* filename) {
    if (!filename) {
        return false;
    }

    FILE* file = fopen(filename, "rb");
    if (file) {
        fclose(file);
        return true;
    }

    return false;
}