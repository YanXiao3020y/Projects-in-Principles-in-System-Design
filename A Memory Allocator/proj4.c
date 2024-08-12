#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

 
#define MAXARGS 80 //MaxArgc
#define SIZE 127 //size of heap

struct block
{
    int size; //no header & footer
    int startIndex; //start of the payload
    bool isAllocated; //true: allocated; false: free
};
struct block blockList[SIZE];
//int listSize = 0; //size of blockList


char *argv[MAXARGS]; //for user input
int argc; //the actual size of argv
char *method; //store a string of First fit or Best fit.
char andTest = 1; //0000 0001 for last digit with &

unsigned char heap[SIZE];

void initialize()
{
    heap[0] = 254; //254 in ASCII //1111 1110 in binary
    heap[126] = 254; //254 in ASCII //1111 1110 in binary
    for(int i=1; i<SIZE-1; i++)
    {
        heap[i] = 0; //0 in ASCII // 0000 0000 in binary
        blockList[i].size = -1;
        blockList[i].startIndex = -1;
        blockList[i].isAllocated = false; //false: free
    }
    
//    //initilize the whole heap
//    blockList[0].size = 125;
//    blockList[0].startIndex = 1;
//    blockList[0].isAllocated = false; //false: free
//    listSize++;
}



void eval(char *cmdline);
int parseline(char* buf);
int built_in_commands();
int helperFirstFit(int sizeofNewBlock); //for malloc only, returns the started address for header
int helperBestFit(int sizeofNewBlock); //for malloc only, returns the started address for header

int main(int argcM, char *argvM[] )
{
    initialize();
    method = argvM[1];
    // method = "BestFit"; // delete???
    while(1)
    {
        char cmdline[MAXARGS];
        printf(">");
        fgets(cmdline, MAXARGS, stdin);
        if (feof(stdin))
                exit(0);
        eval(cmdline);

    }
    
    return 0;
}

void eval(char *cmdline)
{
    char buf[MAXARGS];
    strcpy(buf, cmdline);
    parseline(buf);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */

    if (!built_in_commands())
        printf("Invalid command!\n");
}

int parseline(char* buf)
{
    //change '\t' to ' '
    for(int i=0; i<strlen(buf); i++)
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

int built_in_commands()
{
   
    
    if (!strcmp(argv[0], "quit"))
    {
        exit(0);
        return 1;
    }

    else if (!strcmp(argv[0], "malloc")) //done
    {
        if(argv[1])
        {
            int sizeofNewBlock = atoi(argv[1]);
            if((sizeofNewBlock <= 0 || sizeofNewBlock > 125 ) || !isdigit(argv[1][0])) //???size of malloc cannot exceed 125 due to header & footer
            {
                printf("malloc command: invalid size!\n");
                return 1;
            }

            //===============body begin=========================
            int header = -1; //header of the new block
            if(method == NULL) //default. e.g. "./a.out"
                header = helperFirstFit(sizeofNewBlock);
            else
            {
                if(!strcmp(method, "FirstFit"))
                {
                
                    header = helperFirstFit(sizeofNewBlock);
                }
                else if (!strcmp(method, "BestFit"))
                    header = helperBestFit(sizeofNewBlock);
                else
                    printf("Invalid input for Memory Allocation Strategie!\n");
            }
            
            if(header == -1)
                printf("No space is available in the heap!\n");
            else
                printf("%i\n", header + 1);
            //===============body end===========================
//            printf("malloc\n");
        }
        else
            printf("malloc command: no address given!\n");

        return 1;
    }

    else if (!strcmp(argv[0], "free"))
    {
        if(argv[1])
        {
            int address = atoi(argv[1]);
            
            if((address < 0 || address >= 127 ) || !isdigit(argv[1][0]))
            {
                printf("free command: invalid address!\n");
                return 1;
            }
            
            //===============body begin=========================
            //free current block content
            
            int headerC = address - 1;
            int sizeOfPayloadC = heap[headerC] >> 1; //the size of current block payload
            int footerC = headerC + sizeOfPayloadC - 1;
            int totalSizePayload = sizeOfPayloadC;
            
            for(int i=address; i<=sizeOfPayloadC - 2; i++) //initilize content(without header & footer) to contain 0
                heap[i] = 0;
            heap[headerC] = heap[headerC] - 1; //change the header of the allocated block to free status
            heap[footerC] = heap[headerC]; //change the footer of the allocated block to free status
            
            
            //connect previous free block
            int footerP = address - 2; //footer of previous block
            bool isAllocatedP = (int)(heap[footerP] & andTest); //using last bit to check allocated or not. //true: allocated; false: free
            
            if(footerP > 0 && isAllocatedP == false)
            {
                int sizeOfPayloadP =  heap[footerP] >> 1;//the size of previous block payload
                int headerP = footerP - sizeOfPayloadP + 1;
                totalSizePayload = sizeOfPayloadC + sizeOfPayloadP;
                heap[headerP] = totalSizePayload << 1;
                heap[footerP] = 0;
                heap[headerC] = 0;
                heap[headerP + totalSizePayload - 1] = heap[headerP];
                
                headerC = headerP;
//                sizeOfPayloadC = totalSizePayload;
            }
            
            //connect next free block
            int headerN = address + sizeOfPayloadC - 1; //the index header of next block
            bool isAllocatedN = (int)(heap[headerN] & andTest); //using last bit to check allocated or not. //true: allocated; false: free
            if(headerN < SIZE && isAllocatedN == false) //if free
            {
                int sizeOfPayloadN =  heap[headerN] >> 1;//the size of next block payload
                int footerN = headerN + sizeOfPayloadN - 1;
                totalSizePayload = totalSizePayload + sizeOfPayloadN;
                heap[headerC] = totalSizePayload << 1;
                heap[footerC] = 0;
                heap[headerN] = 0;
                heap[footerN] = heap[headerC];

            }
            
            
            //===============body end===========================
//            printf("heap[0] = %d\n", heap[0]);
//            printf("free\n");
        }
        else
            printf("malloc command: no address given!\n");

        return 1;
    }

    else if (!strcmp(argv[0], "blocklist"))
    {
        //===============body begin=========================
        int current = 0;
        int counter = 0; //size of blockList
        //scan
        for(int i=0; i<SIZE; i+=current)
        {
            current = heap[i] >> 1;
            blockList[counter].size = current - 2; //payload size, which does not include header & footer
            blockList[counter].startIndex = i + 1; //header + 1
            blockList[counter].isAllocated = heap[i] & andTest;
            counter++;
        }
        
        //sort
        for(int i=0; i<counter; i++)
        {
            for(int j=i+1; j<counter; j++)
            {
                if(blockList[i].size < blockList[j].size) // big then small //???
                {
                    //swap
                    int tempSize = blockList[i].size;
                    int tempStartIndex = blockList[i].startIndex;
                    bool tempIsAllocated = blockList[i].isAllocated;
                    
                    blockList[i].size = blockList[j].size;
                    blockList[i].startIndex = blockList[j].startIndex;
                    blockList[i].isAllocated = blockList[j].isAllocated;
                    
                    blockList[j].size = tempSize;
                    blockList[j].startIndex = tempStartIndex;
                    blockList[j].isAllocated = tempIsAllocated;
                }
                else if (blockList[i].size == blockList[j].size)
                {
                    if(blockList[i].startIndex > blockList[j].startIndex)
                    {
                        //swap
                        int tempSize = blockList[i].size;
                        int tempStartIndex = blockList[i].startIndex;
                        bool tempIsAllocated = blockList[i].isAllocated;
                        
                        blockList[i].size = blockList[j].size;
                        blockList[i].startIndex = blockList[j].startIndex;
                        blockList[i].isAllocated = blockList[j].isAllocated;
                        
                        blockList[j].size = tempSize;
                        blockList[j].startIndex = tempStartIndex;
                        blockList[j].isAllocated = tempIsAllocated;
                    }
                    
                }
            }
        }
        
        //print
        for(int i=0; i<counter; i++)
        {
            if(blockList[i].isAllocated == true)
                printf("%i-%i-allocated\n", blockList[i].size, blockList[i].startIndex);
            else
                printf("%i-%i-free\n", blockList[i].size, blockList[i].startIndex);
        }
        
        //===============body end===========================

//        printf("blocklist\n");
        return 1;
    }

    else if (!strcmp(argv[0], "writemem"))
    {
        if(argv[1] && argv[2]) //if arguement 1 (address) & arguement 2 (data to be written) exist
        {
            int address = atoi(argv[1]);
            if((address < 0 || address >= 127 ) || !isdigit(argv[1][0]) )
            {
                printf("writemem command: invalid address!\n");
                return 1;
            }

            //===============body begin=========================
            char *str = argv[2]; //this string cannot contain any whitespace
            //alpha-numeric characters include negative nubmer ???
//            printf("%s\n", str);
//            printf("%i\n", strlen(str));
            for(int i=0; i<strlen(str); i++)
                if(!isalnum(str[i]))
                {
                    printf("writemem command: invalide value!\n");
                    return 1;
                }
            
            for(int i=0; i<strlen(str); i++)
                heap[address++] = str[i];
            
            //===============body end===========================

//            printf("writemem\n");
        }
        else if(argv[1])
        {
            int address = atoi(argv[1]);
            if((address < 0 || address >= 127 ) || !isdigit(argv[1][0]) )
            {
                printf("writemem command: invalid address!\n");
                return 1;
            }
            
            printf("writemem command: invalid value!\n"); //only have arguement 1
        }
        else
            printf("writemem command: no address given!\n");

        return 1;
    }

    else if (!strcmp(argv[0], "printmem"))
    {
        if(argv[1] && argv[2]) //if arguement 1 (address) & arguement 2 (data to be written) exist
        {
            int address = atoi(argv[1]);
            if((address < 0 || address >= 127 ) || !isdigit(argv[1][0]) )
            {
                printf("writemem command: invalid address!\n");
                return 1;
            }

            //===============body begin=========================
            int strSize = atoi(argv[2]);
            int lastIndex = address + strSize;
            for(int i=address; i<lastIndex-1; i++)
                printf("%d-", heap[i]);
            printf("%d\n", heap[lastIndex-1]);
            //===============body end===========================
            
//            printf("printmem\n");
        }
        else if(argv[1])
        {
            int address = atoi(argv[1]);
            if((address < 0 || address >= 127) || !isdigit(argv[1][0]) )
            {
                printf("printmem command: invalid address!\n");
                return 1;
            }
            
            printf("printmem command: invalid value!\n"); //only have arguement 1
        }
        else
            printf("printmem command: no address given!\n");
            
        return 1;
    }

    return 0;
}

int helperFirstFit(int sizeofNewBlock)
{
//    printf("helperFirstFit\n");
    
    int header = -1; //header of the current new block
    //when malloc cannot allocoted more, what the value should be print to screen???
    int current = 0; // size of current block
    
    for(int i=0; i<SIZE; i+=current)
    {
        current = heap[i] >> 1;
        
        if((int)(heap[i]&andTest) == 0) //using & //0:free; 1:allocated
        {
            int fullBlock = sizeofNewBlock + 2; //include header & payload & footer
            int diff = current - fullBlock; //difference between the newblock & current block
            
            //feel two if branch can combine ???
            if(diff >= 0) //can use this block
            {
                
                
                if (diff >= 3) //spilt the block as two blocks
                {
                    //seperate two block
                    heap[i] = (fullBlock << 1) + 1; //change the header of the allocated block
//                    int abc = (fullBlock * 2) + 1;
//                    heap[i] = abc; //change the header of the allocated block
                    heap[i+fullBlock-1] = (fullBlock << 1) + 1;  //change the footer of the allocated block
                    heap[i+fullBlock] = (diff << 1); //change the header of the free block
                    heap[i+current-1] = (diff << 1);  //change the footer of the free block

//                    printf("head1: %d\n", heap[i]);
//                    printf("foot1: %d\n",  heap[i+sizeofNewBlock-1]);
//                    printf("head2: %d\n", heap[i+sizeofNewBlock]);
//                    printf("foot2: %d\n", heap[i+current-1]);
                    header = i;
                    
                    break;
                }
                
//                heap[i] = (current << 1) + 1; //change the header of the allocated block
                heap[i] += 1; //change the header of the allocated block
                heap[i+current-1] += 1;  //change the footer of the allocated block
                header = i;
                break;
            }
        }
    }

    return header;
}

int helperBestFit(int sizeofNewBlock)
{
//    printf("helperBestFit\n");
    
    int header = -1; //header of the current new block
    //when malloc cannot allocoted more, what the value should be print to screen???
    int current = 0; // size of current block
    
    int minSize = 999; //keep track of the current best
    int minIndex = -1; //index of current best
    int sizeOfFullBlock = sizeofNewBlock + 2; //include header & payload & footer
    
    
    for(int i=0; i<SIZE; i+=current)
    {
        current = heap[i] >> 1;
        if((int)(heap[i]&andTest) == 0) //using & //0:free; 1:allocated //???
        {
            int diff = current - sizeOfFullBlock; //difference between the newblock & current block
            if(diff < minSize && diff >= 0)
            {
                minSize = diff;
                minIndex = i;
            }
        }
    }
          
    if(minIndex == -1) //can use this block
    {
        return header;
    }
    else
    {
        current = heap[minIndex] >> 1;
        if (minSize >= 3) //spilt the block as two blocks
        {
            //seperate two block
            heap[minIndex] = (sizeOfFullBlock << 1) + 1; //change the header of the allocated block
            heap[minIndex+sizeOfFullBlock-1] = (sizeOfFullBlock << 1) + 1;  //change the footer of the allocated block
            heap[minIndex+sizeOfFullBlock] = (minSize << 1); //change the header of the free block
            heap[minIndex+current-1] = (minSize << 1);  //change the footer of the free block

    //                    printf("head1: %d\n", heap[i]);
    //                    printf("foot1: %d\n",  heap[i+sizeofNewBlock-1]);
    //                    printf("head2: %d\n", heap[i+sizeofNewBlock]);
    //                    printf("foot2: %d\n", heap[i+current-1]);
            header = minIndex;
            
          
        }
        else
        {
            heap[minIndex] += 1; //change the header of the allocated block
            heap[minIndex+current-1] += 1;  //change the footer of the allocated block
            header = minIndex;
        }
    }
        
    

    return header;
}


//char a[size];
//
//a[0] = 65 or 'A'


//setSize, getSize, getLastBit()
//void setSize(int index, int size)
//{
//    heap[index] = heap[index] >> 1
//}
