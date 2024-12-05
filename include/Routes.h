#ifndef ROUTES_H
#define ROUTES_H

// Route structure representing a node in a binary search tree
// Each node contains a key-value pair and pointers to the left and right child nodes
struct Route
{
	char *key;   // Key of the route
	char *value; // Value associated with the key
	struct Route *left, *right; // Pointers to the left and right child nodes
};

// Initialize a new route with the given key and value
// Returns a pointer to the newly created route
struct Route *initRoute(char *key, char *value);

// Add a new route to the binary search tree
// The tree is modified in place, and the new route is inserted in the correct position
void addRoute(struct Route **root, char *key, char *value);

// Search for a route by key in the binary search tree
// Returns a pointer to the route if found, otherwise returns NULL
struct Route *search(struct Route *root, char *key);

// Perform an inorder traversal of the binary search tree
// The traversal visits nodes in ascending order of their keys
void inorder(struct Route *root);

#endif // ROUTES_H
