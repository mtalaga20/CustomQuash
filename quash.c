

#include <stdio.h> //I/O
#include <stdlib.h> //exit
#include <unistd.h> 
#include <string.h> //str handling for parsing
#include <stdbool.h> //Simple bools
#include <errno.h>
#include <sys/wait.h> //Parent process waiting
#include <fcntl.h> //reading/writing with files



#define BUF_SIZE 512
#define MAX_LINE 100
#define MAX_PATHS 50



/* Function to add elements to array
*/
void addToArray(char** charArray, char* item){
	for(int i=0; i<sizeof(charArray); i++){
		if(charArray[i] == NULL){
			charArray[i] = item;
			break;}
		i++;
	}
}

/* Function to check PATH array
*/
bool checkPath(char* PATH, char** PATH_LIST){
	for (int i=0; i<sizeof(PATH_LIST); i++){ //Checking if path is in path array
		if(access(PATH_LIST[i], F_OK) == 0){		
			strcpy(PATH, PATH_LIST[i]);		
			return true;}
		else{
			printf("Executable not found\n");
			return false;} //Message if executable not in path
	}


}

/* Handles and executes special commands
*/
int specialcommands(char** line, char* HOME, char** PATHS){
	//char* checker;
	char* sc[11];
	sc[0] = "exit\n"; sc[1] = "quit\n"; sc[2] = "jobs\n"; sc[3] = "jobs";
	sc[4] = "cd\n"; sc[5] = "cd"; sc[6] = "set"; sc[7] = "kill\n"; 
	sc[8] = "kill";  sc[9] = "pwd\n"; sc[10] = NULL; //sc[7] = "set\n";
	
	int i = 0;
	int answer = -1;
	while (i < (sizeof(sc) - 1)){
		//printf("|%s", line[0]);
		if (sc[i] == NULL){break;}
		else if(strcmp(line[0], sc[i]) == 0){
			answer = i;
			//printf("answer: %d", answer);
			break;
		}	
		else {i++;}
	}
	
	if (answer == 0 || answer == 1){ //exit and quit
		printf("Exiting...\n");
		exit(0);
		}
	else if (answer == 6 || answer == 7){ //path and home
		int j = 0;
		char* separated[2];
		printf("%s", line[1]);
		while((separated[j] = strsep(&line[1], "=")) != NULL){
			j++;}
	
		if (strcmp(separated[0], "HOME") == 0){
			//system(line[1]);
			putenv(line[1]);
			HOME = line[1];
			printf("Home changed to %s", separated[1]);
		}
		else if(strcmp(separated[0], "PATH") == 0){
			//path stuff
			char* pathCollection[MAX_PATHS];
			//parse(separated[1], pathCollection, ":"); // put each path in path collection
			//for(int i=0; i<sizeof(pathCollection); i++){		 
				//if(pathCollection[i] == NULL){
				//	break;}	
				//strtok(pathCollection[i], "\n");		
				addToArray(PATHS, pathCollection[i]);
				
			//}
			//strcat(PATH, separated[1]);
			printf("Path to include %s", separated[1]);	
		}
	}
	
	else if (answer == 4 || answer == 5){ //cd
		if (line[1] == NULL){
		  printf("\nMoving back to HOME:%s", HOME);
		chdir(HOME);
		}
		else{
			strtok(line[1], "\n");
			chdir(line[1]);
		}
	}
	else if (answer == 7 || answer == 8){
		//kill( ,SIGKILL);

	}
	else if(answer == 9){ //pwd
		char cwd[BUF_SIZE];
		getcwd(cwd, sizeof(cwd));
		printf("\n%s", cwd);
	}
	
	return answer;
}

/* Counts # of occurences of a specified char*
*/
int countOccurences(char* cmd, char** checkArray, char* divider){
	int i = 0;
	int countOfOccurences = 0; //returns int of separated sections	
	while((checkArray[i] = strsep(&cmd, divider)) != NULL){
			//printf("Got a pipe %s", pipeLine[i]);
			countOfOccurences += 1;
			i++;
	}	
	return (countOfOccurences - 1);
}


/* Parse a sentence into an array split by a specified divider
*/
void parse(char* sentence, char** newCollection, char* divider){ //consider adding char
	int i = 0;	
	while((newCollection[i] = strsep(&sentence, divider)) != NULL){
			i++;
		}

}

//Validity check for input
bool checkLine(char* input){
	
	if(strlen(input) <= 0){
		return false;
	}
	else{
		return true;
	}
}

/* Organization to find quash related operators like pipe or <>
*/
int find(char* cmd, char** line, char** pipedcommand, char* HOME, char* PATH, char** PATHS, bool changeOUT, bool changeIN){
	if (checkLine(cmd) == false){return 99;}

	//check for <, >, |
	int checkIn = countOccurences(cmd, pipedcommand, "<");
	int checkOut = countOccurences(cmd, pipedcommand, ">");	
	if (checkIn == 1){
		changeIN = true; 
		return -1;}
	
	if (checkOut == 1){
		changeOUT = true;
		//parse(cmd, line, " "); //May need to parse based on >
		return 2;}

	int pipeCount = countOccurences(cmd, pipedcommand, "|");
	if (pipeCount > 0){
		//printf("%s", pipedcommand[1]);
		return 1;}
	
	else{
		parse(cmd, line, " ");
		//check for set/home/cd
		int special = specialcommands(line, HOME, PATHS);
		//printf("Special %d", special);
		if ((special == -1)){ //-1 returned if command was not found in special
			return 0; //return to execute command
		} 
		else if ((special == 2 || special == 3)){
			return 3;
		}
		else {
			return 99; //to get new input
		}	
	}
}




/* Execute command with and without args
*/
void execCommand(char** command, char* PATH, char**PATHS, bool changeOUT, char* file){
	
	int status;
	char executePATH[MAX_PATHS]; //execute path so executing DN permanently change path
	char userPATH[MAX_PATHS] = "/usr/bin/";	
	char finalPATH[MAX_PATHS];
	bool foundPATH = false;		
	strcpy(executePATH, PATH);
	strcat(executePATH, command[0]);
	strcat(userPATH, command[0]);
	strtok(executePATH, "\n"); strtok(userPATH, "\n"); //Remove \n
	if(access(executePATH, F_OK) == 0){		
		foundPATH = true;
		strcpy(finalPATH, executePATH);	
		addToArray(PATHS, executePATH);
	}
	else if(access(userPATH, F_OK) == 0){
		foundPATH = true;
		strcpy(finalPATH, userPATH);
		addToArray(PATHS, userPATH);	
	}
	else if(checkPath(finalPATH, PATHS)){
		foundPATH = true;
	}

	char buf[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	
	//printf("%ld - %s - %s", sizeof(command), command[0], command[1]);

	if(command[1] != NULL){ //Ensure not reading from file
		for(int i=1; i< sizeof(command); i++){
			if(command[i] == NULL){break;}
			else{
				strcat(buf, command[i]);
			}
		}
	}
	
	
	if(foundPATH || command[1] == NULL){
		
		if(command[1] == NULL){ //Exec for command without args
			strcpy(buf, executePATH);
			strcpy(finalPATH, executePATH);
		}		
		strtok(buf, "\n"); //Remove \n
		pid_t pid_1 = fork(); //child process to execute	
		if (pid_1 < 0){
			printf("Failed command - forking\n");
		}
		else if (pid_1 == 0){
			if(changeOUT == true){ //to file given output redirection
				int wfd;
				printf("Printing to text output\n");
				if (wfd = open(file, O_WRONLY | O_CREAT, 0777) >= 0){
					dup2(wfd, STDOUT_FILENO);
					//dup2(wfd, STDERR_FILENO);
					//printf("Printing to text output");
				}
				else{printf("Error redirecting file\n");}
			}
	
			
			
			//printf("|%s|%s|", finalPATH, buf);
			if (execl(finalPATH, buf, NULL) < 0){
				printf("Error Executing");
				
				exit(0);
			}
		}	
		else{
			if ((waitpid(pid_1, &status, 0)) == -1) {
    				fprintf(stderr, "Process 1 encountered an error. ERROR%d\n", errno);
    				return;
  			}		
		}
	}	
}

/* Execute piped command
*/
int execPiped(char** commandPiped, char* PATH, char**PATHS, bool changeOUT, bool changeIN){
	char* first_section[MAX_LINE]; char* second_section[MAX_LINE];
	int status;
	//Parse each section of pipe command
	parse(commandPiped[0], first_section, " "); parse(commandPiped[1], second_section, " ");
	//For 1st section - PATHS
	char executePATH[MAX_PATHS]; //execute path so executing DN permanently change path
	char finalPATH[MAX_PATHS];	
	strcpy(executePATH, PATH);
	strcat(executePATH, first_section[0]);
	strtok(executePATH, "\n");  //Remove \n
	addToArray(PATHS, executePATH);
	//2nd section PATHS
	char executePATHTWO[MAX_PATHS]; //execute path so executing DN permanently change path
	char finalPATHTWO[MAX_PATHS];	
	strcpy(executePATHTWO, PATH);
	strcat(executePATHTWO, second_section[0]);
	strtok(executePATHTWO, "\n"); //Remove \n
	addToArray(PATHS, executePATHTWO);

	bool foundPATH, foundPATHTWO = false;
	

	if(access(executePATH, F_OK) == 0){		
		foundPATH = true;
		strcpy(finalPATH, executePATH);	
		addToArray(PATHS, executePATH);
	}

	if(access(executePATHTWO, F_OK) == 0){		
		foundPATHTWO = true;
		strcpy(finalPATHTWO, executePATHTWO);	
		addToArray(PATHS, executePATHTWO);
	}

	char buf[BUF_SIZE]; char bufTWO[BUF_SIZE];
	bzero(buf, BUF_SIZE); bzero(buf, BUF_SIZE);
	if(first_section[1] != NULL){ //Ensure not reading from file
		for(int i=0; i< sizeof(first_section); i++){
			if(first_section[i] == NULL){break;}
			else{
				strcat(buf, first_section[i]);
			}
		}
	}
	if(second_section[1] != NULL){ //Ensure not reading from file
		for(int i=0; i< sizeof(second_section); i++){
			if(second_section[i] == NULL){break;}
			else{
				strcat(bufTWO, second_section[i]);
			}
		}
	}

	if(foundPATH && foundPATHTWO){
	
		if(first_section[1] == NULL){ //Exec for command without args
			strcpy(buf, executePATH);
			strcpy(finalPATH, executePATH);
		}
		if(second_section[1] == NULL){ //Exec for command without args
			strcpy(bufTWO, executePATHTWO);
			strcpy(finalPATHTWO, executePATHTWO);
		}
		strtok(buf, "\n"); strtok(bufTWO, "\n");		

		pid_t pid_1, pid_2;
		int p1[2];
		
		if(pipe(p1) < 0){printf("Problem creating pipe\n"); return -1;}

		pid_1 = fork();
		if (pid_1 < 0){
			printf("Failed command - forking\n");
			return -1;
		}
		else if (pid_1 == 0){
			close(p1[0]);
			dup2(p1[1], STDOUT_FILENO);
			close(p1[1]);
			
			if (execl(finalPATH, buf, NULL) < 0){
				printf("Error Executing1\n");
				exit(0);
			}
			
		}	
		pid_2 = fork();		
		if (pid_1 < 0){
			printf("Failed command - forking\n");
			return -1;
		}
		else if (pid_2 == 0){
			close(p1[1]);
			dup2(p1[0], STDIN_FILENO);
			close(p1[0]);

			if (execl(finalPATHTWO, bufTWO, NULL) < 0){
				printf("Error Executing2\n");
				exit(0);
			}
			
		}
		else{
			  if ((waitpid(pid_1, &status, 0)) == -1) {
    				fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    				return -1;}
  
  
			  if ((waitpid(pid_2, &status, 0)) == -1) {
			    fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
			    return -1;
			  }
			
		}
	}
	else{printf("PATH not found.\n");}
}

/* Implements certain functions based on what was found in the user command or file
*/
int Exec(char* cmd, char** line, char** pipedcommand, char* PATH, char** all_paths, bool changeOUT, bool changeIN, int indicator, char** first_half){
	char* bgcommand[4];	
	switch(indicator){
		case 0: //Regular command (with and wthout args)
			//printf("PATH:%s / path %s", paths, bin);
			//printf("\n%s", paths);
			//if(! changeOUT){printf("CHANGEOUT", changeOUT);}
			execCommand(line, PATH, all_paths, changeOUT, NULL);
			break;		
		//Command with single pipe
		case 1: 
			execPiped(pipedcommand, PATH, all_paths, changeOUT, changeIN);
			break;

		//Command with > 
		case 2: 
			changeOUT = true;
			parse(pipedcommand[0], first_half, " ");
			strtok(pipedcommand[1], "\n");
			execCommand(first_half, PATH, all_paths, changeOUT, pipedcommand[1]);
			break;
		//Background/foreground processes
		case 3:
			bgcommand[0] = "bg"; bgcommand[1] = " %"; bgcommand[2] = " 1"; bgcommand[2] = NULL;
			execCommand(bgcommand, PATH, all_paths, false, NULL);
		//Command with <			
		case -1: 
			changeIN = true;			
			//execCommand(line, PATH, all_paths, changeOUT, "testy.txt");
			return -1;
	}




}


int main(int argc, char **argv, char **envp){

 printf("Welcome to the Quash\n");

 char* user = getenv("USER");
 char command[BUF_SIZE];
 char* commandArgs[MAX_LINE]; char* first_half[MAX_LINE];
 char* pipedcommand[MAX_LINE];
 char* HOME;
 char* all_paths[MAX_LINE];
 char* bin = "/bin/";
 char paths[MAX_PATHS];
 strcpy(paths, bin);
 bool changeOUT = false; //set to true if output is redirected
 bool changeIN = false;

 char cwd[BUF_SIZE];
 getcwd(cwd, sizeof(cwd));
 HOME = cwd;

 addToArray(all_paths, paths);
 addToArray(all_paths, "usr/bin");

 printf("Username: %s", user);

 char file[MAX_LINE]; //Used for file input
 //size_t rsize;
 //bool read = false;
 
while(true){
	getcwd(cwd, sizeof(cwd));
	printf("\n%s $", cwd);
	//fflush(stdout);	
	//read input
	if(!changeIN){
		fgets(command, MAX_LINE, stdin);
		int numberOfPipes = find(command, commandArgs, pipedcommand, HOME, paths, all_paths, changeOUT, changeIN);
		Exec(command, commandArgs, pipedcommand, paths, all_paths, changeOUT, changeIN, numberOfPipes, first_half);
		if (numberOfPipes == -1){changeIN = true;}
	}
	else{ //Code for < alterations. Would read each line in file and run it through the switch below
		//if(!read){
		FILE *rfd = fopen("test.txt", "r");
		//read = true;	
		while(fgets(command, MAX_LINE, rfd) != NULL){
			int read_case = find(command, commandArgs, pipedcommand, HOME, paths, all_paths, changeOUT, changeIN);
			Exec(command, commandArgs, pipedcommand, paths, all_paths, changeOUT, changeIN, read_case, first_half);
					
		}
		fclose(rfd);
		changeIN = false;
		
	}		
		//else{printf("Error finding file\n");}	
  	}
return 0;
}
