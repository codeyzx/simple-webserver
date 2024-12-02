#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "HTTP_Server.h"
#include "Routes.h"
#include "Response.h"
#include "Parameters.h"

// Function to handle GET requests
void handle_get_request(int client_socket, struct Route *route, char *url_route)
{
	HTTP_Server http_server;
	char template_path[100] = "";

	// Check if the request is for a static file
	if (strstr(url_route, "/static/") != NULL)
	{
		strcat(template_path, "static/index.css");
		http_set_status_code(&http_server, OK);
	}
	else
	{
		// Search for the route in the routing table
		struct Route *destination = search(route, url_route);
		strcat(template_path, "templates/");

		// If the route is not found, return 404
		if (destination == NULL)
		{
			strcat(template_path, "404.html");
			http_set_status_code(&http_server, NOT_FOUND);
		}
		else
		{
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
void handle_post_request(int client_socket, char *url_route, char *client_msg)
{
	HTTP_Server http_server;
	struct ParameterArray *params = paramInit(10);
	paramParse(params, client_msg);

	// Set response status and body
	http_set_status_code(&http_server, CREATED);
	http_set_response_body(&http_server, "Resource created successfully");

	// Send the response
	send(client_socket, http_server.response, sizeof(http_server.response), 0);
	close(client_socket);
	paramClear(params);
	paramFree(params);
}

// Function to handle PUT requests
void handle_put_request(int client_socket, char *url_route, char *client_msg)
{
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
void handle_delete_request(int client_socket, char *client_msg)
{
	HTTP_Server http_server;
	char *url_route = "";

	// Parse the HTTP header to get the URL
	char *client_http_header = strtok(client_msg, "\n");
	char *header_token = strtok(client_http_header, " ");
	int header_parse_counter = 0;

	while (header_token != NULL)
	{
		if (header_parse_counter == 1)
		{
			url_route = header_token;
		}
		header_token = strtok(NULL, " ");
		header_parse_counter++;
	}

	printf("The route to delete is %s\n", url_route);

	// Construct the file path and attempt to delete the file
	char file_path[256] = "data/";
	strcat(file_path, url_route + 1);

	if (remove(file_path) == 0)
	{
		printf("Resource deleted successfully\n");
		http_set_status_code(&http_server, OK);
		http_set_response_body(&http_server, "Resource deleted successfully");
	}
	else
	{
		printf("Failed to delete resource\n");
		http_set_status_code(&http_server, NOT_FOUND);
		http_set_response_body(&http_server, "Resource not found");
	}

	// Send the response
	send(client_socket, http_server.response, sizeof(http_server.response), 0);
	close(client_socket);
}

// Function to handle ECHO requests
void handle_echo_request(int client_socket, char *client_msg)
{
	char response[4096];
	snprintf(response, sizeof(response),
			 "HTTP/1.1 200 OK\r\n"
			 "Content-Length: %ld\r\n"
			 "Content-Type: text/plain\r\n"
			 "\r\n"
			 "%s",
			 strlen(client_msg), client_msg);

	// Send the response
	send(client_socket, response, strlen(response), 0);
	close(client_socket);
}

// Function to handle client connection
void handle_client_connection(int client_socket, struct Route *route)
{
	char client_msg[4096] = "";
	int bytes_read = read(client_socket, client_msg, 4095);
	if (bytes_read < 0)
	{
		perror("Failed to read from socket");
		close(client_socket);
		exit(1);
	}
	client_msg[bytes_read] = '\0';

	// Parse the HTTP header to get the method and URL
	char *method = "";
	char *url_route = "";
	char *client_http_header = strtok(client_msg, "\n");
	char *header_token = strtok(client_http_header, " ");
	int header_parse_counter = 0;

	while (header_token != NULL)
	{
		if (header_parse_counter == 0)
		{
			method = header_token;
		}
		else if (header_parse_counter == 1)
		{
			url_route = header_token;
		}
		header_token = strtok(NULL, " ");
		header_parse_counter++;
	}

	// Handle different HTTP methods
	if (strcmp(method, "GET") == 0)
	{
		handle_get_request(client_socket, route, url_route);
	}
	else if (strcmp(method, "POST") == 0)
	{
		handle_post_request(client_socket, url_route, client_msg);
	}
	else if (strcmp(method, "PUT") == 0)
	{
		handle_put_request(client_socket, url_route, client_msg);
	}
	else if (strcmp(method, "DELETE") == 0)
	{
		handle_delete_request(client_socket, client_msg);
	}
	else if (strcmp(method, "ECHO") == 0)
	{
		handle_echo_request(client_socket, client_msg);
	}
	else
	{
		// If the method is not recognized, return 400
		HTTP_Server http_server;
		http_set_status_code(&http_server, BAD_REQUEST);
		http_set_response_body(&http_server, "Method Not Allowed");
		send(client_socket, http_server.response, strlen(http_server.response), 0);
		close(client_socket);
	}

	close(client_socket);
}

// Main function to run the server
int main()
{
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

	while (1)
	{
		client_socket = accept(http_server.socket, NULL, NULL);
		if (client_socket < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				continue;
			}
			else
			{
				perror("Failed to accept connection");
				continue;
			}
		}

		pid_t pid = fork();
		if (pid < 0)
		{
			perror("Failed to fork");
			close(client_socket);
			continue;
		}

		if (pid == 0)
		{
			// Child process
			close(http_server.socket);
			handle_client_connection(client_socket, route);
			exit(0);
		}
		else
		{
			// Parent process
			close(client_socket);
		}
	}

	return 0;
}

// Function to initialize the server
void init_server(HTTP_Server *http_server, int port)
{
	http_server->port = port;

	int server_socket = create_tcp_socket();
	bind_socket(server_socket, port);
	start_listening(server_socket);

	http_server->socket = server_socket;
	printf("HTTP Server Initialized\nPort: %d\n", http_server->port);
}