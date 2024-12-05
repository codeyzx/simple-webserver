#include "Response.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Render a static file and return its content as a string
char *render_static_file(char *fileName) {
	FILE *file = fopen(fileName, "r"); // Open the file in read mode
	if (file == NULL) {
		// Return NULL if the file does not exist
		return NULL;
	}

	fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
	long fsize = ftell(file); // Get the size of the file
	fseek(file, 0, SEEK_SET); // Move the file pointer back to the beginning

	char *content = malloc(sizeof(char) * (fsize + 1)); // Allocate memory for the file content
	if (content == NULL) {
		// Return NULL if memory allocation fails
		fclose(file);
		return NULL;
	}

	fread(content, 1, fsize, file); // Read the file content into the allocated memory
	content[fsize] = '\0'; // Null-terminate the string
	fclose(file); // Close the file
	return content; // Return the file content
}
