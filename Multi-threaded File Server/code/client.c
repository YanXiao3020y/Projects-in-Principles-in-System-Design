/*
gcc client.c -o client && ./client circinus-27.ics.uci.edu 50000
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //for strncopy & strcmp
#include <unistd.h> //for function close
#include <time.h> //for type time & its functions
#include <stdbool.h> //for type bool
#include <ctype.h> //for isdigit function

#define MAXLINE 512 //max length of input
#define MAXARGS 80 //max number of argc -- MaxArgc

int clientfd;
char *argv[MAXARGS]; //for user input
int argc; //the actual size of argv
int invalid = false;

int parseline(char* buf); 
int open_clientfd(char *hostname, char *port);
void print_hash(unsigned char *digest);

int main(int argc0, char **argv0)
{
    char *host, *port;
    // rio_t rio;
    
    host = argv0[1];
    port = argv0[2];
    
    clientfd = open_clientfd(host, port);
    
    while(1)
    {
        char buf[MAXLINE];
        fputs("> ", stdout);
        fgets(buf, MAXLINE, stdin);
        if (feof(stdin))
                exit(0);
        
        char cmdline[MAXARGS];
        strcpy(cmdline, buf);       
        parseline(cmdline);

        if (argv[0] == NULL)
            continue;   /* Ignore empty lines */

        //send message to server
        write(clientfd, buf, strlen(buf)); 

        if (!strcmp(argv[0], "quit")) //since server need to receive the quit command
            break;
        

        //receive message from server
        read(clientfd, buf, MAXLINE);

        //print server message if needed
        if (strlen(buf) > 0) 
        {
            if (!strcmp(argv[0], "getHash"))
            {
                char *ptr = buf; // Byte 0: -1; Bytes 1 – n: Characters of the string
                char negativeOne = -1 + '0';
                
                if(buf[0] == negativeOne) //check whether it is error message or getHash 
                {
                    //format message after recieve
                    ptr++;
                    strncpy(buf, ptr, MAXLINE);
                    printf("%s\n", buf); 
                }
                else
                    print_hash(buf);

            }
            else
                printf("%s\n", buf); 
        }
            
    }
    
    close(clientfd);
    exit(0);
}

int parseline(char* buf)
{
    //change '\t' to ' '
    int i=0;
    for(; i<strlen(buf); i++)
        if(buf[i] == '\t')
            buf[i] = ' ';
    // printf("%s", buf);

    char *delim;
    int bg;
    buf[strlen(buf)-1] = ' ';
    while(*buf && (*buf == ' '))
        buf++;
        
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' '))
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)
         return 1;
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    // for(int i = 0; i<argc; i++)
    // {
    //     printf("argv[%d]=%s\n", i, argv[i]);
    // }
    // printf("\nargc: %i\n\n\n", argc);

    return bg;
}

int open_clientfd(char *hostname, char *port) 
{ 
    int clientfd;
    struct addrinfo hints, *listp, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; /* Open a connection */ 
    hints.ai_flags = AI_NUMERICSERV; /* ...using numeric port arg. */ 
    hints.ai_flags |= AI_ADDRCONFIG; /* Recommended for connections */ 
    getaddrinfo(hostname, port, &hints, &listp);

    /* Walk the list for one that we can successfully connect to */
 
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
        break; /* Success */

        close(clientfd); /* Connect failed, try another */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else /* The last connect succeeded */
        return clientfd;
}

void print_hash(unsigned char *digest)
{
    unsigned char * temp = digest;
    printf("Hash: ");
    while(*temp)
    {
        printf("%02x", *temp);
        temp++;
    }
    printf("\n");
}