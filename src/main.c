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

// Fungsi untuk menangani permintaan GET
void handle_get_request(int client_socket, struct Route* route, char* urlRoute) {
    HTTP_Server http_server;
    char template[100] = "";

    if (strstr(urlRoute, "/static/") != NULL) {
        // Misalnya, mencari file statis CSS
        strcat(template, "static/index.css");
        http_set_status_code(&http_server, OK);
    } else {
        // Mencari route yang sesuai untuk template HTML
        struct Route* destination = search(route, urlRoute);
        strcat(template, "templates/");

        if (destination == NULL) {
            strcat(template, "404.html");
            http_set_status_code(&http_server, NOT_FOUND);
        } else {
            strcat(template, destination->value);
            http_set_status_code(&http_server, OK);
        }
    }

    char *response_data = render_static_file(template);
    http_set_response_body(&http_server, response_data);

    send(client_socket, http_server.response, sizeof(http_server.response), 0);
    close(client_socket);
    free(response_data);
}

// Fungsi untuk menangani permintaan POST
void handle_post_request(int client_socket, char* urlRoute, char* client_msg) {
    HTTP_Server http_server;

    // Contoh penanganan data POST (misalnya, form data)
    struct ParameterArray* params = paramInit(10);
    paramParse(params, client_msg);

    // Simpan atau proses data sesuai dengan urlRoute
    // Misalnya, simpan ke file atau database (belum diimplementasikan dalam contoh ini)

    // Respons POST berhasil
    http_set_status_code(&http_server, CREATED);
    http_set_response_body(&http_server, "Resource created successfully");

    send(client_socket, http_server.response, sizeof(http_server.response), 0);
    close(client_socket);
    paramClear(params);
    paramFree(params);
}

// Fungsi untuk menangani permintaan PUT
void handle_put_request(int client_socket, char* urlRoute, char* client_msg) {
    HTTP_Server http_server;

    // Menangani permintaan PUT untuk memperbarui resource
    // Misalnya, update data sesuai URL atau body permintaan
    struct ParameterArray* params = paramInit(10);
    paramParse(params, client_msg);

    // Update data berdasarkan parameter yang diterima (belum diimplementasikan)

    // Respons PUT berhasil
    http_set_status_code(&http_server, OK);
    http_set_response_body(&http_server, "Resource updated successfully");

    send(client_socket, http_server.response, sizeof(http_server.response), 0);
    close(client_socket);
    paramClear(params);
    paramFree(params);
}

// Fungsi untuk menangani permintaan DELETE
void handle_delete_request(int client_socket, char* client_msg) {
    HTTP_Server http_server;
    char *urlRoute = "";

    // Parsing header untuk mendapatkan URL
    char *client_http_header = strtok(client_msg, "\n");
    char *header_token = strtok(client_http_header, " ");
    int header_parse_counter = 0;

    while (header_token != NULL) {
        switch (header_parse_counter) {
            case 1:
                urlRoute = header_token;
                break;
        }
        header_token = strtok(NULL, " ");
        header_parse_counter++;
    }

    printf("The route to delete is %s\n", urlRoute);

    // Menghapus file yang terkait dengan URL
    char file_path[256] = "data/";
    strcat(file_path, urlRoute + 1);  // Menghilangkan "/" dari URL dan menyambungkan

    if (remove(file_path) == 0) {
        printf("Resource deleted successfully\n");
        http_set_status_code(&http_server, OK);
        http_set_response_body(&http_server, "Resource deleted successfully");
    } else {
        printf("Failed to delete resource\n");
        http_set_status_code(&http_server, NOT_FOUND);
        http_set_response_body(&http_server, "Resource not found");
    }

    send(client_socket, http_server.response, sizeof(http_server.response), 0);
    close(client_socket);
}

// Program utama untuk menjalankan server
int main() {
	// initiate HTTP_Server
    HTTP_Server http_server;
    init_server(&http_server, 6969);
    int client_socket;

    // Menginisialisasi routing
    struct Route* route = initRoute("/", "index.html");
    addRoute(&route, "/about", "about.html");
    addRoute(&route, "/sth", "sth.html");
    addRoute(&route, "/chicken", "chicken.html");

    // Menampilkan semua route yang terdaftar
    printf("\n====================================\n");
    printf("=========ALL AVAILABLE ROUTES========\n");
    inorder(route);

    while (1) {
        char client_msg[4096] = "";
        client_socket = accept(http_server.socket, NULL, NULL);
        read(client_socket, client_msg, 4095);

		// parsing client socket header to get HTTP method, route
        char *method = "";
        char *urlRoute = "";
        char *client_http_header = strtok(client_msg, "\n");

        char *header_token = strtok(client_http_header, " ");
        int header_parse_counter = 0;

        // Parsing header HTTP untuk mendapatkan metode dan URL
        while (header_token != NULL) {
            switch (header_parse_counter) {
                case 0:
                    method = header_token;
                    break;
                case 1:
                    urlRoute = header_token;
                    break;
            }
            header_token = strtok(NULL, " ");
            header_parse_counter++;
        }

        // Menangani berbagai metode HTTP
        if (strcmp(method, "GET") == 0) {
            handle_get_request(client_socket, route, urlRoute);
        } else if (strcmp(method, "POST") == 0) {
            handle_post_request(client_socket, urlRoute, client_msg);
        } else if (strcmp(method, "PUT") == 0) {
            handle_put_request(client_socket, urlRoute, client_msg);
        } else if (strcmp(method, "DELETE") == 0) {
            handle_delete_request(client_socket, client_msg);
        } else {
            // Jika metode tidak dikenali
            http_set_status_code(&http_server, BAD_REQUEST);
            http_set_response_body(&http_server, "Method Not Allowed");
            send(client_socket, http_server.response, sizeof(http_server.response), 0);
            close(client_socket);
        }
    }

    return 0;
}
