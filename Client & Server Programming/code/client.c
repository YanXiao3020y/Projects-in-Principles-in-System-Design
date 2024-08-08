/*
gcc client.c -o client && ./client circinus-5.ics.uci.edu 40000
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

#define MAXLINE 256 //max length of input
#define MAXARGS 80 //max number of argc -- MaxArgc

int clientfd;
char *argv[MAXARGS]; //for user input
int argc; //the actual size of argv
int invalid = false;

int parseline(char* buf); 
int open_clientfd(char *hostname, char *port) ;

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

        if (!strcmp(argv[0], "quit"))
            break;

        //format message before sending
        char message[MAXLINE]; // Byte 0: Length of the string (n); Bytes 1 – n: Characters of the string
        char messageSize = strlen(buf) + '0';
        message[0] = messageSize;
        message[1] = '\0';
        strncat(message, buf, MAXLINE);

        //send message to server
        write(clientfd, message, strlen(message)); 

        //receive message from server
        read(clientfd, buf, MAXLINE); //clientfd with buf -- let buf get message from server
        // printf("buf: %s\n", buf);

        //format message before printing
        char *ptr = buf;
        ptr++;
        strncpy(message, ptr, MAXLINE);
       
        // printf("message: %s\n", message);

        if(!strcmp(message,"-1\0")) 
        {
           printf("Unknown\n");
           continue; 
        }
        else if(!strcmp(message,"1\0"))
        {
           printf("Invalid syntax\n");
           continue; 
        }
        
        //print message message
        fputs(message, stdout);
        fputs("\n", stdout);
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

//char message[256]; "3Iam"
//sprintf(message, "%c%s", 3, "Iam");

//write/read to communcate with server
//write
 //         write(clientfd, message, strlen(buf)); 
 //         read(clientfd, message, MAXLINE);
  //        fputs(meassage, stdout); //receive from server

//           read(clientfd, message, 1); //3
//           read(clientfd, message, n); //Iam. n = len(message)
//size = buf[0] 
//message = buf[1:n]

