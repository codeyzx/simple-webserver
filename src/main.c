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
#include "Server.h"

int main() {
	// Initialize signal handling for child processes
	init_signal_handling();
	
	// Initialize semaphore for managing child processes
	init_semaphore();

	HTTP_Server http_server;
	// Initialize the HTTP server on port 6969
	init_server(&http_server, 6969);

	// Initialize routing table
	struct Route *route = init_routing();
	
	// Display all registered routes
	display_routes(route);

	// Start accepting connections and handle them
	accept_connections(&http_server, route);

	// Destroy semaphore before exiting
	sem_destroy(&child_semaphore);
	return 0;
}