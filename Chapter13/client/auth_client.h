/**
 * Authentication Client - For testing of the Authentication Server
 * By Nathan Yocom, 2004 - For APress Book "The Definitive Guide to Linux Network Programming"
 * 
 * auth_client.h = Maine client header file
 */
 
#ifndef AUTH_CLIENT_H
#define AUTH_CLIENT_H

#include <termios.h>    // Included to prevent echoing of password

// Connect via TLSv1 to the given host on the given port
SSL * ssl_client_connect(const char *host, const char *port);
// Get the current users username
const char *getUsername(void);
// Get the given users home directory path
const char *getUsersHome(const char *username);
// Check the users home directory for an existing host key file
int haveServerKey(const char *host,const char *username);
// Get a users key
RSA *getServerKey(const char *host,const char *username);
// Write the given private key to the users home directory
void writePrivKey(const char *host, const char *username, RSA *my_key);
// Prompt the user for their password
const char *getUserPassword(void);

#endif   
