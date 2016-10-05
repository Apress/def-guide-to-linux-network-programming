/**
    Professional Linux Network Programming - Chapter 10 - server.c
    By Nathan Yocom, nate@yocom.org
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
    BIO *my_bio;
    int error = 0, read_in = 0;
    char buffer[512];

    memset(buffer,'\0',sizeof(buffer));

    OpenSSL_add_all_algorithms();   
    SSL_load_error_strings();       

    my_ssl_method = TLSv1_client_method();  

    if((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    if((my_bio = BIO_new_connect("127.0.0.1:5353")) == NULL) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }
    
    if(BIO_do_connect(my_bio) <=0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    SSL_set_bio(my_ssl,my_bio,my_bio);

    if(SSL_connect(my_ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(-1);
    }

    printf("Connection made with [version,cipher]: [%s,%s]\n",SSL_get_version(my_ssl),SSL_get_cipher(my_ssl));
    
    for( read_in = 0; read_in < sizeof(buffer); read_in += error ) {
        error = SSL_read(my_ssl,buffer+read_in,sizeof(buffer) - read_in);
        if(error <= 0)
            break;
    } 

    SSL_shutdown(my_ssl);
    SSL_free(my_ssl);
    SSL_CTX_free(my_ssl_ctx);

    printf("%s",buffer);

    return 0; 

    report_error("Report error (not quit) test\n",__FILE__,__LINE__,0);
    report_error_q("Report error (quit) test\n",__FILE__,__LINE__,0);
    return 0;
}
