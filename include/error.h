/**
 * error.h  - Error handling sys for the Key-Value store
 * 
 * header - defines error codes and functions for managing errors 
 * thorughout the application. 
 */


 #ifndef ERROR_H
 #define ERROR_H

/**
 * Error codes for different types of failures
 * Each error represents a specific failure condition
 */
typedef enum {
    KVS_SUCCESS = 0,            // Operation completed successfully
    KVS_ERROR_MEMORY,           // Memory allocation failed
    KVS_ERROR_KEY_NOT_FOUND,    // Requested key doesn't exist
    KVS_ERROR_INVALID_PARAM,    // Invalid parameter passed to function 
    KVS_ERROR_FILE_IO,          // File input / output operation failed
    KVS_ERROR_CORRUPTION,       // DATA corruption detected
    KVS_ERROR_UNKNOWN           // Uknown or unexpected error
} kvs_error_t;


/**
 * Set the global error state
 * @param error The error code to set
 */
void kvs_set_error(kvs_error_t error);


/**
 * Get the current error state
 * @return The current error code
 */
kvs_error_t kvs_get_error(void);


/**
 * Get a human-redable desc of error
 * @param error The error code to desc
 * @return String desc of the error
 */
const char* kvs_error_string(kvs_error_t error);


/**
 * Clear the error state (set to KVS_SUCCESS)
 */
void kvs_clear_error(void);

 #endif