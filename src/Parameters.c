#include "Parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Print critical memory allocation error
static void _print_critical() {
	printf("============ CRITICAL ============\n");
	printf("Memory could not be allocated for parameters.\n");
}

// Initialize a ParameterArray with a given capacity
struct ParameterArray *paramInit(const size_t capacity) {
	struct ParameterArray *params = malloc(sizeof(struct ParameterArray));
	if (!params) {
		// Print error and return NULL if memory allocation fails
		_print_critical();
		return NULL;
	}

	params->parameters = calloc(capacity, sizeof(struct Parameter));
	if (!params->parameters) {
		// Print error, free allocated memory, and return NULL if memory allocation fails
		_print_critical();
		free(params);
		return NULL;
	}

	params->is_sorted = false; // Initialize is_sorted flag as false
	params->n_members = 0; // Initialize number of members as 0
	params->capacity = capacity; // Set the capacity

	return params;
}

// Add a key-value pair to the ParameterArray
bool paramAdd(struct ParameterArray *params, const char *key, const char *value) {
	if (params->n_members == params->capacity) {
		// Double the capacity if the array is full
		void *alloc = realloc(params->parameters, params->capacity * 2 * sizeof(struct Parameter));
		if (!alloc) {
			// Print error and return false if memory allocation fails
			_print_critical();
			return false;
		}
		params->capacity *= 2;
		params->parameters = alloc;
	}

	// Add the key-value pair to the array
	params->parameters[params->n_members].key = strdup(key);
	params->parameters[params->n_members].value = strdup(value);
	params->is_sorted = false; // Mark the array as unsorted
	params->n_members++; // Increment the number of members
	return true;
}

// Compare keys for sorting
static int key_cmp(const void *a, const void *b) {
	const struct Parameter *_a = a;
	const struct Parameter *_b = b;
	return strcmp(_a->key, _b->key); // Compare the keys
}

// Sort the ParameterArray by keys
void paramSort(struct ParameterArray *params) {
	qsort(params->parameters, params->n_members, sizeof(struct Parameter), key_cmp);
	params->is_sorted = true; // Mark the array as sorted
}

// Get a parameter by key
struct Parameter *paramGet(struct ParameterArray *params, const char *key) {
	if (!params->is_sorted) {
		paramSort(params); // Sort the array if it is not sorted
	}

	struct Parameter search_key = { .key = (char *)key, .value = NULL }; // Create a search key
	return bsearch(&search_key, params->parameters, params->n_members, sizeof(struct Parameter), key_cmp); // Perform binary search
}

// Parse URL parameters and add them to the ParameterArray
void paramParse(struct ParameterArray *params, char *url) {
	char *url_copy = strdup(url); // Create a copy of the URL
	memset(url, 0, strlen(url)); // Clear the original URL

	char key[1024] = {0}; // Buffer to store the key
	char value[1024] = {0}; // Buffer to store the value
	char parsing_rule = 0; // Parsing rule: 0 = not parsing, 'k' = parsing key, 'v' = parsing value

	for (size_t i = 0; i < strlen(url_copy); ++i) {
		switch (url_copy[i]) {
			case '?':
				parsing_rule = 'k'; // Start parsing key
				continue;
			case '=':
				parsing_rule = 'v'; // Start parsing value
				continue;
			case '&':
				paramAdd(params, key, value); // Add the key-value pair to the array
				memset(key, 0, 1024); // Clear the key buffer
				memset(value, 0, 1024); // Clear the value buffer
				parsing_rule = 'k'; // Start parsing next key
				continue;
			default:
				if (parsing_rule == 0) {
					strncat(url, &url_copy[i], 1); // Append character to the original URL
				}
				break;
		}

		switch (parsing_rule) {
			case 'k':
				strncat(key, &url_copy[i], 1); // Append character to the key buffer
				break;
			case 'v':
				strncat(value, &url_copy[i], 1); // Append character to the value buffer
				break;
		}
	}

	paramAdd(params, key, value); // Add the last key-value pair to the array
	free(url_copy); // Free the URL copy
}

// Clear all parameters in the ParameterArray
void paramClear(struct ParameterArray *params) {
	for (size_t i = 0; i < params->n_members; ++i) {
		free(params->parameters[i].key); // Free the key
		free(params->parameters[i].value); // Free the value
	}
	params->n_members = 0; // Reset the number of members
}

// Free the ParameterArray
void paramFree(struct ParameterArray *params) {
	paramClear(params); // Clear all parameters
	free(params->parameters); // Free the parameters array
	free(params); // Free the ParameterArray structure
}
