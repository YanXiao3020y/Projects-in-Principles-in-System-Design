#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 80

struct Memory {
    int address, data;
};

struct PageTable {
    int v_page_num, valid_bit, dirty_bit, page_num, time_stamp;
};

char *argv[MAXLINE]; //for user input
int argc; //the actual size of argv
struct Memory main_memory[32];
struct Memory disk_memory[128];
struct PageTable p_table[16];  //virtual memory
int memoryCounter[4] = {-4, -3, -2, -1}; //in this case, 0 will be first out since it is the smallest number. //This array using for LRU
int counter = 0; //to keep track where the victim page is in main_memory using FIFO.
int counterLRU = 0; //to keep track where the victim page is in main_memory using LRU.
int counterLRU2 = 0;
char *method; //store a string of FIFO or LRU.


void initialize(); //initialize all memory locations to -1 and pages to 0
void eval(char *cmdline);
int built_in_commands();
int parseline(char* buf);
void helperRWFIFO();
void helperRWLRU();
int FIFO();
int LRU();
void counterIncrementLRU(); //increse counter of main_memory

int main(int argcM, char *argvM[] )
{
    method = argvM[1];
//    printf("%s", method);
    initialize();
    while(1)
    {
        char cmdline[MAXLINE];
        printf("> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
                exit(0);
        eval(cmdline);

    }


    return 0;
}

void initialize()
{
    for (int i = 0; i < sizeof(main_memory)/sizeof(main_memory[0]); i++)
    {
        main_memory[i].data = -1;
        main_memory[i].address = i;
    }

    for (int i = 0; i < sizeof(disk_memory)/sizeof(disk_memory[0]); i++)
    {
        disk_memory[i].data = -1;
        disk_memory[i].address = i;
    }

    for (int i = 0; i < sizeof(p_table)/sizeof(p_table[0]); i++)
    {
        p_table[i].v_page_num = p_table[i].page_num = i;
        p_table[i].valid_bit = p_table[i].dirty_bit = 0;
//        p_table[i].time_stamp = 0;
    }
}

void eval(char *cmdline)
{
    char buf[MAXLINE];
    strcpy(buf, cmdline);
    parseline(buf);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */

    if (!built_in_commands())
        printf("Invalid command!\n");
}

int built_in_commands()
{
    if (!strcmp(argv[0], "quit"))
    {
        exit(0);
        return 1;
    }

    else if (!strcmp(argv[0], "read"))
    {
        if(argv[1])
        {
            counterLRU2++;
            int address = atoi(argv[1]);
            
            if(address < 0 || address >= 128 )
            {
                printf("read command: invalid address!\n");
                return 1;
            }
            int pageNumP = address/8; //page number in page table
            if(p_table[pageNumP].valid_bit == 0)
            {
                if(method == NULL) //default. e.g. "./a.out"
                    helperRWFIFO();
                else
                {
                    if(!strcmp(method, "FIFO"))
                    {
                        helperRWFIFO();
                    }
                    else
                        helperRWLRU();
                }
                
            }
            
             //read process
            int pageNumM = p_table[pageNumP].page_num ; //for main_memory
            int index = pageNumM * 8 + address % 8;
            printf("%i \n", main_memory[index].data);
            counterIncrementLRU(); //increse counter of main_memory
        }
        else
            printf("read command: no address given!\n");
        return 1;
    }

    else if (!strcmp(argv[0], "write"))
    {
        
//        printf("write\n");
        
        if(argv[1] && argv[2]) //if arguement 1 (page number) & arguement 2 (data to be written) exist
        {
            counterLRU2++;
            int address = atoi(argv[1]);
            if(address < 0 || address >= 128 )
            {
                printf("write command: invalid address!\n");
                return 1;
            }
            
            int pageNumP = address/8; //page number in page table
            int data = atoi(argv[2]);

            if(p_table[pageNumP].valid_bit == 0)
            {
                if(method == NULL) //default. e.g. "./a.out"
                    helperRWFIFO();
                else
                {
                    if(!strcmp(method, "FIFO")) //e.g. "./a.out FIFO"
                        helperRWFIFO();
                    else //only one e.g. "./a.out LRU"
                        helperRWLRU();
                }
            }
            
            //Overwrite main_memory data
            int pageNumM = p_table[pageNumP].page_num; //page number in main_memory
            int indexM = pageNumM * 8 + address % 8; //index in main_memory
            main_memory[indexM].data = data; //write in data in main_memory
            p_table[pageNumP].dirty_bit = 1;
            counterIncrementLRU(); //increse counter of main_memory
        }
        else if(argv[1])
        {
            int address = atoi(argv[1]);
            if(address < 0 || address >= 128 )
            {
                printf("write command: invalid address!\n");
                return 1;
            }
            
            printf("write command: invalid value!\n");
        }
        else
            printf("write command: no address given!\n");
            
        return 1;
    }

    else if (!strcmp(argv[0], "showmain"))
    {
//        printf("showmain\n");
        if(argv[1]) //if arguement 1 (page number) exist
        {
            int bound = atoi(argv[1]);
            if(bound < 0 || bound >= 4 )
            {
                printf("showmain command: invalid page!\n");
                return 1;
            }
            int pageNumM = atoi(argv[1]); //page number in main_memory
            for(int i=0; i<8; i++)
                printf("%i: %i \n", main_memory[i + pageNumM * 8].address,  main_memory[i + pageNumM * 8].data);
        }
        else
            printf("showmain command: no page given!\n");

        return 1;
    }

     else if (!strcmp(argv[0], "showptable"))
    {
//        printf("showptable\n");
        
        for(int i=0; i < sizeof(p_table)/sizeof(p_table[0]); i++)
            printf("%i:%i:%i:%i\n", p_table[i].v_page_num, p_table[i].valid_bit, p_table[i].dirty_bit, p_table[i].page_num);
        
        return 1;
    }
    

    return 0;
}


void helperRWFIFO() //this helper function designs for read and write operation
{
    //find a victim page
    int vPage = FIFO();
    //1. maybe using if "FIFO" else "LSU" to accept the argument (argvM[1] (main(int argcM, char *argvM[] ))
    //https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
    //2. put the previous step in main() and pass as arugment into this helperRWFIFO(vPage) function
    
    printf("A Page Fault Has Occurred\n");
    int pageNumP = 0; //victim page number in page table
    if(counter >= 4)
    {
        for(int i=0; i < sizeof(p_table)/sizeof(p_table[0]); i++)
        {
            if(p_table[i].valid_bit == 1 && p_table[i].page_num == vPage)
                pageNumP = p_table[i].v_page_num;
        }
    
        int pageNumM = vPage; //page number in main_memory
        if(p_table[pageNumP].dirty_bit == 1)
        {
            //copy victim page from main_memory to disk_memory using virtual page table
            for(int i=0; i<8; i++)
                disk_memory[pageNumP * 8 + i].data = main_memory[pageNumM * 8 + i].data;
        }
    
        //reset valid_bit & dirty to 0, and update page number to virtual page number.
        p_table[pageNumP].valid_bit = 0;
        p_table[pageNumP].dirty_bit = 0;
        p_table[pageNumP].page_num = p_table[pageNumP].v_page_num;
    }
    
    int address = atoi(argv[1]);
    pageNumP = address/8; //page number in page table
    //overwrite the main_memory. copy the desired disk_memory to main_memory using virtual page table
    for(int i=0; i<8; i++) //copy victim page from main_memory to disk_memory using virtual page table
        main_memory[vPage * 8 + i].data = disk_memory[pageNumP * 8 + i].data;
    
    //load the page to main_memory
    p_table[pageNumP].valid_bit = 1;
    p_table[pageNumP].page_num = counter % 4;
    counter++; //increase to next available page in main_memory
}

void helperRWLRU()
{
    //find a victim page
    int vPage = LRU();
    //1. maybe using if "FIFO" else "LSU" to accept the argument (argvM[1] (main(int argcM, char *argvM[] ))
    //https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
    //2. put the previous step in main() and pass as arugment into this helperRWFIFO(vPage) function
    
    printf("A Page Fault Has Occurred\n");
    int pageNumP = 0; //victim page number in page table
    if(counterLRU2 >= 4)
    {
        for(int i=0; i < sizeof(p_table)/sizeof(p_table[0]); i++)
        {
            if(p_table[i].valid_bit == 1 && p_table[i].page_num == vPage)
                pageNumP = p_table[i].v_page_num;
        }
    
        int pageNumM = vPage; //page number in main_memory
        if(p_table[pageNumP].dirty_bit == 1)
        {
            //copy victim page from main_memory to disk_memory using virtual page table
            for(int i=0; i<8; i++)
                disk_memory[pageNumP * 8 + i].data = main_memory[pageNumM * 8 + i].data;
        }
    
        //reset valid_bit & dirty to 0, and update page number to virtual page number.
        p_table[pageNumP].valid_bit = 0;
        p_table[pageNumP].dirty_bit = 0;
        p_table[pageNumP].page_num = p_table[pageNumP].v_page_num;
    }
    
    
    int address = atoi(argv[1]);
    pageNumP = address/8; //page number in page table
    //overwrite the main_memory. copy the desired disk_memory to main_memory using virtual page table
    for(int i=0; i<8; i++) //copy victim page from main_memory to disk_memory using virtual page table
        main_memory[vPage * 8 + i].data = disk_memory[pageNumP * 8 + i].data;
    
    //load the page to main_memory
    p_table[pageNumP].valid_bit = 1;
    p_table[pageNumP].page_num = vPage;
   
}

void counterIncrementLRU()
{
    int address = atoi(argv[1]);
    int pageNumP = address/8; //page number in page table
    int pageNumM = p_table[pageNumP].page_num;
    memoryCounter[pageNumM] = counterLRU;
    counterLRU++; //increase to next available page in main_memory
//    for(int i=0; i<4; i++)
//        printf("%i ", memoryCounter[i]);
//    printf("\n");
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

int FIFO()
{
    return counter % 4;
}

int LRU()
{
    int min = memoryCounter[0];
    int index = 0;
    for(int i=1; i<4; i++)
        if(min > memoryCounter[i])
        {
            min = memoryCounter[i];
            index = i;
        }
            
    return index;
}

//note:

//will address ever be < 0? yes
//will input value for writing in < 0? no, still can be negative

//write
//if the dirty bit is 1, do copy main to disk memory; then overwirte main memory
//if dirty bit is 0, then overwirte  main memory
//how about evict page?

//if valid_bit = 0 for evited page, valid_bit = 1 for new load page



//read
//valid_bit = 1 for new load page


//read checking dirty bit
//write directly assign dirty bit
//write check dirty bit too, need to think again.

//LRU
//read or write; update time stamp
//make main_memory with a extra variable for time_stamp, then just update each of them if they do read or write.

//FIFO
//update tiem stamp --> jsut move something from disk_memo to main_memo
//using pagenumber = counter %4 + 1, use this pagenumber to evict which page


//victim -->using FIFO or LRU ??? return pagenumber in main_memory
//how to do either of them on terminal???

//every time do write, assign dirty number to 1. when should I assign it to 0???
//==>from main_ to disk_memory, reset valid_bit & dirty to 0, and update page number to virtual page number.
