# HTTP Server Project 🚀

This project implements a simple HTTP server in C. It includes functionalities for routing, parameter parsing, and serving static files.

## Visualization of this repo

![Visualization of this repo](./diagram.svg)


## Features 🌟

- **HTTP Request Handling (GET, POST, PUT, DELETE)**: Handle various HTTP request methods.
- **Routing Management with Binary Search Tree (BST)**: Efficiently manage routes using a BST.
- **URL Input Validation**: Validate URL inputs to ensure they are well-formed.
- **Child Process Handling with Fork and Semaphore**: Manage child processes using `fork()` and semaphores for concurrency.
- **Logging and Debugging**: Log server activities and debug information.
- **Unit Testing for Function Validation**: Validate server functions using unit tests.
- **Zombie Processes Handling**: Properly handle zombie processes to prevent resource leaks.


## Files 📁

- **HTTP_Server.c**: Contains functions to initialize the server, set status codes, and set response bodies.
- **Routes.c**: Manages routes using a binary search tree structure.
- **Parameters.c**: Handles dynamic parameter arrays, including initialization, addition, sorting, and retrieval.
- **main.c**: The main entry point of the server, handling client connections and requests.


## Getting Started 🛠️
Follow these steps to set up and run the HTTP server on your machine.

### Prerequisites
- Windows Subsystem for Linux (WSL)
- GCC Compiler

### Installation
1. Install WSL:
```shell
wsl --install
```
2. Clone the Repository:
```shell
git clone https://github.com/codeyzx/simple-webserver.git
cd simple-webserver
```
2. Build the Project:
```shell
make
```
3. Run the Server:
```shell
./build/webserver
```

### Running Unit Tests
Unit tests are included to ensure the server functions as expected.
1. Navigate to the test directory:
```shell
cd test
```
2. Run the tests:
```shell
make run_tests
```


## Authors

1. Yahya Alfon Sinaga (231524064)
2. Amadeus (231524050)
