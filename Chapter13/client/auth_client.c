/**
 * Authentication Client - For testing of the Authentication Server
 * By Nathan Yocom, 2004 - For APress Book "The Definitive Guide to Linux Network Programming"
 * 
 * auth_client.c = Main client source
 */
 
#include "common.h"
#include "auth_client.h"

/** 
 * Attempt to connect with TLSv1 to the given host on the given port and return an SSL * handle to 
 *  the resulting connection, or NULL on error.  If an error occurs, ERR_get_error() can be used 
 *  in conjunction with ERR_error_string() to examine the error in the calling function.
 * 
 * @param host A string indicating the hostname or IP to connect to
 * @param port A string indicating the port to connect to the host on
 * 
 * @return An SSL * handle to an active TLSv1 connection, or NULL on error
 */
SSL * ssl_client_connect(const char *host, const char *port) {
    SSL_METHOD *my_ssl_method;          // The method for connection, we use TLSv1
    SSL_CTX *my_ssl_ctx;                // Our context
    SSL *my_ssl;                        // The SSL pointer we will return
    BIO *my_bio;                        // The BIO used to setup the connection
    char *host_port;                    // A buffer to store the concatenated host:port string

    host_port = w_malloc(strlen(host) + strlen(port) + 2); // Allocate room for "host:port"
    sprintf(host_port,"%s:%s",host,port);                  // And store it formatted 

    my_ssl_method = TLSv1_client_method(); // Set our method to TLSv1    

    if((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) {
        return NULL;                    // If we can't create a context, return NULL
    }

    if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
        SSL_CTX_free(my_ssl_ctx);       // If we had problems, free the memory we already setup
        return NULL;                    //  and return NULL  
    }

    if((my_bio = BIO_new_connect(host_port)) == NULL) {
        SSL_free(my_ssl);               // If we had problems, free the memory we already setup 
        w_free(host_port);          
        return NULL;                    //  and return NULL
    }
    
    if(BIO_do_connect(my_bio) <=0) {
        SSL_free(my_ssl);               // If we had problems, free the memory we already setup 
        BIO_free(my_bio);
        w_free(host_port);          
        return NULL;                    //  and return NULL
    }

    SSL_set_bio(my_ssl,my_bio,my_bio);  // Associate our BIO with the SSL handle as both the read and write pipes

    if(SSL_connect(my_ssl) <= 0) {
        SSL_free(my_ssl);               // If we had problems, free the memory we already setup 
        w_free(host_port);          
        return NULL;                    //   and return NULL
    }

    w_free(host_port);                  // Free the space we setup for the host:port string
    return my_ssl;                      // Return the actual connection (all is well)
}

/** 
 * This function iterates through the password file using getpwent() looking for a username
 *  with the same uid as the userid our process has.  This way we can lookup our username without
 *  requiring user input.  The result is stored in a static buffer which will be destroyed and overwritten
 *  with subsequent calls.
 * 
 * @return A string representation of the current username or NULL if not found
 */
const char *getUsername(void) {
    static char *username_buffer = NULL;
    uid_t my_uid;
    struct passwd *current_pwent = NULL;

    my_uid = getuid();                  // Get our UID
    current_pwent = getpwent();         // Get the first passwd file entry

    if(username_buffer != NULL)  {      // Clear the username buffer if we have been called before
        w_free(username_buffer);
        username_buffer = NULL;
    }

    while(current_pwent && !username_buffer) {  // Loop through each entry in the file until we have found or hit EOF
        if(current_pwent->pw_uid == my_uid) {   // if the UID of this entry matches ours, its our entry
            username_buffer = (char *)w_malloc(strlen(current_pwent->pw_name) + 1); // Allocate room for the username
            strncpy(username_buffer,current_pwent->pw_name,strlen(current_pwent->pw_name) + 1); // Copy the username in
        }
        current_pwent = getpwent();         // Get the next passwd file entry
    }

    endpwent();                         // Make sure we close our access to the pwents
    return username_buffer;
}

/**
 * Similar to getUsername, this function iterates through with getpwent looking for our user's home directory
 *   and returns that directory in a static char buffer that will be overwritten with subsequent calls.
 * 
 * @param username The username to look for a home dir value
 * @return The users home directory as a null terminated string, or NULL of not found
 */
const char *getUsersHome(const char *username) {
    static char *home_buffer = NULL;
    struct passwd *current_pwent = NULL;

    current_pwent = getpwent();         // Get the first passwd file entry
    
    if(home_buffer != NULL)  {      // Clear the username buffer if we have been called before
        w_free(home_buffer);
        home_buffer = NULL;
    }

    while(current_pwent) {  // Loop through each entry in the file until we have found or hit EOF
        if(strcasecmp(username,current_pwent->pw_name) == 0) {   // if the usernames match
            home_buffer = (char *)w_malloc(strlen(current_pwent->pw_dir) + 1); // Allocate room for the username
            strncpy(home_buffer,current_pwent->pw_dir,strlen(current_pwent->pw_dir) + 1); // Copy the username in
        }
        current_pwent = getpwent();         // Get the next passwd file entry
    }

    endpwent();                     // make sure we close our access to the pwents
    return home_buffer;             // Return the result
}

/** 
 * Check the users home directory for a key file that is named .[host].priv which will contain
 *   the users private RSA key for authentication with the given host.
 * 
 * @param host Hostname to look for
 * @param username The username we are running as
 * @return 0 on success, -1 on failure
 */
int haveServerKey(const char *host,const char *username) {
    char *file_path = NULL;                             // Buffer where we will build the path
    const char *user_home = NULL;                       // The user's home directory
    FILE *key_file = NULL;                              // File pointer used to check for file existence

    if((user_home = getUsersHome(username)) == NULL) {
        report_error_q("Unable to find user's home directory",__FILE__,__LINE__,0);
    }

    file_path = (char *)w_malloc(strlen(host) + strlen(user_home) + 15); // Allocate space for the full file path
    
    strncpy(file_path,user_home,strlen(user_home));     // Copy the home directory path in
    strncat(file_path,"/.",2);                          // Concatenate a /. to ensure full path
    strncat(file_path,host,strlen(host));               // Add the hostname
    strncat(file_path,".priv",strlen(".priv"));         // Add a .priv extension

    if((key_file = fopen(file_path,"r")) == NULL) {     // If opening fails
        w_free(file_path);                              //   free the memory we used
        return -1;                                      //   we dont have the file, return -1
    }
    else {                                              // Otherwise
        fclose(key_file);                               //   close the now open file
        w_free(file_path);                              //   free the memory we used
        return 0;                                       //   and return success
    }
}

RSA *getServerKey(const char *host, const char *username) {
    char *file_path = NULL;                             // Buffer where we will build the path
    const char *user_home = NULL;                       // The user's home directory
    RSA *my_key = NULL;

    if((user_home = getUsersHome(username)) == NULL) {
        report_error_q("Unable to find user's home directory",__FILE__,__LINE__,0);
    }

    file_path = (char *)w_malloc(strlen(host) + strlen(user_home) + 15); // Allocate space for the full file path
    
    strncpy(file_path,user_home,strlen(user_home));     // Copy the home directory path in
    strncat(file_path,"/.",2);                          // Concatenate a /. to ensure full path
    strncat(file_path,host,strlen(host));               // Add the hostname
    strncat(file_path,".priv",strlen(".priv"));         // Add a .priv extension
    my_key = key_read_priv(file_path);              //   read the key in
    w_free(file_path);
    return my_key;
}

/** 
 * Write the private portion of an RSA key to the file .[host].priv.
 */
void writePrivKey(const char *host, const char *username, RSA *my_key) {
    char *file_path = NULL;                             // Buffer where we will build the path
    const char *user_home = NULL;                       // The user's home directory
    
    if((user_home = getUsersHome(username)) == NULL) {
        report_error_q("Unable to find user's home directory",__FILE__,__LINE__,0);
    }

    file_path = (char *)w_malloc(strlen(host) + strlen(user_home) + 15); // Allocate space for the full file path
    
    strncpy(file_path,user_home,strlen(user_home));     // Copy the home directory path in
    strncat(file_path,"/.",2);                          // Concatenate a /. to ensure full path
    strncat(file_path,host,strlen(host));               // Add the hostname
    strncat(file_path,".priv",strlen(".priv"));         // Add a .priv extension

    if(key_write_priv(my_key,file_path) != 0) {         // Write the key
        report_error_q("Unable to write private key to file",__FILE__,__LINE__,0);
    }
}

/**
 * Prompt the user for a password and return it in a static buffer that will be overwritten with subsequent calls
 */
const char *getUserPassword(void) {
   struct termios terminal_setup, old_terminal_setup;
   static char *password_buffer[2048];
   char *newline = NULL;

   memset(password_buffer,'\0',2048);

   tcgetattr(STDIN_FILENO, &terminal_setup);            // Retrieve the current terminal settings
   old_terminal_setup = terminal_setup;                 // Save the current setup so we don't clobber it

   terminal_setup.c_lflag &= ~ECHO;                     // Turn echoing flag off
   terminal_setup.c_lflag |= ECHONL;
   tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_setup); // Apply the changed settings

   printf("Password: ");                                // Prompt for the password
   fgets((char *)password_buffer, 2048, stdin);                 // Read the password
   tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_setup);  // Reset the old attributes

   // We need to truncate the last \n that may have been inserted by fgets
   while((newline = strstr((char *)password_buffer,"\n")) != NULL) {
       *newline = '\0';
   }

   return (char *)password_buffer;
}


int main(int argc, char *argv[]) {
    SSL *ssl_connection = NULL;                                 // Pointer for our SSL connection
    const char *host = NULL, *port = NULL;                      // Pointers to the hostname and port number on the cmd line
    const char *username = NULL;                                // Username store
    char *response = NULL;                                      // Server responses
    char *signed_data_buffer = NULL;                            // Buffer and count for our signed data
    unsigned int signed_data_buffer_size = 0;
    RSA *my_rsa_key = NULL;                                     // Our RSA key

    if(argc != 3) {
        fprintf(stderr, "Usage: %s host port\n",argv[0]);       // We should report the problem in a nicer way than report_error
        exit(EXIT_FAILURE);                                     // Exit with an error
    }

    //signal(SIG_PIP,SIG_IGN);

    w_memory_init();                                            // Initialize our memory wrappers state
    openssl_init();                                             // Initialize the OpenSSL library
    
    host = argv[1];                                             // Hostname is the first argument
    port = argv[2];                                             // Port is the second argument
    username = getUsername();                                   // Get our username

    if(username == NULL) {
        report_error_q("Unable to determine the username of this process.",__FILE__,__LINE__,0);
    }

    if((ssl_connection = ssl_client_connect(host,port)) == NULL) { // Attempt a connection using our wrapper
        report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
    }
    
    if(haveServerKey(host,username) == 0) {          // First we look to see whether we have a key already
        ssl_write_uint(ssl_connection,REQUEST_KEY_AUTH);        // Tell the server we want to use PKI authentication
        ssl_write_string(ssl_connection,username);              // Then we send the username

        my_rsa_key = getServerKey(host,username);
        if(my_rsa_key == NULL) {                                // Validate the key that was read in
            report_error_q("Key file exists, but data is invalid",__FILE__,__LINE__,0);
        }
        
        signed_data_buffer = (char *)w_malloc(key_buffer_size(my_rsa_key)); // Allocate memory for the signed data and then sign it
        signed_data_buffer_size = key_sign_data(my_rsa_key,username,strlen(username),signed_data_buffer,key_buffer_size(my_rsa_key));
        ssl_write_uint(ssl_connection,signed_data_buffer_size); // Tell the server how much data to expect
        ssl_write_bytes(ssl_connection,signed_data_buffer,signed_data_buffer_size); // And send the data

        if(ssl_read_uint(ssl_connection) == SERVER_AUTH_SUCCESS) {
            printf("Server responded with SERVER_AUTH_SUCCESS\n");
        } else {
            printf("Server responded with SERVER_AUTH_FAILURE\n");
        }
        w_free(response);
    } else {                                                    // We dont have a PKI key, so we will do password authentication
        ssl_write_uint(ssl_connection,REQUEST_PASS_AUTH);       // Tell the server we want to do password authentication
        ssl_write_string(ssl_connection,username);              // Send the username
        ssl_write_string(ssl_connection,getUserPassword());     // Send the user's password attempt

        if(ssl_read_uint(ssl_connection) == SERVER_AUTH_SUCCESS) {
            printf("Server responded with SERVER_AUTH_SUCCESS, setting up a PKI key for the future.\n");
            my_rsa_key = key_create_key();                      // Create a new RSA key
            if(!my_rsa_key) {                                   // Verify it was actually created
                report_error("Error creating RSA key.",__FILE__,__LINE__,0);
            }
            key_net_write_pub(my_rsa_key,ssl_connection);       // Send the public portion of the key to the server
            writePrivKey(host,username,my_rsa_key);             // Write the private portion to a file
        } else {
            printf("Server responded with SERVER_AUTH_FAILURE\n");
        }
    }

    SSL_shutdown(ssl_connection);
    SSL_free(ssl_connection);
    return 0;
}


