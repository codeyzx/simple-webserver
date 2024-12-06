#include "unit_test.h"
#include <string.h>

void setUp(void)
{
    // Optional: Add any setup code here
}

void tearDown(void)
{
    // Optional: Add any teardown code here
}

// Mock function to simulate sending response
void mock_send_response(int client_socket, const char *response) {
    printf("Sending response: %s\n", response);
}

// Unit test for setting HTTP status code
void test_http_set_status_code(void) {
    HTTP_Server server;
    
    http_set_status_code(&server, OK);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1 200 OK\r\n\r\n", server.status_code);
    
    http_set_status_code(&server, NOT_FOUND);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1 404 Not found\r\n\r\n", server.status_code);
    
    http_set_status_code(&server, BAD_REQUEST);
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1 400 Bad request\r\n\r\n", server.status_code);
}

// Unit test for setting response body
void test_http_set_response_body(void) {
    HTTP_Server server;
    http_set_status_code(&server, OK);
    const char *body = "Hello, World!";
    
    http_set_response_body(&server, body);
    TEST_ASSERT_TRUE(strstr(server.response, "HTTP/1.1 200 OK\r\n\r\n"));
    TEST_ASSERT_TRUE(strstr(server.response, body));
}

// Unit test for handling GET requests
void test_handle_get_request(void) {
    // Mock function to simulate sending response
    char result[100];

    // Initialize the routing table
    struct Route *route = initRoute("/", "index.html");

    // Add the /about route
    addRoute(&route, "/about", "about.html");
    
    // handle the GET request
    handle_get_request(0, route, "/about", result);

    // Check if the response contains the status code
    TEST_ASSERT_TRUE(strstr(result, "HTTP/1.1 200 OK"));
}

// Unit test for input validation
void test_is_valid_input(void) {
    TEST_ASSERT_TRUE(is_valid_input("/valid/path"));
    TEST_ASSERT_FALSE(is_valid_input("/invalid/path with spaces"));
    TEST_ASSERT_FALSE(is_valid_input("/path/with/special#character"));
}

// Unit test for POST requests (echo)
void test_handle_post_request_echo(void) {
    char message[] = "Hello"; // message to be echoed
    char client_request[100]; // client request string
    char result[100]; // response from server

    // create POST request string
    sprintf(client_request, "POST /echo HTTP/1.1\r\n\r\n{\"message\": \"%s\"}", message);
    
    // handle the POST request
    handle_post_request(0, "/echo", client_request, result);
    
    // check if the response contains the message
    TEST_ASSERT_TRUE(strstr(result, message));
}

// Unit test for PUT requests
void test_handle_put_request(void) {
    HTTP_Server server;
    char client_request[] = "PUT /update HTTP/1.1\r\n\r\nname=JohnDoe&age=31";
    
    handle_put_request(0, "/update", client_request);
    
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1 200 OK\r\n\r\n", server.status_code);
    TEST_ASSERT_TRUE(strstr(server.response, "Resource updated successfully"));
}

// Unit test for DELETE requests
void test_handle_delete_request(void) {
    // to delete a resource you need to create a new file first then you deleted it, and then check is the file still exists or not
    FILE *file = fopen("data/test.txt", "w");
    fclose(file);

    char client_request[] = "DELETE /test.txt HTTP/1.1\r\n\r\n";


    handle_delete_request(0, client_request);

    // Check if the file is deleted
    file = fopen("data/test.txt", "r");
    TEST_ASSERT_NULL(file);  // Ensure file no longer exists
    if (file) fclose(file); // Prevent invalid fclose call
}

// Unit test for parameter parsing
void test_param_parse(void) {
    struct ParameterArray *params = paramInit(10);
    
    char query[] = "name=JohnDoe&age=30&country=USA";
    paramParse(params, query);
    
    struct Parameter *name_param = paramGet(params, "name");
    struct Parameter *age_param = paramGet(params, "age");
    struct Parameter *country_param = paramGet(params, "country");
    
    TEST_ASSERT_EQUAL_STRING("JohnDoe", name_param->value);
    TEST_ASSERT_EQUAL_STRING("30", age_param->value);
    TEST_ASSERT_EQUAL_STRING("USA", country_param->value);
    
    paramFree(params);
}

#ifdef TEST_UNIT  // Menandakan bahwa ini adalah kompilasi untuk unit test

int main(void) {
    UNITY_BEGIN();  // Inisialisasi Unity
    chdir(".."); // Change directory to project root

    RUN_TEST(test_http_set_status_code);
    RUN_TEST(test_http_set_response_body);
    RUN_TEST(test_handle_get_request);
    RUN_TEST(test_is_valid_input);
    RUN_TEST(test_handle_post_request_echo);
    RUN_TEST(test_handle_delete_request);
    
    return UNITY_END();  // Menyelesaikan dan menampilkan hasil
}

#endif  // TEST_UNIT