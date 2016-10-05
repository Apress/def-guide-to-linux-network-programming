/**
 * Authentication Server - For testing of the Authentication Server
 * By Nathan Yocom, 2004 - For APress Book "The Definitive Guide to Linux Network Programming"
 * 
 * auth_server.c = Main server source
 */

#include "common.h"
#include "auth_server.h"

/**
* Authenticate a given username and password against the systems PAM interface.
* Null username and/or passwords will fail.  The pam service name in the cache
* as pam_service will be used if it exists, otherwise this will default to 'login'.
*
*	@param username The username to authenticate
*	@param password The password to authenticate
*	@return An integer value indicating success or failure, -1 is fail, 1 is success
*/
int pam_authenticate_user(const char *username,const char *password)
{
  struct auth_struct buffer;                    // Setup our custom structure to provide data through PAM
  static struct pam_conv myauthconv = {         // Setup the structure that will tell PAM about our Conversation func                       
    auth_conv,                                                     
    NULL
  };                                                                 
  pam_handle_t *pamh=NULL;                      // Our handle into PAM
  int ret = 0, authenticated = 0;               // Some status holders

  buffer.username = username;                   // Save a pointer to our username
  buffer.password = password;                   //   and password
  myauthconv.appdata_ptr = &buffer;             // Set the pointer to our structure, this way its available in the 
                                                //  callback process

  if(username && password)                      // Don't call into PAM if one or the other isn't set
  {
    authenticated = 
    (ret = pam_start("login", NULL, &myauthconv, &pamh)) == PAM_SUCCESS && // Connect with PAM on the "login" service
    (ret = pam_authenticate(pamh, 0)) == PAM_SUCCESS &&                    // Ensure that the account authenticates
    (ret = pam_acct_mgmt(pamh, 0)) == PAM_SUCCESS;                         // And that it is not expired or disabled

    pam_end(pamh,ret);                                                     // End our connection with PAM
  }

  if(authenticated)                                                        // Return the result
    return 1;   // Authenticated
  else
    return -1;  // Not
}

/**
* PAM Conversation function.  This is the callback entry for PAM to get the username
*	and password to authenticate.
*/
int auth_conv(int num_msg,const struct pam_message **msg, struct pam_response **response, void *appdata_ptr)
{
  struct pam_response *reply_with = NULL;               // We must set this up to pass back to PAM
  int num_replies; 
  struct auth_struct *user_data;
  user_data = (struct auth_struct *) appdata_ptr;       // This is our data, with username/password

  if(num_msg <= 0)
    return PAM_CONV_ERR;

  reply_with = (struct pam_response *)calloc(num_msg, sizeof(struct pam_response)); 

  if(reply_with == NULL)
    return PAM_SYSTEM_ERR;

  for(num_replies = 0; num_replies < num_msg; num_replies++)
  {
    if(msg[num_replies]->msg_style == PAM_PROMPT_ECHO_OFF)
    {
      reply_with[num_replies].resp_retcode = PAM_SUCCESS; 
      reply_with[num_replies].resp = strdup(user_data->password);       // Copy the password in
    }
    else if(msg[num_replies]->msg_style == PAM_PROMPT_ECHO_ON)
    {
      reply_with[num_replies].resp_retcode = PAM_SUCCESS; 
      reply_with[num_replies].resp = strdup(user_data->username);       // Copy the username in
    }
    else
    {
      free(reply_with); 
      return PAM_CONV_ERR; 
    }
  } 

  *response = reply_with; 
  return PAM_SUCCESS;                                                   // Tell PAM we are done
}

/** The first time this function is called it sets up a listening BIO
 *   on the given port.  Every following call returns the next incoming
 *   connectin, or blocks until one is available. When called with a NULL 
 *   argument the listening BIO is closed and resources freed.
 */
SSL *get_connection(char *port) {
    SSL *my_ssl = NULL;                         // The next connection
    static SSL_CTX *my_ssl_ctx = NULL;          // We use static here so we can use them on subsequent calls
    static SSL_METHOD *my_ssl_method = NULL;   
    static BIO *server_bio = NULL;
    BIO *client_bio = NULL;

    if (port && !server_bio) {                   // If the port is set, but we dont have a BIO
        my_ssl_method = TLSv1_server_method();  //  then we need to setup a new connection

        if ((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) { // Setup a context
            report_error_q("Unable to setup context.",__FILE__,__LINE__,0);
        }

        // We assume our certificate is called server.pem and is in the current dir
        SSL_CTX_use_certificate_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM); 
        // We assume our private key is called server.pem and is in the current dir
        SSL_CTX_use_PrivateKey_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);

        if (!SSL_CTX_check_private_key(my_ssl_ctx)) {    // Verify the certificate
            report_error_q("Private key does not match certificate",__FILE__,__LINE__,0);
        }

        // Setup for accepting and get our BIO
        if ((server_bio = BIO_new_accept(port)) == NULL) {
            report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
        }

        // Make sure the BIO is setup and in a state to get incoming connectins
        if (BIO_do_accept(server_bio) <= 0) {
            report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
        }
    }

    if (port == NULL) {              // If the port is NOT set, we should close things down
        SSL_CTX_free(my_ssl_ctx);
        BIO_free(server_bio);
    } else {                        // Otherwise we are already to accept new connections, just get the next one
        if (BIO_do_accept(server_bio) <= 0) {            // Get the next connection
            report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
        }

        client_bio = BIO_pop(server_bio);               // Pop it off the stack
        if ((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {    // Setup a new SSL pointer for it
            report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
        }

        SSL_set_bio(my_ssl,client_bio,client_bio);      // Set the bio from the stack as the read and write pipes

        if (SSL_accept(my_ssl) <= 0) {                   // Negotiate a connection with the client
            report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
        }
    }   

    return my_ssl;                  // This will be the next connection, or NULL depending on
                                    //   how we were called
}

/** 
 * This is called as the starting point for each new process, once we are here we have 
 *   a connection, and we just need to exit() with EXIT_SUCCESS when we are done.
 */
void child_process(SSL *my_ssl) {
    char *username = NULL, *password = NULL,*key_file = NULL;
    RSA *users_key = NULL;
    int authenticated = 0;
    int string_size = 0;
    unsigned int signed_size = 0;
    byte_t *signed_buffer = NULL;
    w_memory_init();                  // We need to initialize our memory allocation routines
    

    switch (ssl_read_uint(my_ssl)) {
    case SSL_ERROR:
        report_error_q(ERR_error_string(ERR_get_error(),NULL),__FILE__,__LINE__,0); // Report any problems and quit
        break;
    case REQUEST_KEY_AUTH:
        // Key Authentication
        username = ssl_read_string(my_ssl,1024);
        string_size = strlen(username) + strlen(network_get_ip_address(my_ssl)) + 10;
        key_file = w_malloc(string_size);
        snprintf(key_file,string_size,"%s.%s.pub",username,network_get_ip_address(my_ssl));
        users_key = key_read_pub(key_file);
        w_free(key_file);
        signed_size = ssl_read_uint(my_ssl);
        signed_buffer = (byte_t *)w_malloc(signed_size);
        if(ssl_read_bytes(my_ssl,signed_buffer,signed_size) != 0)
            report_error_q("Error reading signed data from client",__FILE__,__LINE__,0);

        if(key_verify_signature(users_key,signed_buffer,signed_size,username,strlen(username)) == 0) {
            ssl_write_uint(my_ssl,SERVER_AUTH_SUCCESS);
            printf("(%s) User %s authenticated via PKI\n",network_get_ip_address(my_ssl),username);
        } else {
            ssl_write_uint(my_ssl,SERVER_AUTH_FAILURE);
            printf("(%s) User %s failed via PKI\n",network_get_ip_address(my_ssl),username);
        }
        break;
    case REQUEST_PASS_AUTH:
        // Password authentication
        username = ssl_read_string(my_ssl,1024);
        password = ssl_read_string(my_ssl,1024);
        authenticated = pam_authenticate_user(username,password);
        printf("(%s) User %s %s via PAM\n",network_get_ip_address(my_ssl),username,authenticated ? "authenticated" : "failed");
        if(authenticated) {
            ssl_write_uint(my_ssl,SERVER_AUTH_SUCCESS);
            users_key = key_net_read_pub(my_ssl);
            string_size = strlen(username) + strlen(network_get_ip_address(my_ssl)) + 10;
            key_file = w_malloc(string_size);
            snprintf(key_file,string_size,"%s.%s.pub",username,network_get_ip_address(my_ssl));
            key_write_pub(users_key,key_file);
            w_free(key_file);
        } else {
            ssl_write_uint(my_ssl,SERVER_AUTH_FAILURE);
        }
        break;
    }

	if(users_key) {
		key_destroy_key(users_key);
	}

    SSL_shutdown(my_ssl);
    SSL_free(my_ssl);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    char *port = NULL;                                          // The port we should listen on
    SSL *my_ssl = NULL;
    int my_pid = 0;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n",argv[0]);            // We should report the problem in a nicer way than report_error
        exit(EXIT_FAILURE);                                     // Exit with an error
    }

    openssl_init();                                             // Initialize the OpenSSL library

    port = argv[1];                                             // Hostname is the first argument

    /*chdir("/etc/auth_server");                                // To have the server truly daemonize and chroot to /etc/auth_server,  
    chroot("/etc/auth_server");								   	//   uncomment these lines, and ensure the cert server.pem is in 
    daemon(0,0); */												//   /etc/auth_server before running.
    
    for (;;) {                                                   // This is our infinite server loop
        my_ssl = get_connection(port);                           // Get the next connection
        my_pid = fork();

        if (my_pid == 0) {                                       // If we are the child process
            child_process(my_ssl);                               //  then stub out
            daemon(0,0);                                         //  and daemonize ourselves
        } else {
            waitpid(my_pid,NULL,0);                              // Wait for our child to daemonize
        }
    }

    return 0;
}



