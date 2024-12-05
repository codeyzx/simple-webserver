#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <signal.h>

#include "HTTP_Server.h"
#include "Routes.h"
#include "Response.h"
#include "Parameters.h"

#define BUFFER_SIZE 4096 // Maximum buffer size for client data
#define MAX_CHILDREN 100000 // Maximum number of active child processes

sem_t child_semaphore; // Semaphore to control the number of child processes

// Signal handler for SIGCHLD to clean up zombie processes
void handle_sigchld(int sig) {
    (void)sig; // Prevent unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0); // Reap all terminated children
}

// Function to validate client input
// Ensures that the input contains only alphanumeric characters, '/', '.', and '-'
int is_valid_input(const char *input) {
	for (size_t i = 0; input[i] != '\0'; ++i) {
		if (!isalnum(input[i]) && input[i] != '/' && input[i] != '.' && input[i] != '-') {
			return 0; // Invalid character found
		}
	}
	return 1; // All characters are valid
}

// Function to handle GET requests
// Processes the GET request, validates the URL, and serves the appropriate content
void handle_get_request(int client_socket, struct Route *route, char *url_route) {
	HTTP_Server http_server;
	char template_path[100] = "";

	// Validate URL
	if (!is_valid_input(url_route)) {
		http_set_status_code(&http_server, BAD_REQUEST);
		http_set_response_body(&http_server, "Invalid URL");
		send(client_socket, http_server.response, strlen(http_server.response), 0);
		close(client_socket);
		return;
	}

	// Check if the request is for a static file
	if (strstr(url_route, "/static/") != NULL) {
		strcat(template_path, "static/index.css");
		http_set_status_code(&http_server, OK);
	} else {
		// Search for the route in the routing table
		struct Route *destination = search(route, url_route);
		strcat(template_path, "templates/");

		// If the route is not found, return 404
		if (destination == NULL) {
			strcat(template_path, "404.html");
			http_set_status_code(&http_server, NOT_FOUND);
		} else {
			strcat(template_path, destination->value);
			http_set_status_code(&http_server, OK);
		}
	}

	// Render the static file and send the response
	char *response_data = render_static_file(template_path);
	http_set_response_body(&http_server, response_data);

	send(client_socket, http_server.response, strlen(http_server.response), 0);
	close(client_socket);
	free(response_data);
}

// Function to handle POST requests
// Processes the POST request, parses the client message, and responds accordingly
void handle_post_request(int client_socket, char *url_route, char *client_msg) {
	HTTP_Server http_server;

	if (strcmp(url_route, "/echo") == 0) {
		// Extract the message from the client request
		char *json_start = strstr(client_msg, ":") + 3;
		char *json_end = strstr(json_start, "\"") - 1;
		char client_msg[256];
		strncpy(client_msg, json_start, json_end - json_start + 1);
		client_msg[json_end - json_start + 1] = '\0';

		printf("Message from client: %s\n", client_msg);

		// Prepare response with newline at the end
		char response[BUFFER_SIZE];
		snprintf(response, sizeof(response),
				 "HTTP/1.1 200 OK\r\n"
				 "Content-Length: %ld\r\n"
				 "Content-Type: application/json\r\n"
				 "\r\n"
				 "{\"result\": \"%s\"}\n",
				 strlen(client_msg) + 14 + 1, client_msg);

		// Send response
		send(client_socket, response, strlen(response), 0);
	} else {
		struct ParameterArray *params = paramInit(10);
		paramParse(params, client_msg);

		// Set response status and body for other routes
		http_set_status_code(&http_server, CREATED);
		http_set_response_body(&http_server, "Resource created successfully");

		// Send the response
		send(client_socket, http_server.response, sizeof(http_server.response), 0);
		paramClear(params);
		paramFree(params);
	}

	close(client_socket);
}

// Function to handle PUT requests
// Processes the PUT request, parses the client message, and updates the resource
void handle_put_request(int client_socket, char *url_route, char *client_msg) {
	HTTP_Server http_server;
	struct ParameterArray *params = paramInit(10);
	paramParse(params, client_msg);

	// Set response status and body
	http_set_status_code(&http_server, OK);
	http_set_response_body(&http_server, "Resource updated successfully");

	// Send the response
	send(client_socket, http_server.response, sizeof(http_server.response), 0);
	close(client_socket);
	paramClear(params);
	paramFree(params);
}

// Function to handle DELETE requests
// Processes the DELETE request, parses the client message, and deletes the resource
void handle_delete_request(int client_socket, char *client_msg) {
	HTTP_Server http_server;
	char *url_route = "";

	// Parse the HTTP header to get the URL
	char *client_http_header = strtok(client_msg, "\n");
	char *header_token = strtok(client_http_header, " ");
	int header_parse_counter = 0;

	while (header_token != NULL) {
		if (header_parse_counter == 1) {
			url_route = header_token;
		}
		header_token = strtok(NULL, " ");
		header_parse_counter++;
	}

	printf("The route to delete is %s\n", url_route);

	// Construct the file path and attempt to delete the file
	char file_path[256] = "data/";
	strcat(file_path, url_route + 1);

	if (remove(file_path) == 0) {
		printf("Resource deleted successfully\n");
		http_set_status_code(&http_server, OK);
		http_set_response_body(&http_server, "Resource deleted successfully");
	} else {
		printf("Failed to delete resource\n");
		http_set_status_code(&http_server, NOT_FOUND);
		http_set_response_body(&http_server, "Resource not found");
	}

	// Send the response
	send(client_socket, http_server.response, sizeof(http_server.response), 0);
	close(client_socket);
}

// Function to handle client connection
// Reads the client request, determines the HTTP method, and calls the appropriate handler
void handle_client_connection(int client_socket, struct Route *route) {
	char client_msg[BUFFER_SIZE] = "";
	int bytes_read = read(client_socket, client_msg, BUFFER_SIZE - 1);
	if (bytes_read < 0) {
		perror("Failed to read from socket");
		close(client_socket);
		sem_post(&child_semaphore);
		return;
	}
	client_msg[bytes_read] = '\0';

	// Extract HTTP method and URL
	char method[16], url_route[256];
	if (sscanf(client_msg, "%15s %255s", method, url_route) != 2) {
		fprintf(stderr, "Invalid HTTP request format.\n");
		close(client_socket);
		sem_post(&child_semaphore);
		return;
	}

	// Validate HTTP method and call the appropriate handler
	if (strcmp(method, "GET") == 0) {
		handle_get_request(client_socket, route, url_route);
	} else if (strcmp(method, "POST") == 0) {
		char *json_start = strstr(client_msg, "\r\n\r\n");
		if (json_start) {
			json_start += 4; // Skip "\r\n\r\n" to get to JSON payload
			handle_post_request(client_socket, url_route, json_start);
		}
	} else if (strcmp(method, "PUT") == 0) {
		char *json_start = strstr(client_msg, "\r\n\r\n");
		if (json_start) {
			json_start += 4; // Skip "\r\n\r\n" to get to JSON payload
			handle_put_request(client_socket, url_route, json_start);
		}
	} else if (strcmp(method, "DELETE") == 0) {
		handle_delete_request(client_socket, client_msg);
	} else {
		HTTP_Server http_server;
		http_set_status_code(&http_server, BAD_REQUEST);
		http_set_response_body(&http_server, "Method Not Allowed");
		send(client_socket, http_server.response, strlen(http_server.response), 0);
	}

	close(client_socket);
	sem_post(&child_semaphore); // Release semaphore
}

// Function to initialize the server
// Sets up the server socket, binds it to the specified port, and starts listening for connections
void init_server(HTTP_Server *http_server, int port) {
	http_server->port = port;

	int server_socket = create_tcp_socket();
	bind_socket(server_socket, port);
	start_listening(server_socket);

	http_server->socket = server_socket;
	printf("HTTP Server Initialized\nPort: %d\n", http_server->port);
}

// Main function to run the server
// Sets up signal handling, initializes the semaphore, and starts accepting client connections
int main() {
	signal(SIGCHLD, handle_sigchld); // Handle zombie processes

	sem_init(&child_semaphore, 0, MAX_CHILDREN); // Initialize semaphore

	HTTP_Server http_server;
	init_server(&http_server, 6969);
	int client_socket;

	// Initialize routing
	struct Route *route = initRoute("/", "index.html");
	addRoute(&route, "/about", "about.html");
	addRoute(&route, "/sth", "sth.html");
	addRoute(&route, "/chicken", "chicken.html");

	// Display all registered routes
	printf("\n====================================\n");
	printf("=========ALL AVAILABLE ROUTES========\n");
	inorder(route);

	while (1) {
		client_socket = accept(http_server.socket, NULL, NULL);
		if (client_socket < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				continue;
			} else {
				perror("Failed to accept connection");
				continue;
			}
		}

		sem_wait(&child_semaphore); // Wait if too many child processes

		pid_t pid = fork();
		if (pid < 0) {
			perror("Failed to fork");
			close(client_socket);
			sem_post(&child_semaphore); // Release semaphore
			continue;
		}

		if (pid == 0) {
			// Child process
			close(http_server.socket);
			handle_client_connection(client_socket, route);
			exit(0);
		} else {
			// Parent process
			close(client_socket);
		}
	}

	sem_destroy(&child_semaphore); // Destroy semaphore
	return 0;
}