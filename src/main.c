/**
 * User-friendly CLI that demonstrates all features of the key-value store
 */

#include "kvstore.h"
#include "persistence.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define  MAX_LINE_LENGTH 1024
#define  MAX_VALUE_LENGTH 512
#define  DEFAULT_FILENAME "kvstore_data.bin"

/**
 * Print the help message showing available commands
 */
static void print_help(void) {
    printf("\nAvailable commands:\n");
    printf("  set <key> <value>  - Set a key-value pair\n");
    printf("  get <key>          - Get value for a key\n");
    printf("  delete <key>       - Delete a key-value pair\n");
    printf("  list               - List all key-value pairs\n");
    printf("  stats              - Show store statistics\n");
    printf("  save [filename]    - Save store to file (default: %s)\n", DEFAULT_FILENAME);
    printf("  load [filename]    - Load store from file (default: %s)\n", DEFAULT_FILENAME);
    printf("  clear              - Clear all entries\n");
    printf("  help               - Show this help message\n");
    printf("  quit               - Exit the program\n");
    printf("\n");
}

/** 
 * Trim whitespaces from the beg and end of a string
 */
static char* trim_whitespaces(char* str) {
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0'){
        return str;
    }

    // Trim trailing whitespaces
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // null-terminate after last non-whitespace character
    end[1] = '\0';

    return str;
}

/**
 * Parse an integer from a string without error checking 
 */
static bool parse_int (const char* str, int* result) {
    if (!str || !result) {
        return false;
    }

    char* endptr;
    long val = strtol(str, &endptr, 10);

    // check for parsing errors
    if (endptr == str || *endptr != '\0') {
        return false;
    }

    // integer overflow
    if (val > INT32_MAX || val < INT32_MIN) {
        return false;
    }

    *result = (int) val;
    return true;
}

/**
 * Handle the 'set' command
 */
static void handle_set_command(kvstore_t* kvs, char* args) {
    // parse key
    char* key_str = strtok(args, " \t");
    if (!key_str) {
        printf("Error: Missing key. Usage: set <key> <value>\n");
        return;
    }

    int key;
    if (!parse_int(key_str, &key)) {
        printf("Error: Invalid key. Key must be an integer.\n");
        return;
    }

    // Parse value (rest of the line)
    char* value = strtok(NULL, "");
    if (!value) {
        printf("Error: missing value. Usage: set <key> <value>\n");
        return;
    }

    value = trim_whitespaces(value);
    if (strlen(value) == 0) {
        printf("Error: Value cannot be empty.\n");
        return;
    }

    if (strlen(value) > MAX_VALUE_LENGTH) {
        printf("Error: Value too long (max %d characters).\n", MAX_VALUE_LENGTH);
        return;
    }

    // Set the key-value pair
    if (kvs_set(kvs, key, value)) {
        printf("Set: %d = \"%s\"\n", key, value);
    } else {
        printf("Error: Failed to set key-value pair: %s\n", kvs_error_string(kvs_get_error()));
    }
}

/**
 * Handle the 'get' command
 */
static void handle_get_command(kvstore_t* kvs, char* args) {
    // parse key
    char* key_str = trim_whitespaces(args);
    if (strlen(key_str) == 0){
        printf("Error: Missing key. Usage: get <key>\n");
        return;
    }

    int key;
    if (!parse_int(key_str, &key)) {
        printf("Error: Invalid key. Key must be an integer.\n");
        return;
    }

    // Get the value
    const char* value = kvs_get(kvs, key);
    if (value) {
        printf("Get: %d = \"%s\"\n", key, value); 
    } else {
        printf("Key %d not found.\n");
    }
}

/**
 * Handle the 'delete' command
 */
static void handle_delete_command(kvstore_t* kvs, char* args) {
    // parse key
    char* key_str = trim_whitespaces(args);
    if (strlen(key_str) == 0) {
        printf("Error: Missing key. Usage: delete <key>\n");
        return;
    }

    int key;
    if (!parse_int(key_str, &key)) {
        printf("Error: Invalid key. Key must be an integer.\n");
        return;
    }

    // Delete the key
    if (kvs_delete(kvs, key)) {
        printf("Deleted key: %d\n", key);
    } else {
        printf("Key %d not found.\n", key);
    }
}

/**
 * Handle the save
 */
static void handle_save_command(kvstore_t* kvs, char *args) {
    // parse filname
    char* filename = trim_whitespaces(args);
    if (strlen(filename) == 0) {
        filename = DEFAULT_FILENAME;
    }

    // save to file
    if (kvs_save(kvs, filename)) {
        printf("Saved %zu entries to '%s'\n", kvs_count(kvs), filename);
    } else {
        printf("Errro: Failed to save to file: %s\n", kvs_error_string(kvs_get_error()));
    }
}

/**
 * Handle the 'load' command
 */
static void handle_load_command(kvstore_t* kvs, char* args) {
    // parse filename
    char* filename = trim_whitespaces(args);
    if (strlen(filename) == 0) {
        filename = DEFAULT_FILENAME;
    }

    // check if file exists 
    if (!kvs_file_exists(filename)) {
        printf("Error: File '%s' does not exist,\n");
        return;
    }

    // Load from file
    if (kvs_load(kvs, filename)) {
        printf("Loaded %zu entries from '%s'\n", kvs_count(kvs), filename);
    } else {
        printf("Error: Failed to load from file: %s\n", 
               kvs_error_string(kvs_get_error()));
    }
}

/**
 * Handle the 'clear' command
 */
static void handle_clear_command(kvstore_t* kvs) {
    size_t count = kvs_count(kvs);
    
    // create a new empty store to replace the current one
    hash_table_t* old_table = kvs->table;
    kvs->table = ht_create(16);

    if (kvs->table) {
        ht_destroy(old_table);
        printf("Cleared %zu entries\n", count);
    } else {
        kvs->table = old_table;
        printf("Error: Failed to clear store: %s\n", 
               kvs_error_string(kvs_get_error()));
    }
}

/**
 * Process a single command line
 */
static bool process_command(kvstore_t* kvs, char* line) {
    // trim whitespaces and check for emtpy line
    line = trim_whitespaces(line);
    if (strlen(line) == 0) {
        return true;
    }

    // parse command
    char* command = strtok(line, " \t");
    if (!command) {
        return true;
    }

    // Get argument (rest of the line)
    char* args = strtok(NULL, "");
    if (!args) {
        args = "";
    }

    // process commands
    if (strcmp(command, "set") == 0) {
        handle_set_command(kvs, args);
    } else if (strcmp(command, "get") == 0) {
        handle_get_command(kvs, args);
    } else if (strcmp(command, "delete") == 0 || strcmp(command, "del") == 0) {
        handle_delete_command(kvs, args);
    } else if (strcmp(command, "list") == 0 || strcmp(command, "ls") == 0) {
        kvs_print_all(kvs);
    } else if (strcmp(command, "stats") == 0) {
        kvs_print_stats(kvs);
    } else if (strcmp(command, "save") == 0) {
        handle_save_command(kvs, args);
    } else if (strcmp(command, "load") == 0) {
        handle_load_command(kvs, args);
    } else if (strcmp(command, "clear") == 0) {
        handle_clear_command(kvs);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
        print_help();
    } else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        return false;
    } else {
        printf("Unknown command: %s (type 'help' for available commands)\n", command);
    }

    return true;

}

/**
 * Entry point of the program 
 * Sets up the key-value store and runs the interactive loop
 */

 int main(int argc, char*argv[]) {
    printf("Key-value Store Interactive Shell\n");
    printf("Type 'help' for available commands, 'quit' or 'exit'.\n\n");

    // create the key-value store
    kvstore_t* kvs = kvs_create(0); 
    if (!kvs) {
        printf("Error: Failed to create key-value store: %s\n", kvs_error_string(kvs_get_error()));
        return 1;
    }

    // Try to load data from default file if it exists
    if (kvs_file_exists(DEFAULT_FILENAME)){
        if (kvs_load(kvs,DEFAULT_FILENAME)) {
            printf("Loaded %zu entries from '%s'\n",
            kvs_count(kvs), DEFAULT_FILENAME);
        } else {
            printf("Warning: Could not load '%s'\n",
            DEFAULT_FILENAME, kvs_error_string(kvs_get_error()));
        }
        printf("\n");
    }

    // main interactive loop
    char line[MAX_LINE_LENGTH];
    while (true) {
        printf("kvs> ");
        fflush(stdout); 

        // Read input line
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        // remove newline character
        line[strcspn(line, "\n")] = '\0';

        // process the command 
        if (!process_command(kvs, line)) {
            break;
        }
    }

    // Auto-save on exit if there's data
    if (kvs_count(kvs) > 0) {
        printf("Auto-saving data to '%s'....\n", DEFAULT_FILENAME);
        if (!kvs_save(kvs, DEFAULT_FILENAME)) {
            printf("Warning: Could not save data: %s\n",
            kvs_error_string(kvs_get_error()));
        }
    }

    //clean up
    kvs_destroy(kvs);
    printf("Goodbye!\n");

    return 0;
 }