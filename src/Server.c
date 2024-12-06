#include "Server.h"

// Semaphore to limit the number of child processes
sem_t child_semaphore;

// Signal handler for SIGCHLD to reap zombie processes
void handle_sigchld(int sig) {
	(void)sig; // Suppress unused parameter warning
	while (waitpid(-1, NULL, WNOHANG) > 0); // Reap all available zombie processes
}

// Function to validate the input URL
int is_valid_input(const char *input) {
	for (size_t i = 0; input[i] != '\0'; ++i) {
		// Check if each character is alphanumeric or one of the allowed special characters
		if (!isalnum(input[i]) && input[i] != '/' && input[i] != '.' && input[i] != '-') {
			return 0; // Return 0 if an invalid character is found
		}
	}
	return 1; // Return 1 if all characters are valid
}

// Function to handle GET requests
void handle_get_request(int client_socket, struct Route *route, char *url_route, char *result) {
	HTTP_Server http_server;
	char template_path[100] = "";

	// Log request URL
    printf("\n\nHandling GET request for URL: %s\n", url_route);

	// Validate the URL
	if (!is_valid_input(url_route)) {
		http_set_status_code(&http_server, BAD_REQUEST); // Set HTTP status to 400 Bad Request
		http_set_response_body(&http_server, "Invalid URL"); // Set response body
		send(client_socket, http_server.response, strlen(http_server.response), 0); // Send response
		strcpy(result, http_server.status_code); // Copy the status code to the result
		close(client_socket); // Close the client socket
		return;
	}

	// Check if the request is for a static file
	if (strstr(url_route, "/static/") != NULL) {
		strcat(template_path, "static/index.css"); // Set path to static file
		http_set_status_code(&http_server, OK); // Set HTTP status to 200 OK
	} else {
		// Search for the route in the routing table
		struct Route *destination = search(route, url_route);
		strcat(template_path, "templates/");

		if (destination == NULL) {
			strcat(template_path, "404.html"); // Set path to 404 page
			http_set_status_code(&http_server, NOT_FOUND); // Set HTTP status to 404 Not Found
		} else {
			strcat(template_path, destination->value); // Set path to the destination file
			http_set_status_code(&http_server, OK); // Set HTTP status to 200 OK
		}
	}

	// Render the static file and send the response
	char *response_data = render_static_file(template_path);
	http_set_response_body(&http_server, response_data);

	// Log response status code
    printf("Response sent with status: %s\n", http_server.status_code);
	send(client_socket, http_server.response, strlen(http_server.response), 0); // Send response
	strcpy(result, http_server.status_code); // Copy the status code to the result

	close(client_socket); // Close the client socket
	free(response_data); // Free the allocated memory for response data
}

// Function to handle POST requests
void handle_post_request(int client_socket, char *url_route, char *client_msg, char *result) {
	HTTP_Server http_server;

	// Handle the /echo route
	if (strcmp(url_route, "/echo") == 0) {
		char *json_start = strstr(client_msg, ":") + 3; // Find the start of the JSON value
		char *json_end = strstr(json_start, "\"") - 1; // Find the end of the JSON value
		char client_msg[256];
		strncpy(client_msg, json_start, json_end - json_start + 1); // Copy the JSON value
		client_msg[json_end - json_start + 1] = '\0'; // Null-terminate the string

		printf("\n\nMessage from client: %s\n", client_msg); // Print the message from the client

		char response[BUFFER_SIZE];
		snprintf(response, sizeof(response),
				 "HTTP/1.1 200 OK\r\n"
				 "Content-Length: %ld\r\n"
				 "Content-Type: application/json\r\n"
				 "\r\n"
				 "{\"result\": \"%s\"}\n",
				 strlen(client_msg) + 14 + 1, client_msg); // Create the response
		
		send(client_socket, response, strlen(response), 0); // Send the response
	} else {
		http_set_status_code(&http_server, NOT_FOUND); // Set HTTP status 404 Not Found
		http_set_response_body(&http_server, "Resource not found"); // Set response body
		send(client_socket, http_server.response, sizeof(http_server.response), 0); // Send response
	}

	strcpy(result, client_msg); // Copy the message to the result
	
	close(client_socket); // Close the client socket
}

// Function to handle PUT requests
void handle_put_request(int client_socket, char *url_route, char *client_msg) {
    HTTP_Server http_server;
    char file_path[256];

    // Menghapus '/' dari URL dan membangun path file
    snprintf(file_path, sizeof(file_path), "./data/%s", url_route + 1); // Menghilangkan '/' pertama

    // Log data yang diterima untuk debugging
    printf("\n\nReceived PUT request for URL: %s\n", url_route);
    printf("Data received: %s\n", client_msg);  // Menampilkan data dari client
    printf("File path: %s\n", file_path); // Menampilkan path file untuk debugging

    // Buka file untuk menulis (akan dibuat jika tidak ada)
    FILE *file = fopen(file_path, "w"); // Membuka file dalam mode tulis (overwrite)
    if (file == NULL) {
        // Jika file tidak ditemukan atau gagal dibuka
        printf("Failed to open file for writing: %s\n", file_path);
        http_set_status_code(&http_server, NOT_FOUND); // Set HTTP status 404 Not Found
        http_set_response_body(&http_server, "Failed to update resource"); // Set response body
        send(client_socket, http_server.response, sizeof(http_server.response), 0); // Send response
        close(client_socket); // Close the client socket
        return;
    }

    // Jika client mengirimkan data dalam format JSON yang valid
    // Tulis data yang diterima ke dalam file
    // Format data sebagai JSON jika perlu
    fprintf(file, "{\n");
    fprintf(file, "  \"data\": %s\n", client_msg); // Menulis data JSON dari client
    fprintf(file, "}\n");

    fclose(file); // Tutup file setelah menulis

    // Set HTTP status dan response body untuk mengkonfirmasi update berhasil
    http_set_status_code(&http_server, OK); // Set HTTP status 200 OK
    http_set_response_body(&http_server, "Resource updated successfully"); // Set response body

    // Kirim response ke client
    send(client_socket, http_server.response, sizeof(http_server.response), 0); // Send response
    close(client_socket); // Close the client socket
}

// Function to handle DELETE requests
void handle_delete_request(int client_socket, char *client_msg) {
	HTTP_Server http_server;
	char *url_route = "";

	// Parse the URL from the HTTP header
	char *client_http_header = strtok(client_msg, "\n");
	char *header_token = strtok(client_http_header, " ");
	int header_parse_counter = 0;

	while (header_token != NULL) {
		if (header_parse_counter == 1) {
			url_route = header_token; // Extract the URL route
		}
		header_token = strtok(NULL, " ");
		header_parse_counter++;
	}

	printf("The route to delete is %s\n", url_route); // Print the route to delete

	// Construct the file path and attempt to delete the file
	char file_path[256] = "data/";
	strcat(file_path, url_route + 1); // Construct the file path

	if (remove(file_path) == 0) {
		printf("Resource deleted successfully\n"); // Print success message
		http_set_status_code(&http_server, OK); // Set HTTP status to 200 OK
		http_set_response_body(&http_server, "Resource deleted successfully"); // Set response body
	} else {
		printf("Failed to delete resource\n"); // Print failure message
		http_set_status_code(&http_server, NOT_FOUND); // Set HTTP status to 404 Not Found
		http_set_response_body(&http_server, "Resource not found"); // Set response body
	}

	send(client_socket, http_server.response, sizeof(http_server.response), 0); // Send response
	close(client_socket); // Close the client socket
}

// Function to handle client connections
void handle_client_connection(int client_socket, struct Route *route) {
	HTTP_Server http_server;
	char client_msg[BUFFER_SIZE] = "";
	int bytes_read = read(client_socket, client_msg, BUFFER_SIZE - 1); // Read client message
	if (bytes_read < 0) {
		perror("Failed to read from socket"); // Print error message
		close(client_socket); // Close the client socket
		sem_post(&child_semaphore); // Release the semaphore
		return;
	}
	client_msg[bytes_read] = '\0'; // Null-terminate the client message

	// Parse the HTTP method and URL
	char method[16], url_route[256];
	if (sscanf(client_msg, "%15s %255s", method, url_route) != 2) {
		fprintf(stderr, "Invalid HTTP request format.\n"); // Print error message
		close(client_socket); // Close the client socket
		sem_post(&child_semaphore); // Release the semaphore
		return;
	}

	// Handle the request based on the HTTP method
	if (strcmp(method, "GET") == 0) {
		handle_get_request(client_socket, route, url_route, ""); // Handle GET request
	} else if (strcmp(method, "POST") == 0) {
		char *json_start = strstr(client_msg, "\r\n\r\n");
		if (json_start) {
			json_start += 4;
			handle_post_request(client_socket, url_route, json_start, ""); // Handle POST request
		}
	} else if (strcmp(method, "PUT") == 0) {
		char *json_start = strstr(client_msg, "\r\n\r\n");
		if (json_start) {
			json_start += 4;
			handle_put_request(client_socket, url_route, json_start); // Handle PUT request
		}
	} else if (strcmp(method, "DELETE") == 0) {
		handle_delete_request(client_socket, client_msg); // Handle DELETE request
	} else {
		http_set_status_code(&http_server, BAD_REQUEST); // Set HTTP status to 400 Bad Request
		http_set_response_body(&http_server, "Method Not Allowed"); // Set response body
		send(client_socket, http_server.response, strlen(http_server.response), 0); // Send response
	}

	close(client_socket); // Close the client socket
	sem_post(&child_semaphore); // Release the semaphore
}

// Function to initialize the server
void init_server(HTTP_Server *http_server, int port) {
	http_server->port = port; // Set the server port

	int server_socket = create_tcp_socket(); // Create a TCP socket
	bind_socket(server_socket, port); // Bind the socket to the port
	start_listening(server_socket); // Start listening for incoming connections

	http_server->socket = server_socket; // Set the server socket
	printf("HTTP Server Initialized\nPort: %d\n", http_server->port); // Print initialization message
}

// Function to initialize the semaphore
void init_semaphore() {
	sem_init(&child_semaphore, 0, MAX_CHILDREN); // Initialize the semaphore
}

// Function to initialize signal handling
void init_signal_handling() {
	signal(SIGCHLD, handle_sigchld); // Set the signal handler for SIGCHLD
}

// Function to initialize routing
struct Route* init_routing() {
	struct Route *route = initRoute("/", "index.html"); // Initialize the root route
	addRoute(&route, "/about", "about.html"); // Add the /about route
	addRoute(&route, "/sth", "sth.html"); // Add the /sth route
	addRoute(&route, "/chicken", "chicken.html"); // Add the /chicken route
	return route; // Return the routing table
}

// Function to display all available routes
void display_routes(struct Route *route) {
	printf("\n====================================\n");
	printf("=========ALL AVAILABLE ROUTES========\n");
	inorder(route); // Display the routes in order
}

// Function to handle client connections in a child process
void handle_client(int client_socket, struct Route *route) {
	handle_client_connection(client_socket, route); // Handle the client connection
	exit(0); // Exit the child process
}

// Function to accept incoming connections
void accept_connections(HTTP_Server *http_server, struct Route *route) {
	int client_socket;
	while (1) {
		client_socket = accept(http_server->socket, NULL, NULL); // Accept a new connection
		if (client_socket < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				continue; // Non-blocking mode, continue to the next iteration
			} else {
				perror("Failed to accept connection"); // Print error message
				continue; // Continue to the next iteration
			}
		}

		sem_wait(&child_semaphore); // Wait for an available slot

		pid_t pid = fork(); // Fork a new process
		if (pid < 0) {
			perror("Failed to fork"); // Print error message
			close(client_socket); // Close the client socket
			sem_post(&child_semaphore); // Release the semaphore
			continue; // Continue to the next iteration
		}

		if (pid == 0) {
			close(http_server->socket); // Close the server socket in the child process
			handle_client(client_socket, route); // Handle the client connection
		} else {
			close(client_socket); // Close the client socket in the parent process
		}
	}
}