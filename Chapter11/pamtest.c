/**
    Chapter 11 - PAM Authentication - PAM Aware Application Programming
    By Nathan Yocom, nate@yocom.org
*/

#include <security/pam_appl.h>
#include <stdio.h>

int my_conv(int, const struct pam_message **, struct pam_response **, void *);

static struct pam_conv conv = {
    my_conv,
    NULL
};

int my_conv(int num_msg,const struct pam_message **msg, struct pam_response **response, void *appdata_ptr) {
    struct pam_response *reply_with = NULL; 
    int num_replies; 
    char buffer[80];

    if( num_msg <= 0 )
        return PAM_CONV_ERR;

    reply_with = (struct pam_response *)calloc(num_msg, sizeof(struct pam_response)); 

    if( reply_with == NULL )
        return PAM_SYSTEM_ERR;

    memset(buffer,'\0',sizeof(buffer));

    for( num_replies = 0; num_replies < num_msg; num_replies++ ) {
        if( msg[num_replies]->msg_style == PAM_PROMPT_ECHO_OFF ) {
            reply_with[num_replies].resp_retcode = PAM_SUCCESS; 
            printf("What is your password? ");
            scanf("%s",buffer);
            reply_with[num_replies].resp = (char *)strdup(buffer); 
        } else if( msg[num_replies]->msg_style == PAM_PROMPT_ECHO_ON ) {
            reply_with[num_replies].resp_retcode = PAM_SUCCESS; 
            printf("What is your username? ");
            scanf("%s",buffer);
            reply_with[num_replies].resp = (char *)strdup(buffer); 
        } else {
            free(reply_with); 
            return PAM_CONV_ERR; 
        }
    } 

    *response = reply_with; 
    return PAM_SUCCESS; 
}

int main(int argc, char *argv[]) {
    pam_handle_t *pamh=NULL;
    int ret;
    int authenticated = 0;

    if( (ret = pam_start("login", NULL, &conv, &pamh)) == PAM_SUCCESS )
        if( (ret = pam_authenticate(pamh, 0)) == PAM_SUCCESS )
            if( (ret = pam_acct_mgmt(pamh, 0)) == PAM_SUCCESS )
                authenticated = 1;
                
    if(authenticated)            
        printf("Authenticated\n");
    else
        printf("Not Authenticated\n");

    if( pam_end(pamh,ret) != PAM_SUCCESS ) {
        pamh = NULL;
        printf("Failed to release pam...\n");
        exit(1);
    }

    if( ret == PAM_SUCCESS )
        return 0;
    else
        return 1;
}


