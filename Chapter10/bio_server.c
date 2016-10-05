/**
    Professional Linux Network Programming - Chapter 8 - server.c
    By Nathan Yocom, plnp@yocom.org
*/

/* 
    Standard includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
/*
    SSL includes
 */
#include <openssl/ssl.h>
#include <openssl/err.h>

int main(int argc, char *argv[]) {
    SSL_METHOD *my_ssl_method;   
    SSL_CTX *my_ssl_ctx;              
    SSL *my_ssl;
    BIO *server_bio,*client_bio;
    int error = 0, wrote = 0;
    char buffer[] = "Hello there! Welcome to the SSL test server.\n\n";

    OpenSSL_add_all_algorithms();   
    SSL_load_error_strings();       

    my_ssl_method = TLSv1_server_method();  

    if((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    SSL_CTX_use_certificate_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM); 
    SSL_CTX_use_PrivateKey_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);

    if(!SSL_CTX_check_private_key(my_ssl_ctx)) {
        fprintf(stderr,"Private key does not match certificate\n");
        exit(-1);
    }

    if((server_bio = BIO_new_accept("5353")) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    if(BIO_do_accept(server_bio) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    for(;;) {
        if(BIO_do_accept(server_bio) <= 0) {
            ERR_print_errors_fp(stderr);
            exit(-1);
        }

        client_bio = BIO_pop(server_bio);
        
        if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
            ERR_print_errors_fp(stderr);
            exit(-1);
        }

        SSL_set_bio(my_ssl,client_bio,client_bio);

        if(SSL_accept(my_ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            exit(-1);
        }

        printf("Connection made with [version,cipher]: [%s,%s]\n",SSL_get_version(my_ssl),SSL_get_cipher(my_ssl));

        for(wrote = 0; wrote < strlen(buffer); wrote += error) {
            error = SSL_write(my_ssl,buffer+wrote,strlen(buffer)-wrote);

            if(error <= 0)
                break;
        }
        
        SSL_shutdown(my_ssl);
        SSL_free(my_ssl);
    }

    SSL_CTX_free(my_ssl_ctx);
    SSL_BIO_free(server_bio);

    return 0;
}
