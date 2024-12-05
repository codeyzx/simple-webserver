// Render a static file and return its content as a string
// The file is read from disk, and its content is returned as a dynamically allocated string
// The caller is responsible for freeing the memory allocated for the string
char *render_static_file(char *fileName);
