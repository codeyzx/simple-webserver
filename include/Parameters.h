#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stddef.h>
#include <stdbool.h>

// Parameter structure representing a key-value pair
// Used to store individual parameters parsed from a URL
struct Parameter
{
	char *key;   // Key of the parameter
	char *value; // Value associated with the key
};

// ParameterArray structure representing an array of parameters
// The array can dynamically grow and supports sorting and searching
struct ParameterArray
{
	struct Parameter *parameters; // Array of parameters
	bool is_sorted;               // Flag indicating if the array is sorted
	size_t n_members;             // Number of parameters currently in the array
	size_t capacity;              // Capacity of the array
};

// Initialize an array of parameters with the given capacity
// Returns a pointer to the newly created ParameterArray
struct ParameterArray *paramInit(const size_t capacity);

// Parse parameters from a URL string and add them to the parameter array
// The URL string is expected to be in the format "key1=value1&key2=value2"
void paramParse(struct ParameterArray *params, char *url);

// Clear all keys and values in the parameter array
// The array is reset to its initial state, but its capacity remains unchanged
void paramClear(struct ParameterArray *params);

// Add a new parameter to the parameter array
// Returns true if the parameter was added successfully, false otherwise
bool paramAdd(struct ParameterArray *params, const char *key, const char *value);

// Sort parameters by keys in ascending order
// The array is sorted in place using a comparison function
void paramSort(struct ParameterArray *params);

// Return a pointer to a parameter with the given key
// Returns NULL if the key is not found in the array
struct Parameter *paramGet(struct ParameterArray *params, const char *key);

// Free all memory consumed by the parameter array
// The array and all its elements are deallocated
void paramFree(struct ParameterArray *params);

#endif
