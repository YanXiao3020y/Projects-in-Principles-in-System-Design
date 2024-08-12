/*
cd ics53/hw6/hw6_o22
gcc server.c -o server && ./server 50000
*/

#include <unistd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h> 
#include "Md5.c"

#define MAXARGS 80 //max number of argc -- MaxArgc
#define MAXLINE 200
#define LISTENQ 1024 
#define MAXCLIENTS 4

struct ClientInfo 
{ 
    int id; 
    FILE* theFile; 
    char fileName[MAXLINE]; 
    char fileMode;	
    // fileMode:
    // r - open a file in read mode.
    // w - opens or create a text file in write mode.
    // a - opens a file in append mode.
    // r+ - opens a file in both read and write mode.
    // a+ - opens a file in both read and write mode.
    // w+ - opens a file in both read and write mode.
};
struct ClientInfo clients[MAXCLIENTS]; 

char argv[MAXARGS][MAXLINE]; //for user input
int argc; //the actual size of argv
int currentClientIndex;  //currentClientIndex = -1 ???*** when the clients is more than 4
static int byte_cnt; //Byte counter
static sem_t mutex; //the mutex that protects it


int open_listenfd(char *port);
void *thread(void *vargp);
static void initEval (void);
void initClientInfo();
void eval (int connfd);
void createAClient(int connfd);
void parseline(char* buf);
int built_in_commands(char *message);


int main(int argc, char** argv) 
{
    initClientInfo();

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = open_listenfd(argv[1]);
    printf("server started\n");

    while (1) 
    {
        clientlen=sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }

    return 0; 
}


int open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next) {
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(listenfd);
    }

    freeaddrinfo(listp);
    if (!p) return -1;
    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);
        return -1;
    }
    return listenfd;
}


void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    eval(connfd);
    close(connfd);
    return NULL;
}

static void initEval (void)
{
    sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void initClientInfo()
{
    for(int i=0; i<MAXCLIENTS; i++)
    {
        clients[i].id = -1;
        clients[i].theFile = NULL;
        strncpy(clients[i].fileName, "", MAXLINE);
        clients[i].fileMode = 'x';
    }

}

void eval (int connfd)
{
    int n;
    char in_msg[MAXLINE] = "\0"; 

    // make a thread for current client
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    pthread_once(&once, initEval);
    while( (n = read(connfd, in_msg, MAXLINE)) != 0 ) //recieve message from client
    {
        sem_wait(&mutex); //begin semaphore 

        //create a new client if the client does not exist & get its currentClientIndex
        createAClient(connfd);

        //???
        // printf("currentClientIndex: %i\n", currentClientIndex);
        // if(currentClientIndex == 1)
        // {
        //     printf("currentClientIndex == 1\n");
        //     out_msg = "A file is already open for reading";
        //     return;
        // }

        // char cmdline[MAXARGS];
        // strncpy(cmdline, in_msg, MAXARGS);   
        // printf("cmdline: %s\n", cmdline);
        // parseline(cmdline);
        // printf("cmdline: %s\n", cmdline);
        
        //parse the message to argv
        parseline(in_msg);
       
        char message[MAXLINE];
        built_in_commands(message);
        // printf("message: %s\n", message);

        //send message to client
        write(connfd, message, MAXLINE);

        //reset in_msg 
        strncpy(in_msg, "\0", MAXLINE);

        sem_post(&mutex); //end semaphore 
    }
}

void createAClient(int connfd) //create a new client if the client does not exist & get its currentClientIndex
{
    //find an existing client if any
    bool clientExist = false;
    for (int i = 0; i < MAXCLIENTS; i++) 
    {
        if (clients[i].id == connfd) 
        {
            currentClientIndex = i;
            clientExist = true;
            break;
        }
    }

    //if the clients does not exist, then create a new client
    if (!clientExist) {
        for (int i = 0; i < MAXCLIENTS; i++) 
        {
            if (clients[i].id == -1) 
            {
                currentClientIndex = i;
                clients[i].id = connfd;
                break;
            }
        }
    }
}

void parseline(char* buf)
{
    char cmd1[MAXLINE] = "\0";     
    char cmd2[MAXLINE] = "\0"; 
    sscanf(buf, "%s %s", cmd1, cmd2);

    strcpy(argv[0], cmd1);
    strcpy(argv[1], cmd2);
}
// int parseline(char* buf)
// {
//     //change '\t' to ' '
//     for(int i=0; i<strlen(buf); i++)
//         if(buf[i] == '\t')
//             buf[i] = ' ';
//     printf("\n---------\nbuf: %s\n", buf);

//     char *delim;
//     int bg;
//     buf[strlen(buf)-1] = ' ';
//     while(*buf && (*buf == ' '))
//         buf++;
        
//     argc = 0;
//     while ((delim = strchr(buf, ' '))) {
//         argv[argc++] = buf;
//         *delim = '\0';
//         buf = delim + 1;
//         while (*buf && (*buf == ' '))
//             buf++;
//     }
//     argv[argc] = NULL;

//     if (argc == 0)
//          return 1;
//     if ((bg = (*argv[argc-1] == '&')) != 0)
//         argv[--argc] = NULL;

//     for(int i = 0; i<argc; i++)
//     {
//         printf("argv[%d]=%s\n", i, argv[i]);
//     }
//     printf("\nargc: %i\n\n=======\n", argc);

//     return bg;
// }

int built_in_commands(char *message)
{ 
    // printf("argv[0]: %s\n", argv[0]);
    // printf("argv[1]: %s\n", argv[1]);
    // printf("message: %s\n", message);

    //if command is not "quit", then print
    if (strcmp(argv[0], "quit") != 0) 
        printf("%s %s\n", argv[0], argv[1]); 
    strncpy(message, "\0", MAXLINE);

    // if (!strcmp(argv[0], "openRead"))
    if (!strcmp(argv[0], "openRead"))
    {
        // printf("openRead built_in_commands\n");
        // strncpy(message, "openRead\n\0", MAXLINE);
        
        
        if (clients[currentClientIndex].theFile == NULL) //if the client has no file open yet   
        {
            bool isOpen = false;
            //if the client is going to open a new file open, but if the file that is opened for appendeding currently by another client, then the error message
            for (int i = 0; i < MAXCLIENTS; i++) 
                if (clients[i].theFile != NULL 
                &&  i != currentClientIndex 
                && !strcmp(clients[i].fileName, argv[1]))
                {

                    if(clients[i].fileMode == 'a')
                    {
                        strncpy(message, "The file is open by another client.\0", MAXLINE);
                        // printf("%s\n", message);
                        isOpen = true;
                        return 1;
                    }
                    else if (clients[i].fileMode == 'r')
                    {
                        strncpy(message, "A file is already open.\0", MAXLINE);
                        // printf("%s\n", message);
                        isOpen = true;
                        return 1;
                    }
                }

            if (!isOpen) //if the client has no file open yet
            {
                clients[currentClientIndex].theFile = fopen(argv[1], "r"); // r = read mode
                if(clients[currentClientIndex].theFile == NULL)
                {
                    strncpy(message, "File not found\0", MAXLINE);
                    // printf("%s\n", message);
                }

                strcpy(clients[currentClientIndex].fileName, argv[1]);
                clients[currentClientIndex].fileMode = 'r';
            }
        }
        else // If the client attempts to open a file for reading while the same client has another file already open for reading, the error message
            {
                strncpy(message, "A file is already open for reading\0", MAXLINE);
                // printf("%s\n", message);
            }

        return 1;
    }
    // else if (!strcmp(argv[0], "openAppend"))
    else if (!strcmp(argv[0], "openAppend"))
    {
        // printf("openAppend built_in_commands\n");
        // strncpy(message, "openAppend\n\0", MAXLINE);

        if (clients[currentClientIndex].theFile == NULL)
        {
            //if the client has no file open yet, but if the file that is going to be appended is currently open for another client then the error message
            for (int i = 0; i < MAXCLIENTS; i++) 
            {
                if (i != currentClientIndex &&
                 clients[i].theFile != NULL &&
                 !strcmp(clients[i].fileName, argv[1])) 
                 {
                    strncpy(message, "The file is open by another client.\0", MAXLINE);
                    // printf("%s\n", message);
                    return 1;
                 }
            }

            clients[currentClientIndex].theFile = fopen(argv[1], "a"); //append 
            strcpy(clients[currentClientIndex].fileName, argv[1]);
            clients[currentClientIndex].fileMode = 'a';
        }
        else
        {
            strncpy(message, "A file is already open for appending\0", MAXLINE);
            // printf("%s\n", message);
        }

        return 1;
    }
    // else if (!strcmp(argv[0], "read"))
    else if (!strcmp(argv[0], "read"))
    {
        // printf("read built_in_commands\n");
        // strncpy(message, "read\n\0", MAXLINE);
        //strncpy(message, "\0", MAXLINE);

        if (clients[currentClientIndex].theFile != NULL) 
        {   
            char fileData[MAXLINE] = "\0";
            int i;
            int n = atoi(argv[1]);
            for(i=0; i<n; i++)
            {
                char ch = fgetc(clients[currentClientIndex].theFile);
                if (ch == EOF) //reach end of the file, stop
                    break;
                fileData[i] = ch;
            }
            fileData[i] = '\0';  
            strncpy(message, fileData, MAXLINE);
        }
        else
        {
            strncpy(message, "File not open\0", MAXLINE);
            // printf("%s\n", message);
        }
            


        return 1;
    }
    // else if (!strcmp(argv[0], "append"))
    else if (!strcmp(argv[0], "append"))
    {
        // printf("append built_in_commands\n");
        // strncpy(message, "append\n\0", MAXLINE);

        if (clients[currentClientIndex].theFile != NULL) 
            fprintf(clients[currentClientIndex].theFile, "%s", argv[1]);
        else
        {
            strncpy(message, "File not open\0", MAXLINE);
            // printf("%s\n", message);
        }

        return 1;
    }
    // else if (!strcmp(argv[0], "close"))
    else if (!strcmp(argv[0], "close"))
    {
        // printf("close built_in_commands\n");
        // strncpy(message, "close\n\0", MAXLINE);

        if (!strcmp(argv[1], clients[currentClientIndex].fileName))
        {
            //close file
            fclose(clients[currentClientIndex].theFile);

            //reset clientInfo except client ID 
            clients[currentClientIndex].theFile = NULL;
            strncpy(clients[currentClientIndex].fileName, "", MAXLINE);
            clients[currentClientIndex].fileMode = 'x';
        }

        return 1;
    }
    // else if (!strcmp(argv[0], "getHash"))
    else if (!strcmp(argv[0], "getHash"))
    {
        // printf("getHash built_in_commands\n");
        // strncpy(message, "getHash\n\0", MAXLINE);

        //if the client has no file open yet, but if the file that is going to be appended is currently open for another client then the error message
        for (int i = 0; i < MAXCLIENTS; i++) 
        {
            if (clients[i].theFile != NULL &&
                !strcmp(clients[i].fileName, argv[1]) &&
                clients[i].fileMode == 'a') 
                {
                    //format message before sending
                    char buf[MAXLINE]; // Byte 0: -1; Bytes 1 – n: Characters of the string
                    char negativeOne = -1 + '0';
                    buf[0] = negativeOne;
                    buf[1] = '\0';
                    // printf("buf: %s\n", buf);

                    strncpy(message, "A file is already open for appending.\0", MAXLINE);
                    strncat(buf, message, MAXLINE);
                    strncpy(message, buf, MAXLINE);
                    // printf("%s\n", message);


                    return 1;
                }
        }


        unsigned char digest[16]; //  a buffer to store the hash into
        MDFile(argv[1], digest); // function that calculates the hash of the file
        strncpy(message, digest, MAXLINE);
        

        return 1;
    }
    // else if (!strcmp(argv[0], "quit"))
    else if (!strcmp(argv[0], "quit"))
    {
        // printf("quit built_in_commands\n");
        // strncpy(message, "quit\n\0", MAXLINE);
        
        //close file
        if (clients[currentClientIndex].theFile != NULL) 
            fclose(clients[currentClientIndex].theFile);

        //reset clientInfo
        clients[currentClientIndex].id = -1;
        clients[currentClientIndex].theFile = NULL;
        strncpy(clients[currentClientIndex].fileName, "", MAXLINE);
        clients[currentClientIndex].fileMode = 'x';

        return 1;
    }



    return 0;
}

