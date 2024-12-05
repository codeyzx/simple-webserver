#include "HTTP_Server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h> // For exit and EXIT_FAILURE
#include <unistd.h> // For close
#include <string.h> // For memset, memcpy, and strncat

// HTTP status codes as per https://www.w3.org/Protocols/HTTP/HTRESP.html
const char _status_code_text[8][50] = {
	"HTTP/1.1 200 OK\r\n\r\n",
	"HTTP/1.1 201 CREATED\r\n\r\n",
	"HTTP/1.1 400 Bad request\r\n\r\n",
	"HTTP/1.1 401 Unauthorized\r\n\r\n",
	"HTTP/1.1 403 Forbidden\r\n\r\n",
	"HTTP/1.1 404 Not found\r\n\r\n",
	"HTTP/1.1 500 Internal Error\r\n\r\n",
	"HTTP/1.1 501 Not implemented\r\n\r\n"
};

// Set the HTTP status code in the response
void http_set_status_code(HTTP_Server *http_server, const enum http_status_code_e status_code) {
	// Clear the existing status code
	memset(http_server->status_code, 0, HTTP_STATUS_CODE_LEN);
	// Copy the new status code from the predefined list
	const char *_status = _status_code_text[status_code];
	memcpy(http_server->status_code, _status, strlen(_status));
}

// Set the HTTP response body, including the status code
void http_set_response_body(HTTP_Server *http_server, const char *body) {
	// Clear the existing response
	memset(http_server->response, 0, HTTP_RESPONSE_BODY_LEN);
	// Copy the status code to the response
	memcpy(http_server->response, http_server->status_code, strlen(http_server->status_code));

	// Calculate the maximum length for the response body
	size_t max_len = (size_t)HTTP_RESPONSE_BODY_LEN - strlen(http_server->status_code) - 5;
	// Append the response body to the response
	strncat(http_server->response, body, max_len);
	// Append the HTTP response terminator
	strncat(http_server->response, "\r\n\r\n", 5);
}

// Create a TCP socket and set socket options
int create_tcp_socket() {
	// Create a socket using IPv4 and TCP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	// Set socket options to reuse the address
	int opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("Failed to set socket options");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	return sockfd;
}

// Bind the socket to the specified port
void bind_socket(int sockfd, int port) {
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; // Use IPv4
	server_address.sin_port = htons(port); // Convert port to network byte order
	server_address.sin_addr.s_addr = INADDR_ANY; // Bind to any available network interface

	// Bind the socket to the address and port
	if (bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		perror("Failed to bind socket");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

// Start listening for incoming connections
void start_listening(int sockfd) {
	// Listen for incoming connections with a backlog of 5
	if (listen(sockfd, 5) < 0) {
		perror("Failed to listen on socket");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}