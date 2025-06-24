/**
 * test.c - Comprehensive test suite for the key-value store
 * 
 * This file implements unit tests for all major functionality including
 * basic operations, persistence, error handling, and edge cases.
 */

#include "../include/kvstore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

/**
 * Test result tracking
 */
static int tests_run = 0;
static int tests_passed = 0;

/**
 * Test file name for persistence tests
 */
#define TEST_FILENAME "test_data.bin"

/**
 * Macro for running tests with automatic counting
 */
#define RUN_TEST(test_func) do { \
    printf("Running %s... ", #test_func); \
    tests_run++; \
    if (test_func()) { \
        printf("PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("FAILED\n"); \
    } \
} while (0)

/**
 * Test basic set and get operations
 */
static bool test_basic_operations(void) {
    kvstore_t* kvs = kvs_create(0);
    if (!kvs) return false;
    
    // Test setting and getting a value
    if (!kvs_set(kvs, 42, "hello world")) {
        kvs_destroy(kvs);
        return false;
    }
    
    const char* value = kvs_get(kvs, 42);
    if (!value || strcmp(value, "hello world") != 0) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test overwriting a value
    if (!kvs_set(kvs, 42, "goodbye world")) {
        kvs_destroy(kvs);
        return false;
    }
    
    value = kvs_get(kvs, 42);
    if (!value || strcmp(value, "goodbye world") != 0) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test getting non-existent key
    value = kvs_get(kvs, 99);
    if (value != NULL) {
        kvs_destroy(kvs);
        return false;
    }
    
    kvs_destroy(kvs);
    return true;
}

/**
 * Test deletion operations
 */
static bool test_deletion(void) {
    kvstore_t* kvs = kvs_create(0);
    if (!kvs) return false;
    
    // Set some values
    kvs_set(kvs, 1, "one");
    kvs_set(kvs, 2, "two");
    kvs_set(kvs, 3, "three");
    
    // Verify they exist
    if (kvs_count(kvs) != 3) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Delete one
    if (!kvs_delete(kvs, 2)) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Verify it's gone
    if (kvs_count(kvs) != 2) {
        kvs_destroy(kvs);
        return false;
    }
    
    if (kvs_get(kvs, 2) != NULL) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Verify others still exist
    if (!kvs_get(kvs, 1) || !kvs_get(kvs, 3)) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Try to delete non-existent key
    if (kvs_delete(kvs, 99)) {
        kvs_destroy(kvs);
        return false;
    }
    
    kvs_destroy(kvs);
    return true;
}

/**
 * Test persistence (save and load)
 */
static bool test_persistence(void) {
    // Clean up any existing test file
    unlink(TEST_FILENAME);
    
    // Create store and add data
    kvstore_t* kvs1 = kvs_create(0);
    if (!kvs1) return false;
    
    kvs_set(kvs1, 100, "one hundred");
    kvs_set(kvs1, 200, "two hundred");
    kvs_set(kvs1, 300, "three hundred");
    
    // Save to file
    if (!kvs_save(kvs1, TEST_FILENAME)) {
        kvs_destroy(kvs1);
        return false;
    }
    
    kvs_destroy(kvs1);
    
    // Create new store and load data
    kvstore_t* kvs2 = kvs_create(0);
    if (!kvs2) return false;
    
    if (!kvs_load(kvs2, TEST_FILENAME)) {
        kvs_destroy(kvs2);
        return false;
    }
    
    // Verify loaded data
    if (kvs_count(kvs2) != 3) {
        kvs_destroy(kvs2);
        return false;
    }
    
    const char* value = kvs_get(kvs2, 100);
    if (!value || strcmp(value, "one hundred") != 0) {
        kvs_destroy(kvs2);
        return false;
    }
    
    value = kvs_get(kvs2, 200);
    if (!value || strcmp(value, "two hundred") != 0) {
        kvs_destroy(kvs2);
        return false;
    }
    
    value = kvs_get(kvs2, 300);
    if (!value || strcmp(value, "three hundred") != 0) {
        kvs_destroy(kvs2);
        return false;
    }
    
    kvs_destroy(kvs2);
    unlink(TEST_FILENAME);  // Clean up
    return true;
}

/**
 * Test error handling
 */
static bool test_error_handling(void) {
    // Test invalid parameters
    kvstore_t* kvs = kvs_create(0);
    if (!kvs) return false;
    
    // NULL value should fail
    if (kvs_set(kvs, 1, NULL)) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test operations on NULL store
    if (kvs_set(NULL, 1, "test")) return false;
    if (kvs_get(NULL, 1) != NULL) return false;
    if (kvs_delete(NULL, 1)) return false;
    if (kvs_count(NULL) != 0) return false;
    
    // Test file operations with invalid filenames
    if (kvs_save(kvs, NULL)) {
        kvs_destroy(kvs);
        return false;
    }
    
    if (kvs_load(kvs, NULL)) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test loading non-existent file
    if (kvs_load(kvs, "non_existent_file.bin")) {
        kvs_destroy(kvs);
        return false;
    }
    
    kvs_destroy(kvs);
    return true;
}

/**
 * Test large dataset to verify scaling
 */
static bool test_large_dataset(void) {
    kvstore_t* kvs = kvs_create(0);
    if (!kvs) return false;
    
    const int NUM_ITEMS = 1000;
    char value_buffer[64];
    
    // Insert many items
    for (int i = 0; i < NUM_ITEMS; i++) {
        snprintf(value_buffer, sizeof(value_buffer), "value_%d", i);
        if (!kvs_set(kvs, i, value_buffer)) {
            kvs_destroy(kvs);
            return false;
        }
    }
    
    // Verify count
    if (kvs_count(kvs) != NUM_ITEMS) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Verify some random items
    for (int i = 0; i < NUM_ITEMS; i += 100) {
        snprintf(value_buffer, sizeof(value_buffer), "value_%d", i);
        const char* retrieved = kvs_get(kvs, i);
        if (!retrieved || strcmp(retrieved, value_buffer) != 0) {
            kvs_destroy(kvs);
            return false;
        }
    }
    
    // Delete half the items
    for (int i = 0; i < NUM_ITEMS; i += 2) {
        if (!kvs_delete(kvs, i)) {
            kvs_destroy(kvs);
            return false;
        }
    }
    
    // Verify count
    if (kvs_count(kvs) != NUM_ITEMS / 2) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Verify deleted items are gone and remaining items exist
    for (int i = 0; i < NUM_ITEMS; i++) {
        const char* retrieved = kvs_get(kvs, i);
        if (i % 2 == 0) {
            // Should be deleted
            if (retrieved != NULL) {
                kvs_destroy(kvs);
                return false;
            }
        } else {
            // Should still exist
            snprintf(value_buffer, sizeof(value_buffer), "value_%d", i);
            if (!retrieved || strcmp(retrieved, value_buffer) != 0) {
                kvs_destroy(kvs);
                return false;
            }
        }
    }
    
    kvs_destroy(kvs);
    return true;
}

/**
 * Test hash table resizing
 */
static bool test_resizing(void) {
    // Start with very small capacity to force resizing
    kvstore_t* kvs = kvs_create(2);
    if (!kvs) return false;
    
    // Insert enough items to trigger multiple resizes
    for (int i = 0; i < 100; i++) {
        char value[32];
        snprintf(value, sizeof(value), "item_%d", i);
        if (!kvs_set(kvs, i, value)) {
            kvs_destroy(kvs);
            return false;
        }
    }
    
    // Verify all items are still accessible
    for (int i = 0; i < 100; i++) {
        char expected[32];
        snprintf(expected, sizeof(expected), "item_%d", i);
        const char* retrieved = kvs_get(kvs, i);
        if (!retrieved || strcmp(retrieved, expected) != 0) {
            kvs_destroy(kvs);
            return false;
        }
    }
    
    kvs_destroy(kvs);
    return true;
}

/**
 * Test edge cases
 */
static bool test_edge_cases(void) {
    kvstore_t* kvs = kvs_create(0);
    if (!kvs) return false;
    
    // Test empty string value
    if (!kvs_set(kvs, 1, "")) {
        kvs_destroy(kvs);
        return false;
    }
    
    const char* value = kvs_get(kvs, 1);
    if (!value || strcmp(value, "") != 0) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test very long value
    char long_value[1000];
    memset(long_value, 'A', sizeof(long_value) - 1);
    long_value[sizeof(long_value) - 1] = '\0';
    
    if (!kvs_set(kvs, 2, long_value)) {
        kvs_destroy(kvs);
        return false;
    }
    
    value = kvs_get(kvs, 2);
    if (!value || strcmp(value, long_value) != 0) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test negative keys
    if (!kvs_set(kvs, -100, "negative key")) {
        kvs_destroy(kvs);
        return false;
    }
    
    value = kvs_get(kvs, -100);
    if (!value || strcmp(value, "negative key") != 0) {
        kvs_destroy(kvs);
        return false;
    }
    
    // Test zero key
    if (!kvs_set(kvs, 0, "zero key")) {
        kvs_destroy(kvs);
        return false;
    }
    
    value = kvs_get(kvs, 0);
    if (!value || strcmp(value, "zero key") != 0) {
        kvs_destroy(kvs);
        return false;
    }
    
    kvs_destroy(kvs);
    return true;
}

/**
 * Main test function
 */
int main(void) {
    printf("Running Key-Value Store Test Suite\n");
    printf("==================================\n\n");
    
    // Run all tests
    RUN_TEST(test_basic_operations);
    RUN_TEST(test_deletion);
    RUN_TEST(test_persistence);
    RUN_TEST(test_error_handling);
    RUN_TEST(test_large_dataset);
    RUN_TEST(test_resizing);
    RUN_TEST(test_edge_cases);
    
    // Print results
    printf("\n==================================\n");
    printf("Test Results: %d/%d tests passed\n", tests_passed, tests_run);
    
    if (tests_passed == tests_run) {
        printf("All tests PASSED! ✓\n");
        return 0;
    } else {
        printf("Some tests FAILED! ✗\n");
        return 1;
    }
}
