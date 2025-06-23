#include "error.h"
#include <pthread.h>

// Global error state
static kvs_error_t global_error = KVS_SUCCESS;

// mutex for thread-safe error handling
static pthread_mutex_t error_mutex = PTHREAD_MUTEX_INITIALIZER;

// Set global error state
void kvs_set_error(kvs_error_t error){
    pthread_mutex_lock(&error_mutex);
    global_error = error;
    pthread_mutex_unlock(&error_mutex);
}

//Get the current error state
kvs_error_t kvs_get_error(void) {
    kvs_error_t error;
    pthread_mutex_lock(&error_mutex);
    error = global_error;
    pthread_mutex_unlock(&error_mutex);
    return error;
}

// Human-redable desc of error
const char* kvs_error_string(kvs_error_t error) {
    switch (error) {
        case KVS_SUCCESS:
            return "Success";
        case KVS_ERROR_MEMORY:
            return "Memory allcoation failed";
        case KVS_ERROR_KEY_NOT_FOUND:
            return "Key not found";
        case KVS_ERROR_INVALID_PARAM:
            return "Invalid parameter";
        case KVS_ERROR_FILE_IO:
            return "File I/O error";
        case KVS_ERROR_CORRUPTION:
            return "Data corruption detected";
        case KVS_ERROR_UNKNOWN:
        default:
            return "Unknown Error";
    }
}   


// clear the error state
void kvs_clear_error(void){
    kvs_set_error(KVS_SUCCESS);
}