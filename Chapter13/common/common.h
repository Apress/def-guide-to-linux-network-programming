/**
 * Common Code for Authentication Server and Client
 * Chapter 13 - "The Definitive Guide to Linux Network Programming"
 */
 
#ifndef COMMON_CODE_H
#define COMMON_CODE_H

#include <stdlib.h> // Needed for size_t
#include <stdio.h>  // Needed for fprintf and stderr
#include <string.h> // For strlen etc

#include <openssl/ssl.h>    // OpenSSL header files for openssl_init 
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <sys/types.h>	// Includes for the network function
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>

#include <unistd.h>

#include <pwd.h> // Include for getpwent


#define ENTROPY_SIZE 512    // Define a constant for controlling how many bytes to seed PRNG with

#define byte_t char         // Define a constant to be used for a single byte data type, if sizeof(char) > 1 on your system, this should
                            //  be changed to something that is sizeof(??) == 1
                            
#define REQUEST_KEY_AUTH            10                  // Client message tells the server to go into Key authentication mode
#define REQUEST_PASS_AUTH           11                  // Client message tells the server to go into Password auth mode
#define SERVER_AUTH_SUCCESS         1                   // Server message tells the client that authentication was successful
#define SERVER_AUTH_FAILURE         2                   // Server message tells the client that authentication failed
#define SSL_ERROR                   0                   // If ssl_read_uint returns 0 it is an error

// Report an error, then exit the thread/program                
void report_error_q(const char *msg, const char *file, int line_no, int use_perror);
// Report an error without exiting
void report_error(const char *msg, const char *file, int line_no, int use_perror);


// The structure we use to maintain a list of allocated memory
typedef struct memory_list {
		void *address;
		size_t size;
		struct memory_list *next;
		struct memory_list *prev;
} memory_list;

// Memory management wrappers for leak detection 
void *w_malloc(size_t bytes);
void w_free(void *f_address);
void w_free_all(void);

// The global memory allocation list used by the memory management wrappers
memory_list *memory_list_head;
// The global couter indicating the number of bytes allocated
unsigned long global_memory_count;

// A one-time initialization function to setup the memory list and global memory count variables
void w_memory_init(void);

// A one-time initializeation function to setup and initialize OpenSSL, including seeding the PRNG
void openssl_init(void);

// A function that is registered to be called when the program terminates successfully, this cleans
//  up memory used by openssl_init automagically
void openssl_destroy(void);

// SSL Management wrapper allows us to read a null terminated string
char *ssl_read_string(SSL *my_ssl,size_t limit);
// SSL Management wrapper allows us to write a null terminated string
void ssl_write_string(SSL *my_ssl,const char *message);
// SSL Management wrapper allows us to read an unsigned int
unsigned int ssl_read_uint(SSL *my_ssl);
// SSL Management wrapper allows us to write an unsigned int
void ssl_write_uint(SSL *my_ssl,unsigned int value);
// SSL Management wrapper allows us to read a single byte
byte_t ssl_read_byte(SSL *my_ssl);
// SSL Management wrapper allows us to read a stream of bytes
int ssl_read_bytes(SSL *my_ssl,void *buf,unsigned int limit);
// SSL Management wrapper allows us to write a single byte
void ssl_write_byte(SSL *my_ssl,byte_t this_byte);
// SSL Management wrapper allows us to write a stream of bytes
void ssl_write_bytes(SSL *my_ssl, void *message, unsigned int length);

// A Network management wrapper allows us to get the IP address of a client
const char *network_get_ip_address(SSL *my_ssl);

// Create a new RSA Key
RSA * key_create_key(void); 
// Destroy a key in memory
void key_destroy_key(RSA *);
// Determine maximum signed data buffer size
unsigned int key_buffer_size(RSA *);      
// Sign data 
unsigned int key_sign_data(RSA *,const char *,unsigned int,char *,unsigned int); 
// Write a private key to a file
int key_write_priv(RSA*, char *);
// Read a private key from a file
RSA *key_read_priv(char *);
// Write a public key over the network
void key_net_write_pub(RSA *,SSL *); 
// Verify private key with public key 
int key_verify_signature(RSA *, char *,unsigned int,char *,unsigned int); 
// Write a public key to a file
int key_write_pub(RSA*, char *);
// Read public key from the network
RSA *key_net_read_pub(SSL *);      
// Read a public key from a file
RSA *key_read_pub(char *);

#endif
