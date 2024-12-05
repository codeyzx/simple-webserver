#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stddef.h>

#define HTTP_STATUS_CODE_LEN 50
#define HTTP_RESPONSE_BODY_LEN 4096

// HTTP status codes enumeration
// Represents various HTTP status codes that can be returned by the server
enum http_status_code_e
{
	OK = 0,            // 200 OK
	CREATED,           // 201 Created
	BAD_REQUEST,       // 400 Bad Request
	UNAUTHORIZED,      // 401 Unauthorized
	FORBIDDEN,         // 403 Forbidden
	NOT_FOUND,         // 404 Not Found
	INTERNAL_ERROR,    // 500 Internal Server Error
	NOT_IMPLEMENTED    // 501 Not Implemented
};

// Defined in HTTP_Server.c
// Array of status code text representations
extern const char _status_code_text[8][50];

// HTTP_Server structure representing the server state
// Contains information about the server's socket, port, status code, and response body
typedef struct HTTP_Server
{
	int socket;                             // Server socket
	int port;                               // Port number
	char status_code[HTTP_STATUS_CODE_LEN]; // HTTP status code as a string
	char response[HTTP_RESPONSE_BODY_LEN];  // HTTP response body
} HTTP_Server;

// Initialize the HTTP server with the given port
// Sets up the server state and prepares it to accept connections
extern void init_server(HTTP_Server *http_server, int port);

// Create a TCP socket
// Returns the file descriptor for the new socket
int create_tcp_socket();

// Bind the socket to the given port
// Associates the socket with the specified port number
void bind_socket(int sockfd, int port);

// Start listening for incoming connections on the socket
// The server begins accepting connection requests from clients
void start_listening(int sockfd);

// Set the HTTP status code for the response
// Updates the status code in the server state
void http_set_status_code(HTTP_Server *http_server, const enum http_status_code_e);

// Set the response body for the HTTP response
// Updates the response body in the server state
void http_set_response_body(HTTP_Server *http_server, const char *body);

#endif