#ifndef __COMELANG_NET_H__
#define __COMELANG_NET_H__

#include <neo-c.h>

using c
{
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef __LINUX__
#include <endian.h>
#endif
#ifdef __MAC__
#include <libkern/OSByteOrder.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>

static int neo_setsockopt_reuseaddr(int sock)
{
    int opt = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}
}

using neo-c-net;

typedef int socket_fd;

uniq Result<int>*% socket_fd::write(socket_fd self, string str)
{
    if(self < 0 || str == null) {
        return new Result<int>.None();
    }

    int result = write(self, str, str.length());

    if(result < 0 || result != str.length()) {
        return new Result<int>.None();
    }

    return new Result<int>.Some(result);
}

class server_socket_iterator
{
    int port;
    int socket_family;
    int socket_type;
    int protocol;
    bool reuse;
    socket_fd sock;
    socket_fd current_socket;
    bool initialized;
    bool reconnect_next;

    new(int port=8080, int socket_family=AF_INET, int socket_type=SOCK_STREAM, int protocol=0, bool reuse=false) {
        self.port = port;
        self.socket_family = socket_family;
        self.socket_type = socket_type;
        self.protocol = protocol;
        self.reuse = reuse;
        self.sock = -1;
        self.current_socket = -1;
        self.initialized = false;
        self.reconnect_next = false;
    }

    void initialize_listener() {
        self.sock = socket(self.socket_family, self.socket_type, self.protocol);
        if(self.sock < 0) {
            die("socket failed");
        }

#ifndef __ANDROID__
        if(self.reuse) {
            if(neo_setsockopt_reuseaddr(self.sock)) {
                close(self.sock);
                self.sock = -1;
                die(s"setsockopt");
            }
        }
#endif

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(self.port);

        if(bind(self.sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Unable to bind");
        }

        if(listen(self.sock, 3) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Unable to listen");
        }

        self.initialized = true;
    }

    socket_fd accept_client() {
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        socklen_t addrlen = sizeof(address);

        socket_fd new_socket = accept(self.sock, (struct sockaddr *)&address, &addrlen);
        if(new_socket < 0) {
            die(s"Unable to accept");
        }

        return new_socket;
    }

    socket_fd begin() {
        if(!self.initialized) {
            self.initialize_listener();
        }

        if(self.current_socket < 0) {
            self.current_socket = self.accept_client();
        }

        return self.current_socket;
    }

    socket_fd next() {
        if(self.reconnect_next) {
            if(self.current_socket >= 0) {
                close(self.current_socket);
            }
            self.current_socket = self.accept_client();
            self.reconnect_next = false;
        }

        return self.current_socket;
    }

    bool end() {
        return self.sock < 0;
    }

    void reconnect() {
        self.reconnect_next = true;
    }

    server_socket_iterator* iter() {
        return self;
    }
}

void server_socket_iterator*::finalize(server_socket_iterator* self)
{
    if(self.current_socket >= 0) {
        close(self.current_socket);
    }
    if(self.sock >= 0) {
        close(self.sock);
    }
}

uniq server_socket_iterator*% server_socket(int port=8080, int socket_family=AF_INET, int socket_type=SOCK_STREAM, int protocol=0, bool reuse=false)
{
    return new server_socket_iterator(port, socket_family, socket_type, protocol, reuse);
}

class client_socket_iterator
{
    int port;
    string address;
    socket_fd sock;
    bool initialized;

    new(int port=8080, char* address="127.0.0.1") {
        self.port = port;
        self.address = string(address);
        self.sock = -1;
        self.initialized = false;
    }

    void connect_socket() {
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));

        self.sock = socket(AF_INET, SOCK_STREAM, 0);
        if(self.sock < 0) {
            die(s"socket");
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(self.port);

        if(inet_pton(AF_INET, self.address, &serv_addr.sin_addr) <= 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Invalid address/ Address not supported");
        }
        if(connect(self.sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Connection Failed");
        }
    }

    socket_fd begin() {
        if(!self.initialized) {
            self.connect_socket();
            self.initialized = true;
        }

        return self.sock;
    }

    socket_fd next() {
        return self.sock;
    }

    bool end() {
        return self.sock < 0;
    }

    client_socket_iterator* iter() {
        return self;
    }
}

uniq Result<string>*% client_socket2(int port, const char* data, const char* address="127.0.0.1")
{
    if(data == null || address == null) {
        return new Result<string>.None();
    }

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return new Result<string>.None();
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        close(sock);
        return new Result<string>.None();
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return new Result<string>.None();
    }
    
    int write_result = write(sock, data, strlen(data));
    if(write_result < 0 || write_result != strlen(data)) {
        close(sock);
        return new Result<string>.None();
    }
    
    buffer*% buf = new buffer();
    
    char buf2[1024] = { '\0' };

    int size = read(sock, buf2, 1024);
    
    if(size < 0) {
        close(sock);
        return new Result<string>.None();
    }
    
    buf.append(buf2, size);

    close(sock);
    
    return new Result<string>.Some(buf.to_string());
}

void client_socket_iterator*::finalize(client_socket_iterator* self)
{
    if(self.sock >= 0) {
        close(self.sock);
    }
}

uniq client_socket_iterator*% client_socket(int port=8080, char* address="127.0.0.1")
{
    return new client_socket_iterator(port, address);
}

class httpd_socket_iterator
{
    int port;
    int socket_family;
    int socket_type;
    int protocol;
    bool reuse;
    socket_fd sock;
    socket_fd current_socket;
    bool initialized;

    new(int port=8080, int socket_family=AF_INET, int socket_type=SOCK_STREAM, int protocol=0, bool reuse=false) {
        self.port = port;
        self.socket_family = socket_family;
        self.socket_type = socket_type;
        self.protocol = protocol;
        self.reuse = reuse;
        self.sock = -1;
        self.current_socket = -1;
        self.initialized = false;
    }

    void initialize_listener() {
        self.sock = socket(self.socket_family, self.socket_type, self.protocol);
        if(self.sock < 0) {
            die("socket failed");
        }

#ifndef __ANDROID__
        if(self.reuse) {
            if(neo_setsockopt_reuseaddr(self.sock)) {
                close(self.sock);
                self.sock = -1;
                die(s"setsockpt failed");
            }
        }
#endif

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(self.port);

        if(bind(self.sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Unable to bind");
        }

        if(listen(self.sock, 3) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Unable to listen");
        }

        self.initialized = true;
    }

    socket_fd accept_client() {
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        socklen_t addrlen = sizeof(address);

        socket_fd new_socket = accept(self.sock, (struct sockaddr *)&address, &addrlen);
        if(new_socket < 0) {
            die(s"Unable to accept");
        }

        return new_socket;
    }

    socket_fd begin() {
        if(!self.initialized) {
            self.initialize_listener();
        }

        self.current_socket = self.accept_client();
        return self.current_socket;
    }

    socket_fd next() {
        if(self.current_socket >= 0) {
            close(self.current_socket);
        }
        self.current_socket = self.accept_client();
        return self.current_socket;
    }

    bool end() {
        return self.sock < 0;
    }

    httpd_socket_iterator* iter() {
        return self;
    }
}

uniq void ERR_print_errors_fp(FILE* f)
{
/*
unsigned long err = ERR_get_error();  // エラーコードを取得
char err_str[256];  // エラーメッセージを格納するためのバッファ
ERR_error_string_n(err, err_str, sizeof(err_str));  // エラーメッセージを文字列に変換
fprintf(f, "OpenSSL Error: %s\n", err_str);  // エラーメッセージを標準エラーに出力
*/
}

void httpd_socket_iterator*::finalize(httpd_socket_iterator* self)
{
    if(self.current_socket >= 0) {
        close(self.current_socket);
    }
    if(self.sock >= 0) {
        close(self.sock);
    }
}

uniq httpd_socket_iterator*% httpd_socket(int port=8080, int socket_family=AF_INET, int socket_type=SOCK_STREAM, int protocol=0, bool reuse=false)
{
    return new httpd_socket_iterator(port, socket_family, socket_type, protocol, reuse);
}

class httpsd_socket_iterator
{
    int port;
    bool reuse;
    socket_fd sock;
    socket_fd client;
    SSL_CTX* ctx;
    SSL* ssl;
    bool initialized;

    new(int port=443, bool reuse=false) {
        self.port = port;
        self.reuse = reuse;
        self.sock = -1;
        self.client = -1;
        self.ctx = null;
        self.ssl = null;
        self.initialized = false;
    }

    void initialize_listener() {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        const SSL_METHOD *method = SSLv23_server_method();

        self.ctx = SSL_CTX_new(method);
        if(self.ctx == null) {
            die(s"Unable to create SSL context");
        }

        if(SSL_CTX_use_certificate_file(self.ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
            die(s"SSL_CTX_use_certificate_file");
        }

        if(SSL_CTX_use_PrivateKey_file(self.ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
            die(s"SSL_CTX_use_PrivateKey_file");
        }

        self.sock = socket(AF_INET, SOCK_STREAM, 0);
        if(self.sock < 0) {
            die(s"Unable to create socket");
        }

        if(self.reuse) {
            if(neo_setsockopt_reuseaddr(self.sock)) {
                close(self.sock);
                self.sock = -1;
                die(s"setsockopt");
            }
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(self.port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if(bind(self.sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Unable to bind");
        }

        if(listen(self.sock, 1) < 0) {
            close(self.sock);
            self.sock = -1;
            die(s"Unable to listen");
        }

        self.initialized = true;
    }

    void clear_current() {
        if(self.ssl != null) {
            SSL_shutdown(self.ssl);
            SSL_free(self.ssl);
            self.ssl = null;
        }
        if(self.client >= 0) {
            close(self.client);
            self.client = -1;
        }
    }

    SSL* accept_client() {
        while(true) {
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            socklen_t len = sizeof(addr);
            self.client = accept(self.sock, (struct sockaddr*)&addr, &len);

            if(self.client < 0) {
                die(s"Unable to accept");
            }

            self.ssl = SSL_new(self.ctx);
            SSL_set_fd(self.ssl, self.client);

            if(SSL_accept(self.ssl) <= 0) {
                ERR_print_errors_fp(stdout);
                self.clear_current();
                continue;
            }

            return self.ssl;
        }
    }

    SSL* begin() {
        if(!self.initialized) {
            self.initialize_listener();
        }

        return self.accept_client();
    }

    SSL* next() {
        self.clear_current();
        return self.accept_client();
    }

    bool end() {
        return self.sock < 0;
    }

    httpsd_socket_iterator* iter() {
        return self;
    }
}

void httpsd_socket_iterator*::finalize(httpsd_socket_iterator* self)
{
    self.clear_current();
    if(self.sock >= 0) {
        close(self.sock);
    }
    if(self.ctx != null) {
        SSL_CTX_free(self.ctx);
    }
    EVP_cleanup();
}

uniq httpsd_socket_iterator*% httpsd_socket(int port=443, bool reuse=false)
{
    return new httpsd_socket_iterator(port, reuse);
}

impl server_socket_iterator
{
    iter_begin iter() {
        ({
            int _dummy_result = 0;
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _dummy_result
        })
    }
    iter filter() {
        bool result = `block();

        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();

        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;

        if(!_skip_while_done) \{
            result = `block();

            if(!result) \{
                _skip_while_done = true;
            \}
        \}

        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();

        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end end() {
    }
}

impl client_socket_iterator
{
    iter_begin iter() {
        ({
            int _dummy_result = 0;
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _dummy_result
        })
    }
    iter filter() {
        bool result = `block();

        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();

        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;

        if(!_skip_while_done) \{
            result = `block();

            if(!result) \{
                _skip_while_done = true;
            \}
        \}

        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();

        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end end() {
    }
}

impl httpd_socket_iterator
{
    iter_begin iter() {
        ({
            int _dummy_result = 0;
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _dummy_result
        })
    }
    iter filter() {
        bool result = `block();

        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();

        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;

        if(!_skip_while_done) \{
            result = `block();

            if(!result) \{
                _skip_while_done = true;
            \}
        \}

        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();

        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end end() {
    }
}

impl httpsd_socket_iterator
{
    iter_begin iter() {
        ({
            int _dummy_result = 0;
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _dummy_result
        })
    }
    iter filter() {
        bool result = `block();

        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();

        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;

        if(!_skip_while_done) \{
            result = `block();

            if(!result) \{
                _skip_while_done = true;
            \}
        \}

        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();

        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end end() {
    }
}

#endif
