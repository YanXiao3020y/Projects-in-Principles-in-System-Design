#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

 

#define MAXLINE 80
#define MAXARGS 80 //MaxArgc
#define MAXJOBS 5

int const SIZE = 200;
char *argv[MAXARGS];
int argc; //the actual size of argv

struct job{
    pid_t pid; //-1: not exist; other integers: exist
    int status; //1:foreground; 2: background; 3: stopped; 0: undefine
    char commandLine[MAXLINE];
}
jobList[MAXJOBS] = { [0 ... MAXJOBS - 1] = { -1, 0} }; //initilize pid = -1 and status = 0 as its default value.
 



void eval(char *cmdline);
int parseline(char *buf);
int built_in_commands();


int fgCheckingWithID()  //checking whether there is at least one foreground job. If yes, return the first occurrence of jobID; if not, return 0 for other situations.
{
    for(int i=0; i<MAXJOBS; i++)
    {
        if(jobList[i].status == 1)
        {
            return i;
        }
    }
    return 0;
}
int getJobID(int pid) //find and return which job this pid belong to. If not found or pid is invalid, return -1.
{
    if(pid < 1)
        return -1;
    for(int i=0; i<MAXJOBS; i++)
    {
        if(jobList[i].pid == pid)
            return i;
    }
    return -1;
}

int availJobID()//return the first available job ID
{
    for(int i=0; i<MAXJOBS; i++)
    {
        if(jobList[i].pid == -1)
        {
            return i;
        }
    }
    return -1;
}

void int_handler(int sig) //???
{
    // printf("Proces %d received signal %d\n", getpid(), sig);
    return;

    // int olderrno = errno;
    // pid_t child_pid = jobList[fgCheckingWithID()].pid;
    // sigset_t mask_all, prev_all;

    // sigfillset(&mask_all);
    // sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    // if(child_pid != 0)
    // {
    //     kill(child_pid, SIGINT);
    //     printf("Killing process %d\n", child_pid);
    // }
    // sigprocmask(SIG_SETMASK, &prev_all, NULL);

    // errno = olderrno;
}

void deleteJob(int jobID) //delete the job using jobID by changing value to default
{
    if(jobID < 0 && jobID >= MAXJOBS) //within bound
    {
        return;
    }
    jobList[jobID].pid = -1; //to default value
    jobList[jobID].status = 0; //undefine status
    strcpy(jobList[jobID].commandLine, ""); //to empty string
}

int getJobIDForFBK()//get jobID (may return jobID or pid) for foreground, background, kill command
{
    if(argv[1] == NULL) //not exist
    {
        return -1;
    }

    if(argv[1][0] != '%') // pid
    { 
        if(getJobID(atoi(argv[1])) != -1)
        {
            return getJobID(atoi(argv[1]));
        }
        
    }
    else //job id
    { 
        // printf("argv[1]: %s \n", argv[1]);
        // printf("argv[1][1]: %d \n", argv[1][1]);
        // printf("argv[1] + 1: %d \n", argv[1]+1);

        int jobID = atoi(argv[1] + 1) - 1;
        // printf("jobID: %i \n", jobID);
        if(0 <= jobID && jobID < MAXJOBS && jobList[jobID].pid != -1)
        {
            return jobID;
        }
    }
}



void chld_handler(int sig) //need to test???
{
    // printf("Proces %d received signal %d\n", getpid(), sig);
    pid_t child_pid;
    int status;
    
    sigset_t mask_all, prev_all;

    sigfillset(&mask_all);

    while((child_pid = waitpid(-1, &status, WNOHANG|WUNTRACED)) > 0)
    {
       
        if(WIFSTOPPED(status)) //change current job' status if child stopped
        {
            sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
            jobList[getJobID(child_pid)].status = 3; //stop status
            printf("\nJob [%i] (%i) stopped by signal %i\n",getJobID(child_pid)+1,child_pid,WSTOPSIG(status));
            sigprocmask(SIG_SETMASK, &prev_all, NULL);
        }
        else if(WIFSIGNALED(status) || WIFEXITED(status)) //remove from jobList if child terminated normally or by a signal
        {
            
            sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
            deleteJob(getJobID(child_pid)); /* Delete the child from the job list */ 
            sigprocmask(SIG_SETMASK, &prev_all, NULL);
            printf("Child %d terminated with exit status %d\n", child_pid, WEXITSTATUS(status));
        }
        else
            fprintf(stdout, "%s\n", "waitpid error");
    }
}

void tstp_handler(int sig) 
{
    if(fgCheckingWithID() != -1) //exist a foreground process
    {
        jobList[fgCheckingWithID()].status = 3; //stopped status
        kill(jobList[fgCheckingWithID()].pid, SIGTSTP);
    }
}

void IORedirection(){
    // printf("in1\n");
    // for(int i=0; i<argc; i++)
    //     printf("%s \n", argv[i]);
    // printf("argc: %i", argc);

	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO; //mode for setting permission bits.

    for(int i = 1; i < argc; i++) //start from i=1 since the argv[0] is executable file that does not contain any symbol
    {
        if(i + 1 < argc && strcmp(argv[i+1], ""))  //check whether next index is out of bound or not, and whether the next element exists or not
        {
            if(!strcmp(argv[i], "<")) //input from file
            {
                int inputFile = open(argv[i + 1], O_RDONLY, mode);  //open the input file, inputFile is the input file descriptor
                dup2(inputFile, STDIN_FILENO);
                close(inputFile); //close file
            }
            else if(!strcmp(argv[i], ">")) //output to file
            {
                // printf("in1 > \n");
                // printf("%s", argv[i + 1]);
                // printf("in1 > \n");
                
                int outputFile = open(argv[i + 1], O_CREAT|O_WRONLY|O_TRUNC, mode); //open the output file, outputFile is the output file descriptor
                dup2(outputFile, STDOUT_FILENO);
                close(outputFile); //close file
            }
            else if(!strcmp(argv[i], ">>")) //append to output file
            {
                int outputFile = open(argv[i + 1], O_CREAT|O_WRONLY|O_APPEND, mode); //open the output file, and append the new content at the end of the file. outputFile is the ouput file descriptor
                dup2(outputFile, STDOUT_FILENO);
                close(outputFile); //close file
            }
        }
        
    }
}

int main()
{
    while (1) 
    {
        char cmdline[MAXLINE];
        int input = dup(STDIN_FILENO);
	    int output = dup(STDOUT_FILENO);
        signal(SIGINT, int_handler); 
        signal(SIGCHLD, chld_handler);
        signal(SIGTSTP, tstp_handler);
        //input 
        printf("prompt > ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
                exit(0);
        eval(cmdline);

        dup2(input, STDIN_FILENO);
		dup2(output, STDOUT_FILENO);
    }
}

void eval(char *cmdline)
{
    char buf[MAXLINE];
    int bg;
    pid_t pid;

    strcpy(buf, cmdline);
    bg = parseline(buf); //1: background; 0: foreground
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */

    if (!built_in_commands()) 
    {
        int jobID = availJobID();
        if(jobID == -1)
        {
            printf("No more empty space to add a job\n");
            return;
        }
        strcpy(jobList[jobID].commandLine, cmdline); //initialize

        int child_status; 
        if ((pid = fork()) == 0)  //child
        {  
            if (execv(argv[0], argv) < 0) 
            { 
                printf("%s: Command not found.\n", argv[0]);
                exit(0);
            }
        }
        else //parent
        {
            // // /* Parent waits for foreground job to terminate */
            if (!bg) //foreground
            {
                //how to let fail command not add in?
                jobList[jobID].pid = pid; //initialize
                jobList[jobID].status = 1; //initialize to foreground

                int child_status;
                pid_t wipd = waitpid(pid, &child_status, 0);
                if (wipd < 0)
                    printf("waitfg: waitpid error");  // unix_error("waitfg: waitpid error");

                if(WIFEXITED(child_status))
                {
                    printf("Child %d terminated with exit status %d\n", wipd, WEXITSTATUS(child_status));
                }
                else
                    printf("Child %d terminated abnormally\n", wipd);
                deleteJob(getJobID(wipd)); // Delete the child from the job list 
                
            } 
            else
            {
                jobList[jobID].pid = pid; //initialize
                jobList[jobID].status = 2; //initialize to background
                
            }

            
        }
        
        
    }

    return;

}



int built_in_commands() 
{
    int jobID = getJobIDForFBK(); //may return jobID or pid

    IORedirection(); // redirect stdin (<), stdout (>) or append (>>)
    
    if (!strcmp(argv[0], "quit"))//done
    {
        // reap all the child processes
        for(int i = 0; i < MAXJOBS; i++)
        {
            if(jobList[i].pid != -1)
            {
                kill(jobList[i].pid, SIGINT);
            }
	    }
        exit(0);
    }

    else if (!strcmp(argv[0], "&")) //done in parseline(), and will process background or foreground in eval()
        return 1;

    else if(!strcmp(argv[0], "cd")) //done
    {
        if(!strcmp(argv[1], "..")) //if tok is ".."
            chdir("..");
        else
        {
            chdir(argv[1]);  
        }
        return 1;
    }

    else if(!strcmp(argv[0], "pwd")) //done
    {
        char buffer[SIZE];
        getcwd(buffer, sizeof(buffer));
        printf("%s \n",buffer);
        return 1;
    }


    else if(!strcmp(argv[0], "jobs")) //done
    {
        for(int i=0; i<MAXJOBS; i++)
        {
            if(jobList[i].pid != -1)
            {
                printf("[%i] (%i) ", i+1, jobList[i].pid);
                //1:foreground; 2: background; 3: stopped; 0: undefine
                switch(jobList[i].status)
                {
                    // case 1: 
                    //     printf("foreground"); //foreground state
                    //     break;
                    case 2: 
                        printf("Running"); //background state
                        break;
                    case 3: 
                        printf("Stopped"); //stop state
                        break;
                    default:
                        printf("Others"); //include forground and undefine
                        break;
                }
                printf(" %s", jobList[i].commandLine);

            }
        }
         return 1;
    }

    else if(!strcmp(argv[0], "kill")) //done
    {
        // printf("\n in1 %i\n", jobID);
        if(jobID != -1)
        {
            if(jobID < 0 || jobID >= MAXJOBS) //convert pid to the jobID
            {
                printf("\n in2 %i\n", jobID);
                jobID = getJobID(jobID);
            }
            kill(jobList[jobID].pid, SIGKILL);
            // deleteJob(jobID); 
        }

         return 1;
    }

    else if(!strcmp(argv[0], "fg")) //done
    {
        if(jobID != -1 && (jobList[jobID].status == 2 || jobList[jobID].status == 3))  //background or stop
        {
            if(jobID < 0 || jobID >= MAXJOBS) //convert pid to the jobID
            {
                jobID = getJobID(jobID);
            }
            // printf("\nin1 %i", jobID);

            kill(jobList[jobID].pid, SIGCONT);

            jobList[jobID].status = 1; //initialize to foreground

            int child_status;
            pid_t wipd = waitpid(jobList[jobID].pid, &child_status, 0);
            if (wipd < 0)
                printf("waitfg: waitpid error");  // unix_error("waitfg: waitpid error");

            if(WIFEXITED(child_status))
            {
                printf("Child %d terminated with exit status %d\n", wipd, WEXITSTATUS(child_status));
            }
            else
            {
                printf("Child %d terminated abnormally\n", wipd);
            }

            deleteJob(getJobID(wipd)); // Delete the child from the job list 

           
        }


        return 1; 
    }

    else if(!strcmp(argv[0], "bg"))
    {
        if(jobID != -1 && jobList[jobID].status == 3) //if in stopped state
        {
            if(jobID < 0 || jobID >= MAXJOBS) //convert pid to the jobID
            {
                jobID = getJobID(jobID);
            }
            jobList[jobID].status = 2; //change to background state
            kill(jobList[jobID].pid, SIGCONT);
        }

        return 1;
    }



   return 0;
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







