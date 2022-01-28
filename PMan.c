#include <unistd.h>     // fork(), execvp()
#include <stdio.h>      // printf(), scanf(), setbuf(), perror()
#include <stdlib.h>     // malloc()
#include <sys/types.h>  // pid_t 
#include <sys/wait.h>   // waitpid()
#include <signal.h>     // kill(), SIGTERM, SIGKILL, SIGSTOP, SIGCONT
#include <errno.h>      // errno
#include <string.h>     // define,string processing,etc
#include "linkedlist.h"

#define BACKGROUND "&"
#define CMD_BG "bg"
#define CMD_BGLIST "bglist"
#define CMD_BGKILL "bgkill"
#define CMD_BGSTOP "bgstop"
#define CMD_BGCONT "bgstart"
#define CMD_PSTAT "pstat"
#define PROMPT "PMan: > "
#define MAX_INPUT_ITEM 15
#define MAX_INPUT_ITEM_LEN 50
#define MAX_STAT 7




process_list_t *input_handle(char *input,process_list_t *p_list);
process_list_t *bg_entry(char *input_list[],int index,process_list_t *p_list);
void bglist_entry(process_list_t *p_list);
process_list_t *bgsig_entry(process_list_t *p_list, char *input_list[], char *flag);
void pstat_entry(pid_t pid);

process_list_t *check(process_list_t *p_list, pid_t pid);
process_list_t *watching(process_list_t *p_list);

/*
 * Recieve command from user, if it matches with the pre-defined command,
 * call the corresponding function.
 */
process_list_t *input_handle(char *input,process_list_t *p_list){
    char *temp;
    char *input_list[MAX_INPUT_ITEM]={NULL};
    char *flag;
    char input_list_src[MAX_INPUT_ITEM][MAX_INPUT_ITEM_LEN];
    int index=0;
    
    /* parse the input cmd (e.g., with strtok)
    */
    memset(input_list_src, '\0', sizeof(input_list_src)); // initialize input_list_src[] with '\0'

    temp=strtok(input," ");
    flag=temp;
    while((temp=strtok(NULL, " ")) && index < MAX_INPUT_ITEM){
        strncpy(input_list_src[index], temp, strlen(temp));
        input_list[index]=input_list_src[index];
        index++;
    }
    
    if ((strcmp(flag,CMD_BG) == 0) && (index != 0)){
        p_list = bg_entry(input_list,index,p_list);
    } else if ((strcmp(flag, CMD_BGLIST) == 0) && (index == 0)) {
        bglist_entry(p_list);
    } else if ((strcmp(flag, CMD_BGKILL) == 0 || strcmp(flag, CMD_BGSTOP) == 0 || strcmp(flag, CMD_BGCONT) == 0) && (index == 1)) {
        p_list = bgsig_entry(p_list, input_list, flag);
    } else if ((strcmp(flag, CMD_PSTAT) == 0) && (index == 1)) {
        char *str;
        pid_t pid;
        int found = 0;

        pid = (int)strtol(input_list[0], &str, 10); // note: strtol converts string to long int
        if ((found = find_process(p_list, pid))){
            pstat_entry(pid);
        }
        else{
            printf("Procces %d does not exist\n", pid);
        }
    } else {
        printf("%s: Please enter the command in correct format!\n",flag);
    }

    return p_list;
}


process_list_t *bg_entry(char *input_list[],int index,process_list_t *p_list){
	
	pid_t pid;
    int p_status;
    
	pid = fork();
	if(pid == 0){
		if(execvp(input_list[0], input_list) < 0){
			perror("Error on execvp");
            exit(-1);
		}
		exit(EXIT_SUCCESS);
	}
	else if(pid > 0) {

        int retVal;
	
		retVal = waitpid(pid, &p_status, WNOHANG | WUNTRACED);
		if (retVal == -1) { 
			perror("Fail at waitpid"); 
			exit(-1); 
		}
        
        if(!WEXITSTATUS(p_status) || !WIFEXITED(p_status)){
            char *temp1=NULL;     // a constant to hold the link
            char temp2[MAX_INPUT_ITEM_LEN]={"\0",};  // intermediate variable
            char p_path[MAX_INPUT_ITEM_LEN]={"\0",};  // process address
            sprintf(temp2,"/proc/%d/exe",pid);
            temp1=temp2;
            readlink(temp1,p_path,MAX_INPUT_ITEM_LEN);
            p_list=add_front(p_list,p_path,pid);
        }

    }
	else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
    
    return p_list;
}


void bglist_entry(process_list_t *p_list){
    process_list_t *temp = NULL;
    int total=0;
    
    char *temp1;     // a constant to hold the link
    char temp2[MAX_INPUT_ITEM_LEN]={"\0",};  // intermediate variable
    char p_path[MAX_INPUT_ITEM_LEN]={"\0",};  // process address


    for(temp = p_list;temp != NULL;temp=temp->next){
        sprintf(temp2,"/proc/%d/exe",temp->pid);
        temp1=temp2;
        readlink(temp1,p_path,MAX_INPUT_ITEM_LEN);
        temp1 = p_path;
        strncpy(temp->path,temp1,strlen(temp1)+1);
        
        printf("%d:  %s\n",temp->pid,temp->path);
        total++;
    }
    
    printf("Total background processes: %d\n",total);
}

/*
 * Handel the "bgkill","bgstop","bgstart" commands. Use fork() create
 * child process to excute the shell "kill" command with different signal.
 */
process_list_t *bgsig_entry(process_list_t *p_list, char *input_list[], char *flag){
    pid_t pid;
    pid_t target_pid;
    //int p_status;
    int found = 0;
    char *str;
    
    target_pid = (int)strtol(input_list[0],&str,10); // note: strtol converts string to long int
    found = find_process(p_list,target_pid);
    
    if(!found){
        printf("Procces %d does not exist\n",target_pid);
        return p_list;
    }
    
    if(strcmp(flag,CMD_BGKILL) == 0){
        char *argv_execvp[] = {"kill","-9",input_list[0],NULL};
        
        pid = fork();
        if(pid == 0){
            if(execvp(argv_execvp[0], argv_execvp) < 0){
                perror("Error on execvp");
                exit(-1);
            }
        }
        else if(pid > 0) {
            // no code here yet
        }
    }
    else if(strcmp(flag,CMD_BGSTOP) == 0){
        char *argv_execvp[] = {"kill","-19",input_list[0],NULL};
        
        pid = fork();
        if(pid == 0){
            if(execvp(argv_execvp[0], argv_execvp) < 0){
                perror("Error on execvp");
                exit(-1);
            }
        }
        else if(pid > 0) {
            // no code here yet
        }
    }
    else if(strcmp(flag,CMD_BGCONT) == 0){
        char *argv_execvp[] = {"kill","-18",input_list[0],NULL};
        
        pid = fork();
        if(pid == 0){
            if(execvp(argv_execvp[0], argv_execvp) < 0){
                perror("Error on execvp");
                exit(-1);
            }
        }
        else if(pid > 0) {
            // no code here yet
        }
    }
    
    return p_list;
}

/*
 * Handle the "pstat" command. Read process status information 
 * from "/proc/<pid>/stat" and "/proc/<pid>/status" with the pid
 * that passed in
 */
void pstat_entry(pid_t pid){
    char comm[MAX_INPUT_ITEM_LEN] = {"\0",} , state[5] = {"\0",};
    unsigned long int utime = 0 , stime = 0;
    long rss = 0;
    int voluntary_ctxt_switches = 0, nonvoluntary_ctxt_switches = 0;
    char *fpath = NULL;
    char temp[MAX_INPUT_ITEM_LEN] = {"\0",};

    FILE *fp = NULL;

    // read from /proc/<pid>/stat
    sprintf(temp, "/proc/%d/stat", pid);
    fpath = temp;

    fp = fopen(fpath, "r");

    if (fp){

        fscanf(fp, "%*s %*[(]%[^)]) %s %*s %*s %*s %*s %*s %*s %*s \
            %*s %*s %*s %lu %lu %*s %*s %*s %*s %*s \
            %*s %*s %*s %ld",comm, state, &utime, &stime, &rss);

    }

    fclose(fp);

    // read from /proc/<pid>/status
    sprintf(temp, "/proc/%d/status", pid);
    fpath = temp;

    fp = fopen(fpath, "r");

    if (fp){
        char line[100] = {"\0",};
        char *str = NULL;

        while(fgets(line,100,fp )){
            if (strncmp(line,"volun",5) == 0){

                if(line[strlen(line)-1] == '\n'){
                    line[strlen(line) - 1] = '\0';
                }

                str = line;
                sscanf(str, "%*[^0-9]%d", &voluntary_ctxt_switches);
            }
            else if (strncmp(line, "nonvo", 5) == 0){

                if (line[strlen(line) - 1] == '\n'){
                    line[strlen(line) - 1] = '\0';
                }

                str = line;
                sscanf(str, "%*[^0-9]%d", &nonvoluntary_ctxt_switches);
            }
        }
    }

    //print out the process status information
    printf("Comm:                       %s\n", comm);
    printf("State:                       %s\n", state);
    printf("Utime:                       %lu ms\n", utime * 10);
    printf("Stime:                       %lu ms\n", stime * 10);
    printf("RSS:                        %ld\n", rss);
    printf("voluntary_ctxt_switches:     %d\n", voluntary_ctxt_switches);
    printf("nonvoluntary_ctxt_switches:  %d\n", nonvoluntary_ctxt_switches);

    fclose(fp);
}

/*
 * Monitoring the background of any processe that executed in 
 * PMan and reporting the state of them (did not use)
 */
process_list_t *check(process_list_t *p_list, pid_t pid){
    int p_status;
    pid_t target_pid;
    
    target_pid = waitpid(pid,&p_status,WNOHANG | WUNTRACED | WCONTINUED);
    if(target_pid > 0){
        if(WIFSTOPPED(p_status)){
            printf("Procces %d stopped\n", target_pid);
        }
        else if(WIFCONTINUED(p_status)){
            printf("Procces %d continued\n", target_pid);
        }
        else if(WIFSIGNALED(p_status)){
            printf("Procces %d was killed\n", target_pid);
            p_list = remove_process(p_list,target_pid);
        }
        else if(WIFEXITED(p_status)){
            printf("Procces %d was exited\n", target_pid);
            p_list = remove_process(p_list,target_pid);
        }       
    }
    
    return p_list;
}

/*
 * Monitoring the background of any processe that executed in 
 * PMan and reporting the state of them
 */
process_list_t *watching(process_list_t *p_list){
    pid_t pid;
    int p_status;
    int found = 0;
    
    pid = waitpid(-1,&p_status,WNOHANG | WUNTRACED | WCONTINUED);
    if(pid > 0){
        if(!(found = find_process(p_list,pid))){
            return p_list;
        }
        
        if(WIFSTOPPED(p_status)){
            printf("Procces %d stopped\n", pid);
        }
        else if(WIFCONTINUED(p_status)){
            printf("Procces %d continued\n", pid);
        }
        else if(WIFSIGNALED(p_status)){
            printf("Procces %d was killed\n", pid);
            p_list = remove_process(p_list,pid);
        }
        else if(WIFEXITED(p_status)){
            printf("Procces %d was exited\n", pid);
            p_list = remove_process(p_list,pid);
        }       
    }
    
    return p_list;
}

/*
 * Repeatedly prompt the "PMan: > " and read input from user
 */
int main(){
    char input[MAX_INPUT_ITEM_LEN];
    process_list_t *p_list=NULL;

	for(;;){
        memset(input,'\0',sizeof(char)*MAX_INPUT_ITEM_LEN);
		printf("%s", PROMPT);

        p_list = watching(p_list);

        fgets(input, MAX_INPUT_ITEM_LEN, stdin);
        
        //p_list=watching(p_list);
        
        if((strlen(input)-1) <= 0){  // do nothing when input is empty
            continue;
        }
		
        if(input[strlen(input)-1]=='\n'){  // eliminate the '\n' for each input
            input[strlen(input)-1]='\0';
        }
        
		if (strcmp(input,"exit") == 0) {  // when enter "exit", terminates program
			return 0;
		}
        
        p_list=input_handle(input,p_list);
		
	}
	return 0;
}

/*
void check_zombieProcess(void){
	int status;
	int retVal = 0;
	
	while(1) {
		usleep(1000);
		if(headPnode == NULL){
			return ;
		}
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			//remove the background process from your data structure
		}
		else if(retVal == 0){
			break;
		}
		else{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
	}
	return ;
}*/