/**
 * Common Code for Authentication Client and Server
 * By Nathan Yocom, 2004 - For APress Book "The Definitive Guide to Linux Network Programming"
 */
#include "common.h"

/**
 * Reports the error specified in msg to stderr, indicates the name of the file and line number the 
 *  error occured at as well.
 * 
 * @param msg The string to print as the message
 * @param file The name of the file, typically passed as __FILE__
 * @param line_no The line number within the file parameter, typically passed as __LINE__
 * @param use_perror Flag indicating whether the system call perror() should be used to report the error (non-zero), or not (zero)
 */
void report_error(const char *msg, const char *file, int line_no, int use_perror)
{
    fprintf(stderr,"[%s:%d] ",file,line_no);    // Print the file and line 

    if(use_perror != 0) {
        perror(msg);                            // Use perror
    } else {                                    //  otherwise
        fprintf(stderr, "%s\n",msg);            // Print the message
    }
}

/**
 * Reports the error specified in msg to stderr, indicates the name of the file and line number the 
 *  error occured at as well, exactly as in report_error(), and calls exit() when finished.
 * 
 * @param msg The string to print as the message
 * @param file The name of the file, typically passed as __FILE__
 * @param line_no The line number within the file parameter, typically passed as __LINE__
 * @param use_perror Flag indicating whether the system call perror() should be used to report the error (non-zero), or not (zero)
 * 
 * @return Does not return
 */
void report_error_q(const char *msg, const char *file, int line_no, int use_perror)
{
    report_error(msg,file,line_no,use_perror);
    exit(EXIT_FAILURE);
}

/**
 * Wraps the malloc() function so that we can keep track of where and how much memory we have allocated.  This 
 *   allows us to catch out of memory errors, as well as potentially monitor and report potential leaks.
 * 
 * @param bytes The number of bytes needed to be allocated
 * 
 * @return If successful the return value is the start address of the newly allocated memory, otherwise the function
 *   does not return.
 */
void * w_malloc(size_t bytes) {
    void *memory = NULL;                                // Store the allocated memory here
    memory_list *new_item = NULL;                       // Our new item structure
    memory = calloc(bytes,1);                           // Try to allocate the memory
    new_item = calloc(1,sizeof(struct memory_list));    // Then allocate memory for the list item as well
    if(memory) {                                        // Make sure the first allocation worked
        if(new_item == NULL) {                          // Make sure the second allocation worked
            report_error_q("Memory allocation error, no room for memory list item.",__FILE__,__LINE__,0);
        }
        global_memory_count += bytes + sizeof(struct memory_list);  // Increment our global byte count
        new_item->address = memory;                     // Initialize the new item's address pointer
        new_item->size = bytes;                         // Set the bytes parameter to the size of the buffer in ->address
        new_item->next = NULL;                          // Start with a NULL prev and next pointer
        new_item->prev = NULL;
        if(memory_list_head) {                          // If the list already has items
            new_item->next = memory_list_head;          // Then we just add this item to the head of the list
            memory_list_head->prev = new_item;
            memory_list_head = new_item;
        } else {
            memory_list_head = new_item;                // The list didn't have items, we add this as the head
        }
        return memory;                                  // Return the allocated memory
    } else {
        report_error_q("Memory allocation error, out of memory.",__FILE__,__LINE__,0);  // The first allocation failed
        return NULL;                                    // We don't actually get here, but the compiler will complain otherwise
    }
}

/**
 * Wraps the malloc() function so that we can keep track of where and how much memory we have allocated.  This 
 *   allows us to catch out of memory errors, as well as potentially monitor and report potential leaks.
 * 
 * @param bytes The number of bytes needed to be allocated
 * 
 * @return If successful nothing is returned, if something fails this function will not return and will report an error
 *   to stderr.
 */
void w_free(void *f_address) {
    memory_list *temp = NULL,*found = NULL;                         // Temporary pointers for list manipulation

    if(f_address == NULL)                                           // Can't free nothing ;)
        return;

    for(temp=memory_list_head;temp!=NULL;temp = temp->next) {       // Walk the memory list looking for an item
        if(temp->address == f_address) {                            //   with the same address as we are asked to free
            found = temp;                                           //   and note that we have found it then
            break;                                                  //   break from the for loop to save time
        }
    }

    if(!found) {                                                    // If we haven't found it, then we shouldn't free it
        report_error_q("Unable to free memory not previously allocated",__FILE__,__LINE__,0);   // Report this as an error
    }

    global_memory_count -= found->size + sizeof(struct memory_list); // Decrement our global byte count

    free(f_address);                                                // Actually free the data
                                                                    // Then remove the item from the list:
    if(found->prev)                                                 //   If there is an item previous to us
        found->prev->next = found->next;                            //      point it at the next item
    if(found->next)                                                 //   If there is an item after us
        found->next->prev = found->prev;                            //      point it at the previous item
    if(found == memory_list_head)                                   //   If we are the head of the list
        memory_list_head = found->next;                             //      move the head of the list up one

    free(found);                                                    // Now we can actually free the memory used by our item
}     

/**
 * Wrapper that allows us to free all allocated memory at exit time
 */
void w_free_all(void) {
   memory_list *temp = NULL;

   while(memory_list_head) {
       free(memory_list_head->address);
       temp = memory_list_head->next;
       free(memory_list_head);
       memory_list_head = temp;
   }
}

/**
 * This initialization function should be called only once (although it tries to prevent double calling errors) to
 *   initialize the global variables used for memory management/wrappers.  This includes setting the count of allocated
 *   memory to 0 and the head of the list to NULL.
 */ 
void w_memory_init(void) {
    static int state = 0;           // Initialize a static variable we can use to keep state

    if(state != 0)                  // If the variable is not zero then we have already been called
        return;                     //  do nothing but return
                                    // If the variable is 0 then we have not been called before
    state = 1;                      // Note that we have now been called
    memory_list_head = NULL;        // Set the head of the memory list to NULL (no items)
    global_memory_count = 0;        // Start the memory allocation count at 0
    atexit(w_free_all);             // Register to have w_free_all() called at normal termination
}

/** 
 * This initialization function should be called only once (although it tries to prevent double calling errors) to
 *   initialize the OpenSSL libraries and seed the PRNG with data from /dev/random.
 */
void openssl_init(void) {
    static int state = 0;           // Initialize a static variable we can use to keep state        
    int bytes_read = 0;

    if(state != 0)                  // If the variable is not zero then we have already been called 
        return;                     //  do nothing but return                                       
                                    // If the variable is 0 then we have not been called before     
    state = 1;                      // Note that we have now been called
    if(atexit(openssl_destroy))     // Register to have openssl_destroy automagically called on exit
        report_error("atexit() failed, openssl_destroy() will not be called on exit.",
                     __FILE__,__LINE__,0);  // Report (non-fatal) that this didn't work
    OpenSSL_add_all_algorithms();   // Initialize OpenSSL ciphers and digests
    SSL_load_error_strings();       // Load all available error strings for later use

    // Try to seed the PRNG with ENTROPY_SIZE bytes from /dev/random
    printf("Seeding PRNG with /dev/random, this may take a moment... ");
    fflush(stdout);                 // Make sure our message displays (not buffers)
    if((bytes_read = RAND_load_file("/dev/random",ENTROPY_SIZE)) != ENTROPY_SIZE) {
        report_error_q("Seeding PRNG failed",__FILE__,__LINE__,0);
    } 
    // If we get here, we are all done
    printf("Done\n");
    fflush(stdout);
}

/**
 * This function is registered to be called on normal program termination with the atexit() system call.  When
 *   the program terminates, whether by return or by a call to exit(), this function will be called and will then
 *   cleanup OpenSSL memory usage resulting from the openssl_init function.
 */
void openssl_destroy(void) {
    EVP_cleanup();                  // Cleanup from the OpenSSL_add_all_algorithms()
    ERR_free_strings();             // Cleanup from the SSL_load_error_strings()
}


/**
* Reads a string into a buffer of length limit that is created with w_malloc.
*	Strings longer than 'limit' characters will be terminated and subsequent
*	calls will read the remainder of the string.  If an error occurs
*	the string as it stands is returned.  If no error occurs, the resulting string is returned.
*   Calling functions should take care to ensure they use w_free() to release
*   the string returned.
*
* 	@param my_ssl The SSL session to read from
*   @param limit The maximum number of bytes to read before returning
*	@return A NULL terminated string created with w_malloc(), the string may be truncated or empty on error
*/
char *ssl_read_string(SSL *my_ssl,size_t limit)
{                                                         
    char * buffer = NULL;                           // The buffer to store the string in
    char this_one;                                  // The last read byte
    int error = 0, read_in = 0;                     // Counters for our read loop

    buffer = w_malloc(limit);                       // Allocate space for the string read in

    while(read_in < limit) {                        // Ensure we don't overflow
        error = SSL_read(my_ssl,&this_one,1);       // Read a single byte from SSL, this doesn't seem
                                                    //   very optimized, but SSL does a lot of internal buffering
                                                    //   that prevents this from being an overly large sacrifice
        if(error > 0) {                             // As long as we read some data
            buffer[read_in++] = this_one;           //    Insert that data into our string
            if(this_one == '\0') return buffer;     //    Check to see if it was null, and if so, return the string as it stands
        } else {                                    // SSL_read returned an error
            return buffer;                          //    Return whatever we have read up to this point
        }
    }
    // If we get here, then we did not encounter an \0 character
    //  before reaching the limit of our buffer.  
    buffer[limit-1]='\0';                           //  Terminate the string and truncate
    return buffer;                                  //  and return the resulting string
}

/**
* Write a NULL terminated string over an SSL connection.  If an error occurs writing simply stops and this function
*  returns.
* 	@param my_ssl The ssl connection to use
*	@param message The message to write
*/
void ssl_write_string(SSL *my_ssl,const char *message)
{
    int ret_val = 0, bytes_written = 0;             // Counters and state management for the write loop
    int bytes_to_write;                             // Counter to keep track of where we are in the message

    bytes_to_write = strlen(message) + 1;           // We start by needing the send the whole message

    while(bytes_written < bytes_to_write) {         // While there are bytes to write
        ret_val = SSL_write(my_ssl, message + bytes_written, bytes_to_write - bytes_written); // Write as many as we can
        if(ret_val <= 0) {
            break;                                  // Break out of our loop if an error (i.e. SSL_SHUTDOWN) occurs
        } else {
            bytes_written += ret_val;               // Otherwise increment our count by the number of bytes written so we 
                                                    //  can loop and send the rest in subsequent iterations
        }
    }
}

/** 
* Read an unsigned int from the SSL connection given.  We use the ntohl() 
*   function to ensure proper endianess
* 
* @param my_ssl The SSL connection to read from
* @return An unsigned integer value, this value is 0 on error
*/
unsigned int ssl_read_uint(SSL *my_ssl)
{
    unsigned int value = 0;       // Our default return is 0 for error
    
    if(ssl_read_bytes(my_ssl,&value,sizeof(unsigned int)) != -1) { // If reading was successful
        value = ntohl(value);                                      // Ensure proper endianess
        return value;                                              // and return the value
    } else
        return 0;                                                  // If we had an error, return 0
}

/**
* Write an unsigned int to the given SSL connection, we use htonl to ensure
*   proper endianess.
*/
void ssl_write_uint(SSL *my_ssl,unsigned int value)
{
    unsigned int to_write = 0;                                      // The buffer we will actually write
    to_write = htonl(value);                                        // Assign the buffer the network byte order ver of value
    ssl_write_bytes(my_ssl,&to_write,sizeof(unsigned int));         // Write the buffer
}

/**
* Read a single byte from the SSL connection.  
*
*	@param my_ssl The SSL connection to read from
*   @return The byte read
*/
byte_t ssl_read_byte(SSL *my_ssl)
{
    byte_t this_byte;
    if(SSL_read(my_ssl,&this_byte,sizeof(byte_t)) != 1)          // Try to read a single byte
        return '\0';                                              //  Return NULL on error
    else {
        return this_byte;
    }
        
}

/**
*  Read a stream of bytes from an SSL connection.
*
*   @param my_ssl The ssl connection to use
*   @param buf    The buffer to read into
*   @param limit  The number of bytes to read
*
*   @return 0 on success, -1 on error
*/
int ssl_read_bytes(SSL *my_ssl,void *buf,unsigned int limit)
{
    byte_t *my_buf = NULL;                          // Pointer to the buffer we want to write to
    unsigned int x = 0;                             // Counter for iteration over the buffer
    
    my_buf = (byte_t *)buf;                         // Point our pointer at the buffer provided in the arg list
    
    for(;x<limit;x++) {                             // Walk the entire buffer
        my_buf[x] = ssl_read_byte(my_ssl);          //  read a byte at a time
    }

    return 0;                                       //   and 0 on success
}

/**
* Writes a single byte on the SSL connection.
*
*	@param my_ssl The ssl connection to use
*	@param this_byte The byte to write
*/
void ssl_write_byte(SSL *my_ssl,byte_t this_byte)
{
    SSL_write(my_ssl,&this_byte,1);
}

/**
* Write a sequence of bytes on the SSL connection.
* 
* @param my_ssl The ssl connection to use
* @param message The byte_t array to send
* @param length The number of bytes to send
*/
void ssl_write_bytes(SSL *my_ssl, void *message, unsigned int length)
{
    int ret_val = 0, bytes_written = 0;             // Counters for our loop
    byte_t *buffer = NULL;                          // Our send buffer
    
    buffer = (byte_t *)message;                     // convert to byte_t array
    
    while(bytes_written < length) {                 // While we still have bytes to write
        ret_val = SSL_write(my_ssl,                 // Write as many as SSL_write can
                            buffer + bytes_written, 
                            length - bytes_written);

        if(ret_val <= 0)                            // If an error is encountered
            break;                                  //  simply stop writing
        else                                        // Otherwise
            bytes_written += ret_val;               //  move up in the buffer for the next iteration
    }
}

/**
* Get the remote clients IP address as a string.  The result of 
*  this call is stored in a static buffer that will be overwritten
*  next time this function is called.  If you need to retain the 
*  returned value be sure you copy it out.  Note this means that this
*  function is NOT thread safe.
*
*	@param my_ssl The SSL connection to get the clients file descriptor from
*	@return A const char pointer to the clients IP address as a \0 terminated string
*/
const char *network_get_ip_address(SSL *my_ssl)  
{
    struct sockaddr_in addr;            // The sockadd_in structure for gathering our client data
    int sizeof_addr = 0;                // Holder for the number of bytes used by the addr structure
    int clientFd = 0;                   // The clients file descriptor

    clientFd = SSL_get_fd(my_ssl);      // Retrieve the file descriptor for the current connection
    sizeof_addr = sizeof(addr);         // Setup for our getpeername call
    getpeername(clientFd, (struct sockaddr *) &addr, &sizeof_addr); // Fill in the addr structure

    return(const char *) inet_ntoa(addr.sin_addr);  // Retrieve the IP as a string from the addr structure
}

/**
* Create a new RSA key for PKI signing and verification.
* All keys are created with 2048 bit public modulus and 
* using RSA_F4 exponent (65537). All keys generated should
* be destroyed with key_destroy_key().
*	
*	@return an RSA * to the RSA key or NULL on error
*/
RSA * key_create_key(void)
{
    RSA *new_key = NULL;                // Setup storage for the new key

    new_key = RSA_generate_key(2048,RSA_F4,NULL,NULL);  // Generate the key

    if(new_key) {                       // Verify the key
        if(RSA_check_key(new_key) == 1) // Test
            return new_key;             // Return if valid
        else
            return NULL;                // Or return NULL on error
    }

    return NULL;                        // If we get here then the create failed, return NULL
}

/**
* Destroy an RSA key object.
* @param this_key A pointer to the RSA object to destroy
*/
void key_destroy_key(RSA *this_key)
{
    if(this_key)                        // Make sure the key exists first
        RSA_free(this_key);             // Then free its resources
}

/** 
* Determine the size a buffer must be to store data (signed).
* @param this_key The RSA key that will be used to sign data
* @return Size of the necessary buffer in bytes
*/
unsigned int key_buffer_size(RSA *this_key)
{
    return RSA_size(this_key);          // We just wrap the RSA_size() function
}

/**
* Sign a piece of data with the given key.  
*
*	@param this_key The RSA Key to sign with
*	@param original The original data to use
*	@param orig_size The size in bytes of the original data 
*	@param signd The buffer for signed data (at least key_buffer_size() bytes large)
*	@param signd_length The length of the signd buffer in bytes
*
*   @return The number of signed bytes in the passed buffer (0 on error)
*/
unsigned int key_sign_data(RSA *this_key,const char *original,unsigned int orig_size,char *signd,unsigned int signd_length)
{
    EVP_MD_CTX my_evp;                  // Our EVP context for signing
    EVP_PKEY * pkey;                    // Our EVP key which encapsulates the RSA key
    unsigned int signed_length = 0;     // The final signed buffer size

    if(this_key == NULL)                // If the key is not valid, don't bother signing
        return 0;                       //  just return an error

    pkey = EVP_PKEY_new();              // Create space for our PKEY 
    EVP_PKEY_set1_RSA(pkey,this_key);   // Associate our RSA key with the PKEY
    signed_length = signd_length;       // We need to tell SignFinal() how many bytes in the original and it will return
                                        //   the number of signed bytes, so we save this value out to a locally scoped var

    EVP_SignInit(&my_evp, EVP_md5());   // Begin the signing process, we will use md5() for the hash
    EVP_SignUpdate (&my_evp,original, orig_size);   // We only have the one buffer, add that
    EVP_SignFinal (&my_evp,signd, &signed_length,pkey); // Finalize the signing process
    EVP_PKEY_free(pkey);                // Free up the uneeded space

    return signed_length;               // And return the total number of signed bytes
}

/** 
 * Write a private key to a file.
 * 
 * @param this_key the RSA key to write 
 * @param filename the filename to write to
 * @return 0 on success, -1 on error.
 */
int key_write_priv(RSA *this_key, char *filename) 
{
    FILE *store_file = NULL;            // The file handle we will give to PEM_write_RSAPrivateKey
    int retval = 0;

    store_file = fopen(filename,"w");   // Open the file for writing

    if(!store_file)                     // Make sure the open succeeded
        return -1;                      //  return an error if it didn't

    // Write the private key with no encryption and no password
    retval = PEM_write_RSAPrivateKey(store_file,this_key,NULL,NULL,0,NULL,NULL);

    fclose(store_file);                 // Close the file

    if(retval)                          // If PEM_write_RSAPrivateKey() returned a non-zero value it was successful
        return 0;                       //  and we return 0 for success
    else
        return -1;
}

/** 
 * Read a private key from a file.
 * 
 * @param filename the filename to read from
 * @return a pointer to the key, or NULL on error.
 */
RSA *key_read_priv(char *filename)
{
    FILE *store_file = NULL;            // The file handle we will use
    RSA *this_key = NULL;               // The key we hope to retrieve
    store_file = fopen(filename,"r");   // Open the file for reading

    if(!store_file)                     // Verify the open worked
        return NULL;                    //  return NULL if it didn't

    // Read the key from the file
    this_key = PEM_read_RSAPrivateKey(store_file,NULL,NULL,NULL);

    fclose(store_file);                 // Close the file
    return this_key;                    // Return the key (a null from OpenSSL gets passed up)
}

/**
* Write a public key to the network.  This is recv'd on the other
* end with key_net_read_pub().
*   @param this_key The key to send
*   @param my_ssl The ssl connection to send it over
*/
void key_net_write_pub(RSA *this_key,SSL *my_ssl)
{
    unsigned int buf_size;                      // The number of bytes necessary to store  a DER encoded public key
    unsigned char *buf,*next;                   // Pointers to a buffer we will use for DER encoding
    buf_size = i2d_RSAPublicKey(this_key,NULL); // Get the number of bytes needed
    ssl_write_uint(my_ssl,buf_size);            // Tell the other end of the connection how many bytes to expect
    buf = next = (unsigned char *)w_malloc(buf_size); // Allocate space for the DER encoding
    i2d_RSAPublicKey(this_key,&next);           // Encode the key
    ssl_write_bytes(my_ssl,buf,buf_size);       // Write the encoded key over the network
    w_free(buf);                                // and finally free our buffer
}

/**
* Read a public key written by key_net_write_pub().
*   @param my_ssl The ssl connection to read the key from
*   @param len The length in bytes to expect
*/
RSA *key_net_read_pub(SSL *my_ssl)
{
    RSA *this_key = NULL;                       // The key we hope to recieve
    unsigned int len = 0;                       // The number of bytes we should expect
    unsigned char *temp = NULL,*buff;           // The buffer to hold the DER encoded key

    len = ssl_read_uint(my_ssl);                // First find out how many bytes in the encoded key
    buff = temp = (unsigned char *)w_malloc(len); // Create a buffer for it
    ssl_read_bytes(my_ssl,temp,len);            // Read the encoded key
    this_key = d2i_RSAPublicKey(NULL,&temp,len);// Decode the key 
    w_free(buff);                               // Free our buffer
    return this_key;                            // and return the key
}

/**
* Verify signed data using a public key.
*
*	@param this_key The RSA Key to verify with
*	@param signd The signed data
*	@param s_length Number of bytes to consider in signd
*	@param u_signed The data to verify against
*	@param u_length Length of u_signed in bytes
* 
*   @return 0 on success, -1 on failure
*/
int key_verify_signature(RSA *this_key, char *signd,unsigned int s_length,char * u_signed,unsigned int u_length)
{
    EVP_MD_CTX my_evp;                          // The signing context
    EVP_PKEY * pkey;                            // Our EVP encapsulation key
    int retval = 0;                             // Track return value

    if(this_key == NULL)                        // Make sure the key is valid
        return -1;                              //  Return an error if it isn't

    pkey = EVP_PKEY_new();                      // Create space for our evp key
    EVP_PKEY_set1_RSA(pkey,this_key);           // Associate our RSA key with our EVP key
    EVP_VerifyInit(&my_evp,EVP_md5());          // Begin the verification process, remember we used md5 for our hash
    EVP_VerifyUpdate(&my_evp,u_signed,u_length);// Add the known data (unsigned)
    retval = EVP_VerifyFinal (&my_evp,signd,s_length,pkey); // Finalize and verify

    EVP_PKEY_free(pkey);                        // Free our PKEY
    
    if(retval)                                  // If retval is nonzero then the signature was verified
        return 0;                               //  return 0 for success
    else                                        // Else the signature was incorrect
        return -1;                              //  return -1 for failure
}

/** Write a public key to a file.
 * 
 * @param filename the filename to write to
 * @return 0 on success, -1 on error
 */
int key_write_pub(RSA *this_key, char *filename )
{
    FILE *store_file = NULL;                    // The file handle we will use
    int retval = 0;                             // Track the return value
    
    store_file = fopen(filename,"w");           // open the file for writing

    if(!store_file)                             // Verify the open worked
        return -1;                              // Fail if it didn't

    retval = PEM_write_RSA_PUBKEY(store_file,this_key); // Write the key to the file

    fclose(store_file);                         // Close the file

    if(retval)                                  // If retval is nonzero, then writing succeeded
        return 0;                               //  return 0
    else                                        // Otherwise it failed
        return -1;                              //  return -1
}

/** Read a public key from a file.
 * 
 * @param filename the filename to read from
 * @return a pointer to the key, or NULL on error.
 */
RSA *key_read_pub(char *filename) 
{
    FILE *store_file = NULL;                    // The file handle we will use
    RSA *this_key = NULL;                       // The key we hope to retrieve

    store_file = fopen(filename,"r");           // Open the file for reading

    if(!store_file)                             // Make sure the open worked
        return NULL;                            //  fail if it didnt

    this_key = PEM_read_RSA_PUBKEY(store_file,NULL,NULL,NULL); // Read from the file
    
    fclose(store_file);                         // Close the file

    return this_key;                            // Return the key
}
