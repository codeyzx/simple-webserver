#ifndef SERVER_H
#define SERVER_H

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

#define BUFFER_SIZE 4096
#define MAX_CHILDREN 100000

// Semaphore for managing child processes
extern sem_t child_semaphore;

// Signal handler for SIGCHLD to clean up child processes
void handle_sigchld(int sig);

// Validate input string
int is_valid_input(const char *input);

// Handle GET request
void handle_get_request(int client_socket, struct Route *route, char *url_route);

// Handle POST request
void handle_post_request(int client_socket, char *url_route, char *client_msg, char *result);

// Handle PUT request
void handle_put_request(int client_socket, char *url_route, char *client_msg);

// Handle DELETE request
void handle_delete_request(int client_socket, char *client_msg);

// Handle client connection based on the request type
void handle_client_connection(int client_socket, struct Route *route);

// Initialize the HTTP server
void init_server(HTTP_Server *http_server, int port);

// Initialize semaphore
void init_semaphore();

// Initialize signal handling
void init_signal_handling();

// Initialize routing table
struct Route* init_routing();

// Display all registered routes
void display_routes(struct Route *route);

// Handle client connection
void handle_client(int client_socket, struct Route *route);

// Accept incoming connections and handle them
void accept_connections(HTTP_Server *http_server, struct Route *route);

#endif