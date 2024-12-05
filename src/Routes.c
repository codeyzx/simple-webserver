#include "Routes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initialize a new route node with the given key and value
struct Route *initRoute(char *key, char *value) {
	struct Route *temp = (struct Route *)malloc(sizeof(struct Route));
	temp->key = key; // Set the key for the route
	temp->value = value; // Set the value for the route
	temp->left = temp->right = NULL; // Initialize left and right children as NULL
	return temp;
}

// Perform in-order traversal of the binary search tree and print the routes
void inorder(struct Route *root) {
	if (root != NULL) {
		inorder(root->left); // Traverse the left subtree
		printf("%s -> %s \n", root->key, root->value); // Print the current node
		inorder(root->right); // Traverse the right subtree
	}
}

// Add a new route to the binary search tree
void addRoute(struct Route **root, char *key, char *value) {
	if (*root == NULL) {
		*root = initRoute(key, value); // Initialize the root if it is NULL
	} else if (strcmp(key, (*root)->key) == 0) {
		// If the key already exists, print a warning
		printf("============ WARNING ============\n");
		printf("A Route For \"%s\" Already Exists\n", key);
	} else if (strcmp(key, (*root)->key) > 0) {
		// If the key is greater, add to the right subtree
		addRoute(&(*root)->right, key, value);
	} else {
		// If the key is smaller, add to the left subtree
		addRoute(&(*root)->left, key, value);
	}
}

// Search for a route by key in the binary search tree
struct Route *search(struct Route *root, char *key) {
	if (root == NULL || strcmp(key, root->key) == 0) {
		// Return the node if found or if the tree is empty
		return root;
	} else if (strcmp(key, root->key) > 0) {
		// If the key is greater, search in the right subtree
		return search(root->right, key);
	} else {
		// If the key is smaller, search in the left subtree
		return search(root->left, key);
	}
}
