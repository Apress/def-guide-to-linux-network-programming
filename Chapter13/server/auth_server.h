/**
 * Authentication Client - For testing of the Authentication Server
 * By Nathan Yocom, 2004 - For APress Book "The Definitive Guide to Linux Network Programming"
 * 
 * auth_client.h = Maine client header file
 */
 
#ifndef AUTH_SERVER_H
#define AUTH_SERVER_H

#include <security/pam_appl.h> // Include for PAM

// Setup/Get connections
SSL *get_connection(char *port);
// Authenticate a username/password via PAM
int pam_authenticate_user(const char *,const char *);
// Our PAM Conversation function
int auth_conv(int, const struct pam_message **, struct pam_response **, void *);
// The child processes 'main'
void child_process(SSL *my_ssl);
// The PAM conversation function
int auth_conv(int num_msg,const struct pam_message **msg, struct pam_response **response, void *appdata_ptr);

// The structure used to pass a username and password to pam
typedef struct auth_struct
{
  // Username to authenticate 
  const char *username;
  // Password to use
  const char *password;
} auth_struct;


#endif   
