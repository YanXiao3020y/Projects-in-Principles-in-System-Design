/*
cd ics53/hw5/hw5_a
gcc server.c -o server && ./server MRNA.csv PFE.csv 40000
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

#define MAXLINE 256
#define MAXARGS 80 //max number of argc -- MaxArgc
#define LISTENQ 0 //backlog, set to 0 is fine from slide
#define STOCKSIZE 503 //the whole .csv has 504 lines, but the first line is not for data, so the size is 503

struct stock
{
    char date[MAXLINE];
    double close;
}; 

struct stock PFE_stocks[STOCKSIZE]; 
struct stock MRNA_stocks[STOCKSIZE]; 
char *argv[MAXARGS]; //for user input
int argc; //the actual size of argv
int invalid; //for sending signal to client to print "Invalid syntax"
char date[3][5]; //four digits for year + '\0' ==> 5 space total

void readData(char *fileName, struct stock *type);
int open_listenfd(char *port);
int parseline(char* buf); 
int built_in_commands(char *message);
bool searchDate(int *index, char *theDate);
void parseDate(char *theDate, const char *delims); //parsing date
bool searchDate(int *index, char *theDate); //true if found; otherwise, false.
double maxProfitFunction(int indexStart, int indexEnd, struct stock *typeStock);
double maxLossFunction(int indexStart, int indexEnd, struct stock *typeStock);
void parsingDate(char *theDate, int *day, int *month, int *year, const char *delims);
int invalidDate(char *theDate); //return 1 if date is invalid in term of format. return 0 if valid. return -1 if date is actual date but not within range of 7/2/2019 and 6/30/2021
bool isReverse(char *theDate1, char* theDate2); //return true if date reverse. Otherwise, return false. 

int main(int argc0, char *argv0[])
{
    //read from csv files
    readData("./PFE.csv", PFE_stocks);
    readData("./MRNA.csv", MRNA_stocks);

    //test print 
    // for(int i=0; i<STOCKSIZE; i++)
    //     printf("Data: %s Close: %f \n", PFE_stocks[i].date, PFE_stocks[i].close);
    // for(int i=0; i<STOCKSIZE; i++)
    //     printf("Data: %s Close: %f \n", MRNA_stocks[i].date, MRNA_stocks[i].close);

    //connection
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough room for any addr */ 
    char client_hostname[MAXLINE], client_port[MAXLINE];
    
    listenfd = open_listenfd(argv0[3]);
    while (1) 
    {
        clientlen = sizeof(struct sockaddr_storage); /* Important! */
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen); 
        getnameinfo((struct sockaddr *) &clientaddr, clientlen,client_hostname, MAXLINE, client_port, MAXLINE, 0);
        // printf("Connected to (%s, %s)\n", client_hostname, client_port);//delete ???
        printf("server started\n");

        char buf[MAXLINE];
        char message[MAXLINE];
        size_t n;

        //recieve message from client using read()
        while((n = read(connfd, buf, MAXLINE)) != 0)  //connfd, buf -- let buf get message from client
        {
            invalid = 0;

            //format message after receiving
            char *ptr = buf;
            ptr++;
            strncpy(message, ptr, MAXLINE);
            fputs(message, stdout);

            //parse and store each part to argv[]
            char cmdline[MAXARGS];
            strcpy(cmdline, message);       
            parseline(cmdline);

            int built_in_signal = built_in_commands(buf);
            if(built_in_signal == -1) //for unknown type
                invalid = -1;
            else if(!built_in_signal)//buf will be overwrite for unformatted outpt message
                invalid = 1;
            // printf("message: %s\n", message);
            // printf("invalid: %i\n", invalid);

            //change message to indicate invalid status since it has been checked invalid for any reason 
            if(invalid == -1)
                strncpy(buf, "-1\0", MAXLINE); 
            else if(invalid)
                strncpy(buf, "1\0", MAXLINE); 

            //format message before sending
            char messageSize = strlen(buf) + '0';
            message[0] = messageSize;
            message[1] = '\0';
            strncat(message, buf, MAXLINE);

            //send message to client 
            write(connfd, message, MAXLINE);
            // printf("message: %s\n", message);

        }

        // echo(connfd);
        close(connfd); 
    }
    // printf("server end\n"); //delete???
    exit(0);
}

void readData(char *fileName, struct stock *type_stocks)
{
    FILE* fp = fopen(fileName, "r");

    char buffer[1024];
    int row = 0;
    int column = 0;
    int indexPFE = 0;

    while(fgets(buffer, 1024, fp))
    {
        column = 0;
        
        row++;
        if(row == 1)
            continue;

        char* value = strtok(buffer, ",");
        while(value)
        {
            if(column == 0)
                strncpy(type_stocks[indexPFE].date, value, MAXLINE);
                
            if(column == 4)
                type_stocks[indexPFE].close = atof(value);

            value = strtok(NULL, ",");
            column++;
        }
        indexPFE++;
    }

    fclose(fp);
}

int open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;
    
    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; /* Accept connect. */ 
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ...on any IP addr */ 
    hints.ai_flags |= AI_NUMERICSERV; /* ...using port no. */ 
    getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

    /* Eliminates "Address already in use" error from bind */
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
   
    /* Bind the descriptor to the address */
    if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) 
        break; /* Success */
    close(listenfd); /* Bind failed, try the next */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept conn. requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);    
        return -1;
    }
    return listenfd;
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

void parseDate(char *theDate, const char *delims) //parsing date
{
    char * tok = strtok(theDate, delims);
    unsigned indexDate = 0;
    while(tok != NULL)
    {
        strncpy(date[indexDate], tok, 5);
        tok = strtok(NULL, delims);
        indexDate++;
    }
}

bool searchDate(int *index, char *theDate) //true if found; otherwise, false.
{
    //parsing date
    const char *delims = "-";
    parseDate(theDate, delims);

    //input date
    int day, month, year;
    year = atoi(date[0]);
    month = atoi(date[1]);
    day = atoi(date[2]);
    // printf(" year: %i month: %i day: %i\n", year, month, day);

    char buf[MAXLINE] = "\0";
    char int_str[5];
    char ch[] = "/";
    char ch0[] = "\0";
    sprintf(int_str, "%d", month);
    strncat(buf, int_str, MAXLINE); //append month
    strncat(buf, ch, MAXLINE); // "/"
    sprintf(int_str, "%d", day);
    strncat(buf, int_str, MAXLINE); //append day
    strncat(buf, ch, MAXLINE); // "/"
    sprintf(int_str, "%d", year);
    strncat(buf, int_str, MAXLINE); //append year
    strncat(buf, ch0, MAXLINE); // "\0"
    // printf("buf: %s\n\n", buf);

    //search in PFE_stocks
    int i=0;
    for(; i<STOCKSIZE; i++)
    {
        //the PFE.csv and MRNA.csv files have the same exact dates and same number of entries, so just checking in PFE.csv should be enough
        //https://piazza.com/class/l8bz16ajmq660p/post/278
        if(!strcmp(buf, PFE_stocks[i].date)) // && !strcmp(buf, MRNA_stocks[i].date)
        {
            *index = i;
            return true;
        }
    }
    
    return false;
}

double maxProfitFunction(int indexStart, int indexEnd, struct stock *typeStock)
{
    double maxProfit = 0.0;
    char buyDay[MAXLINE] = "\0";
    char sellDay[MAXLINE] = "\0";

    int i, j;
    for(i=indexStart; i< indexEnd+1; i++)
    {
        for(j=i; j<indexEnd+1; j++)
        {
            double diff = typeStock[j].close - typeStock[i].close;
            if(maxProfit <= diff) //why need to be equal???
            {
                maxProfit = diff;
                strncpy(buyDay,  typeStock[i].date, MAXLINE);
                strncpy(sellDay,  typeStock[j].date, MAXLINE);
            } 
        }
    }
    return maxProfit;
}

double maxLossFunction(int indexStart, int indexEnd, struct stock *typeStock)
{
    double maxLoss = 0.0;
    char buyDay[MAXLINE] = "\0";
    char sellDay[MAXLINE] = "\0";

    int i, j;
    for(i=indexStart; i< indexEnd+1; i++)
    {
        for(j=i; j<indexEnd+1; j++)
        {
            double diff = typeStock[j].close - typeStock[i].close;
            if(maxLoss >= diff) //why need to be equal???
            {
                maxLoss = diff;
                strncpy(buyDay,  typeStock[i].date, MAXLINE);
                strncpy(sellDay,  typeStock[j].date, MAXLINE);
            } 
        }
    }
    return maxLoss;
}

void parsingDate(char *theDate, int *day, int *month, int *year, const char *delims)
{
    //parsing date
    // const char *delims = "-";
    char * tok = strtok(theDate, delims);
    char date[3][5]; //year month day. //four digits for year + '\0' ==> 5 space total
    unsigned indexDate = 0;
    while(tok != NULL)
    {
        strncpy(date[indexDate], tok, 5);
        // printf("\ndate %i: %s\n", indexDate, date[indexDate]);
        tok = strtok(NULL, delims);
        indexDate++;
    }
    // int i=0;
    // for(; i<3; i++)
    //     printf("\ndate %i: %s\n", i, date[i]);

    //valid date (useful dates that after 1900)
    *year = atoi(date[0]);
    *month = atoi(date[1]);
    *day = atoi(date[2]);
}

int invalidDate(char *theDate) //return 1 if date is invalid in term of format. return 0 if valid. return -1 if date is actual date but not within range of 7/2/2019 and 6/30/2021
{
    //check whether the date check valid or not 
    if(theDate[4] != '-' || theDate[7] != '-')
        return 1;

    //e.g. 2019-07-02 ==> abcd-ef-gh, then it will be "Invalid Syntax"
    int i=0; 
    for(; i<10; i++)
    {
        if(i == 4 || i == 7)
            continue;
        if(!isdigit(theDate[i]))
            return 1;
    }

    //parsing date
    int day, month, year;
    const char *delims = "-";
    parsingDate(theDate, &day, &month, &year, delims);
    // printf("\nyear %i\n", year);
    // printf("month %i\n", month);
    // printf("day %i\n", day);

    struct tm input = {
        .tm_mday = day,
        .tm_mon = month - 1,
        .tm_year = year - 1900,
    }; //date
    time_t t = mktime(&input); 
    struct tm *output = localtime(&t); 
    if (day != output->tm_mday || month != output->tm_mon + 1|| year != output->tm_year + 1900)
        return -1;

    //check date within range of 7/2/2019 and 6/30/2021.
    struct tm earliest = {
        .tm_mday = 2,
        .tm_mon = 7 - 1,
        .tm_year = 2019 - 1900,
    };
    struct tm latest = {
        .tm_mday = 30,
        .tm_mon = 6 - 1,
        .tm_year = 2021 - 1900,
    };
    if((difftime(mktime(&input), mktime(&earliest)) < 0) ||(difftime(mktime(&latest), mktime(&input)) < 0))
        return -1;

    return 0; //valid date
}

bool isReverse(char *theDate1, char* theDate2) //return true if date reverse. Otherwise, return false. 
{
    //parsing date
    const char *delims = "-";
    int day1, month1, year1;
    int day2, month2, year2;
    parsingDate(theDate1, &day1, &month1, &year1, delims);
    parsingDate(theDate2, &day2, &month2, &year2, delims);

    //put each date to struct tm
    struct tm input1 = {
        .tm_mday = day1,
        .tm_mon = month1 - 1,
        .tm_year = year1 - 1900,
    }; //date
    struct tm input2 = {
        .tm_mday = day2,
        .tm_mon = month2 - 1,
        .tm_year = year2 - 1900,
    }; //date

    if((difftime(mktime(&input1), mktime(&input2)) > 0))
        return true;

    return false;
}

int built_in_commands(char *message)
{ 
    if (!strcmp(argv[0], "PricesOnDate"))
    {
        if(!argv[1])
            return 0;

        //search the exisitence of actual date.
        char bufDate[MAXLINE];
        strncpy(bufDate, argv[1], MAXLINE);
        if (invalidDate(bufDate) == 1)  // if date is invalid in term of format
            return 0; //for printing "Invalid syntax"
        else if (invalidDate(bufDate) == -1) //if date is actual date but not within range of 7/2/2019 and 6/30/2021, 
            return -1;  //for printing "Unknown"

        //search whehter the date exist in csv files or not
        //if both csv file has that day, then do nothing; otherwise, return -1
        int indexInput = -1;
        // char *theDate = argv[1];
        if(!searchDate(&indexInput, argv[1]) && indexInput == -1)
            return -1; 
        // printf("indexInput: %i\n", indexInput);

        //output message
        char buf[MAXLINE] = "\0";
        char int_str[5]; //only 4 digits + '\0'
        char ch0 = '\0';
        char ch1[] = "PFE: ";
        char ch2[] = " | ";
        char ch3[] = "MRNA: ";
        strncat(buf, ch1, MAXLINE); //"PFE: "
        sprintf(int_str, "%0.2f", PFE_stocks[indexInput].close); 
        strncat(buf, int_str, MAXLINE); // "PFE: value"
        strncat(buf, ch2, MAXLINE); // "PFE: value | "
        strncat(buf, ch3, MAXLINE); //"PFE: value | MRNA: "
        sprintf(int_str, "%0.2f", MRNA_stocks[indexInput].close); 
        strncat(buf, int_str, MAXLINE); // "PFE: value | MRNA: value"
        strncat(buf, &ch0, MAXLINE); // // "PFE: value | MRNA: value\0"

        //transfer formated string to message varialbe
        strncpy(message, buf, MAXLINE);
        // printf("message: %s\n", message);

        return 1;
    }
    else if (!strcmp(argv[0], "MaxPossible"))
    {
        // printf("MaxPossible\n\n");

        if(!argv[1] || !argv[2] || !argv[3] || !argv[4])
            return 0;

        //if not argv[2] "PFE" or "MRNA", (using De Morgan's) then invalid syntax
        if (strcmp(argv[2], "PFE") && strcmp(argv[2], "MRNA"))
            return 0;

        //the date of argv[3] should be earlier than the date of argv[4], which argv[3] < argv[4]. So if argv[3] > argv[4], then it reverses,  
        //check whether the order of the date is reverse or not or not. 
        //https://piazza.com/class/l8bz16ajmq660p/post/287
        char bufDate1[MAXLINE], bufDate2[MAXLINE];
        strncpy(bufDate1, argv[3], MAXLINE);   
        strncpy(bufDate2, argv[4], MAXLINE);   
        if(isReverse(bufDate1, bufDate2)) //If date is reverse in term of format
            return 0; //for printing "Invalid syntax"

        //search the exisitence of actual date.
        // char bufDate1[MAXLINE], bufDate2[MAXLINE];
        strncpy(bufDate1, argv[3], MAXLINE); 
        strncpy(bufDate2, argv[4], MAXLINE);
        if(invalidDate(bufDate1) == 1 || invalidDate(bufDate2) == 1 ) //return 1 if date is invalid in term of format. return 0 if valid. return -1 if date is actual date but not within range of 7/2/2019 and 6/30/2021
            return 0;
        else if (invalidDate(bufDate1) == -1 && invalidDate(bufDate2) == -1 )
            return -1;

        //search whehter the dates exist in csv files or not 
        //argv[3] && argv[4] 
        int indexStart, indexEnd; 
        if(!searchDate(&indexStart, argv[3]) || !searchDate(&indexEnd, argv[4]))
        {
            invalid = -1; //unknown date
            return 1;
        }   
        // printf("indexStart: %i\n", indexStart);
        // printf("indexEnd: %i\n\n", indexEnd);

        if (!strcmp(argv[1], "profit"))
        {
            // printf("profit\n");
            
            double maxProfit;
            if (!strcmp(argv[2], "PFE"))
                maxProfit = maxProfitFunction(indexStart, indexEnd, PFE_stocks);
            else if (!strcmp(argv[2], "MRNA"))
                maxProfit = maxProfitFunction(indexStart, indexEnd, MRNA_stocks);
            // printf("maxProfit: %f\n\n", maxProfit);

            //convert maxProfit to string
            char int_str[5]; //only 4 digits + '\0'
            sprintf(int_str, "%0.2f", maxProfit); 

             //transfer formated string to message varialbe
            strncpy(message, int_str, MAXLINE);
            // printf("message: %s\n\n", message);
        }
        else if (!strcmp(argv[1], "loss"))
        {
            // printf("loss\n");

            double maxLoss;
            if (!strcmp(argv[2], "PFE"))
                maxLoss = maxLossFunction(indexStart, indexEnd, PFE_stocks);
            else if (!strcmp(argv[2], "MRNA"))
                maxLoss = maxLossFunction(indexStart, indexEnd, MRNA_stocks);
            // printf("maxLoss: %f\n\n", maxLoss);

            //convert negative to positive
            if(maxLoss < 0)
                maxLoss *= -1;

            //convert maxProfit to string
            char int_str[5]; //only 4 digits + '\0'
            sprintf(int_str, "%0.2f", maxLoss); 

            //transfer formated string to message varialbe
            strncpy(message, int_str, MAXLINE);
            // printf("message: %s\n\n", message);
        }
        else
            return 0;

        return 1;
    }

    return 0;
}

//profit , buy low sell high. max
//loss, buy high sell low. max