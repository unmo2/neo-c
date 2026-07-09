#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Mock database server to detect SQL injection patterns */
static int mock_db_socket = -1;
static char captured_query[4096] = {0};

static void* mock_db_server(void *arg) {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3366);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 1);
    
    int client = accept(server_sock, NULL, NULL);
    if (client >= 0) {
        recv(client, captured_query, sizeof(captured_query) - 1, 0);
        close(client);
    }
    close(server_sock);
    return NULL;
}

START_TEST(test_sql_injection_parameterization)
{
    /* Invariant: SQL injection payloads must not appear unescaped in queries */
    const char *payloads[] = {
        "' OR '1'='1",           /* Classic SQL injection */
        "'; DROP TABLE users; --", /* Destructive injection */
        "admin' --",             /* Authentication bypass */
        "user123"                /* Valid benign input */
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        memset(captured_query, 0, sizeof(captured_query));
        
        /* Start mock DB server */
        pthread_t db_thread;
        pthread_create(&db_thread, NULL, mock_db_server, NULL);
        usleep(100000);
        
        /* Call login function with payload as username */
        /* This would call: client_socket2(3366, "INSERT INTO users(username, password) VALUES('\{payloads[i]}', 'pass')") */
        char query[512];
        snprintf(query, sizeof(query), "INSERT INTO users(username, password) VALUES('%s', 'pass')", payloads[i]);
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(3366);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            send(sock, query, strlen(query), 0);
            close(sock);
        }
        
        pthread_join(db_thread, NULL);
        
        /* Assert: SQL injection patterns must be escaped or parameterized */
        if (i < 3) { /* For injection payloads */
            ck_assert_msg(
                strstr(captured_query, "' OR") == NULL && 
                strstr(captured_query, "DROP TABLE") == NULL &&
                strstr(captured_query, "'; ") == NULL,
                "SQL injection payload detected unescaped in query: %s", captured_query
            );
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("SQL_Injection_Security");
    tc_core = tcase_create("Parameterization");

    tcase_add_test(tc_core, test_sql_injection_parameterization);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}