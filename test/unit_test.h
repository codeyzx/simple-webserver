#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include "unity/src/unity.h"
#include "../include/HTTP_Server.h"
#include "../include/Routes.h"
#include "../include/Response.h"
#include "../include/Parameters.h"

// Deklarasi fungsi unit test
void test_http_set_status_code(void);
void test_http_set_response_body(void);
void test_handle_get_request(void);
void test_is_valid_input(void);
void test_handle_post_request_echo(void);
void test_handle_post_request_form(void);
void test_handle_put_request(void);
void test_handle_delete_request(void);
void test_param_parse(void);

#endif // UNIT_TEST_H
