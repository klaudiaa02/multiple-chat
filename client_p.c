/*************************************************************
* CLIENT connects to the server (defined in argv[1]) on the specified port
* (in argv[2]), writing the predefined word (in argv[3]).
* USAGE: >client <serveraddress> <port>
*************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdbool.h>

#define BUF_SIZE 1024
#define MAX_MSG_SIZE 2048
#define MAX_LINE_LENGTH 1024

void erro(char *msg);
//bool fileExists(const char *filename);
//char* read_last_line(FILE *file);



int main(int argc, char *argv[]) 
{
 char endServer[100];
 int fd;
 struct sockaddr_in addr;
 struct hostent *hostPtr;
 


 
 if (argc != 3) {
 printf("client <host> <port>\n");
 exit(-1);
 }
 strcpy(endServer, argv[1]);
 if ((hostPtr = gethostbyname(endServer)) == 0)
 erro("I couldn't get address");
 bzero((void *) &addr, sizeof(addr));
 addr.sin_family = AF_INET;
 addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
 addr.sin_port = htons((short) atoi(argv[2]));
 
 if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
erro("socket");
 if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
erro("Connect");
 
////////////////////////////////////////////////////////////////////////
 
char *buffer = NULL;
size_t size = 0;
ssize_t characters_read;
int nread = 0;
char buffer_rec[BUF_SIZE];

 
// READS THE MENU FOR THE FIRST TIME, ITS USING PRINTF TO SHOW THE MENU THE REST OF THE CODE, BUT SINCE ITS THE FIRST TIME CONNECTING TO THE SERVER, DOING A READ IS LIKE "KNOWING" IF THE SERVER IS SENDING AND THE CLIENT RECEIVING FINE 
nread = read(fd, buffer_rec, BUF_SIZE-1); // SIZE OF WHAT'S RECEIVED
buffer_rec[nread] = '\0';   // "CUTS" THE STRING TO ITS SIZE SO IT WONT HAVE CHARACTERS THAT WERE USED BEFORE IF THE BUFFER WAS LONGER THAN IT IS NOW 
printf("%s\n", buffer_rec);

while(1)
{
 //HERE WE WRITE TO THE SERVER THE COMMANDS WE WANT, FOR EXAMPLE '2' TO LOGIN
printf("-> ");
characters_read = getline(&buffer, &size, stdin);
write(fd, buffer, size);
buffer[characters_read] = '\0';

	// IF YOU WRITE OTHER NUMBER THAT IS NOT RECOGNIZED IN THE MENU
	if((strcmp(buffer, "1\n") != 0) && (strcmp(buffer, "2\n") != 0) && (strcmp(buffer, "3\n") != 0) && (strcmp(buffer, "4\n") != 0) && (strcmp(buffer, "clear\n") != 0))
	{
	printf("Unknown Command\n");
	}

	// JUST TO CLEAR THE SCREEN TO DELETE OLD MESSAGES
	if((strcmp(buffer, "3\n")) == 0 || (strcmp(buffer, "clear\n") == 0))
	{
	system("clear");
	printf("Welcome to ChatRC\nHere are some commands for you:\n1-register  2-login  3-clear  4-exit\n");
	}

	
////////////////////////////////////// EXIT //////////////////////////////////////
// THIS LEAVES THE SERVER
    if(strcmp(buffer, "4\n") == 0)
    {
    break;
    }
    
////////////////////////////////////// EXIT //////////////////////////////////////

////////////////////////////////////// REGISTER //////////////////////////////////////

    if(strcmp(buffer, "1\n") == 0)
    {
    

   
  // SENDS USERNAME   
    printf("\nUsername: ");    
characters_read = getline(&buffer, &size, stdin);
write(fd, buffer, size);
buffer[characters_read] = '\0';
   
  // SENDS PASSWORD
    printf("\nPassword: ");   
characters_read = getline(&buffer, &size, stdin);
write(fd, buffer, size);
buffer[characters_read] = '\0';
      
  // ANSWER FROM SERVER ABOUT THE REGISTER
nread = read(fd, buffer_rec, BUF_SIZE-1);
buffer_rec[nread] = '\0';

      //REGISTER WORKED
    if(strcmp(buffer_rec, "\nRegister successful") == 0)  
    {
    printf("%s\n\n", buffer_rec);  // DC
    }
    else
    {
    printf("\nRegister not successful --> Username already exists\n\n");
    }

       

}

////////////////////////////////////// REGISTER END //////////////////////////////////////


////////////////////////////////////// LOGIN ////////////////////////////////////////

    if(strcmp(buffer, "2\n") == 0)
    {
    


   char username[BUF_SIZE];
   
  // SENDS USERNAME   
    printf("\nUsername: ");    
characters_read = getline(&buffer, &size, stdin);
write(fd, buffer, size);
buffer[characters_read] = '\0';
strcpy(username, buffer);
  // SENDS PASSWORD
    printf("\nPassword: ");   
characters_read = getline(&buffer, &size, stdin);
write(fd, buffer, size);
buffer[characters_read] = '\0';
  
      
  // ANSWER FROM THE SERVER
nread = read(fd, buffer_rec, BUF_SIZE-1);
buffer_rec[nread] = '\0';
        
int PID = 0; // PROCESS ID FROM THE CLIENT INITIALIZED AS ZERO
        
        // HERE WE KNOW IF THE LOGIN WORKED
    	if(strcmp(buffer_rec, "\nLogin successful") == 0)  
    	{
    	
    	// SENDS THE PID TO THE SERVER
    	PID = getpid();	//GETS THE PID
    	sprintf(buffer, "%d", PID);   //SAVE INT PID VALUE IN THE BUFFER STRING
    	write(fd, buffer, strlen(buffer));      // TIMING 1 (JUST A DEBUG THING)
	
	
    	
    	
    	while(1) //WHILE MENU
    	{
    	
    	//CLEARS THE SCREEN
    	system("clear");
    	
	strtok(username, "\n"); // REMOVES \n
	printf("Hi %s, here are some commands for you:", username); // WHILE MENU, THE CLIENT COMES HERE IF THE LOGIN WAS SUCCESSFUL
	printf("\n5-chat  6-ban users  7-block words  8-logout\n");
	
	// WRITING TO THE SERVER THE COMMANDS FOR EXAMPLE 8 TO LOGOUT
	printf("-> ");
	characters_read = getline(&buffer, &size, stdin);
	write(fd, buffer, size);
	buffer[characters_read] = '\0';
	
	
	// CHAT
	if(strcmp(buffer, "5\n") == 0)
	{
	system("clear");
	printf("Online Users:\n");
	
	nread = read(fd, buffer_rec, BUF_SIZE-1);  // GETS THE ONLINE NAMES STRING, THAT WAS SENT BY THE SERVER AND "MADE" BY THE FUNCTION extract_names()
	buffer_rec[nread] = '\0';   // TIMING 2 (DEBUG, USED THIS TO KNOW WHEN THE SERVER AND THE CLIENT ARE AT THE "SAME PART" IN THE CODE)
	
	
    // DISPLAYS WHO'S ONLINE
    int i, j;
    int len_to_erase = strlen(username);

	// HERE WE WANT TO ERASE THE NAME OF THE CLIENT THAT IS ASKING FOR THE LIST, FROM THAT LIST SINCE IT DOESNT MAKE SENSE FOR HIM TO SHOW
	// THIS FOR GOES THROUGH EACH CHARACTER IN BUFFER_REC
    for (i = 0; buffer_rec[i] != '\0'; i++) {
            // HERE WE CHECK IF THE STRING IN DIFFERENT POSITIONS MATCH THE USERNAME AND HAS A SPACE NEXT TO IT
        if (strncmp(&buffer_rec[i], username, len_to_erase) == 0 && (buffer_rec[i + len_to_erase] == ' ' || buffer_rec[i + len_to_erase] == '\0')) {
        //IF IT FINDS A MATCH, SHIFTS THE CHARACTERS IN THE STRING TO "REPLACE" THE MATCHED USERNAME
            for (j = i; buffer_rec[j + len_to_erase] != '\0'; j++) {
                buffer_rec[j] = buffer_rec[j + len_to_erase + 1];
            }
            //FINISH THE STRING
            buffer_rec[j] = '\0';
            // HERE WE "PLACE" THE CHECKING POINT IN THE PLACE WHERE THE WORD THAT GOT ERASED WAS SO IT WONT SKIP ANYTHING
            i--;
        }
    }

	// IF THE ONLINE LIST IS EMPTY OR NOT
	if(strlen(buffer_rec) > 1)
	{	
	printf("%s\n", buffer_rec); // DONT COMMENT
	printf("Write the name of the user you want to chat with or 'exit' to leave\n");
	}
	else
	{
	printf("No one is online, type 'exit' to go to the menu\n");
	}

char user_to_chat[20] = "0";
	
		// CHAT WHILE
		while(1){
	int leave = 0;	// THIS VARIABLE IS USED TO EXIT THE WHILE CHAT IN CASE THE CLIENT LEAVES THE CHAT INTERFACE WHILE


		// SENDS TO THE SERVER THE USERNAME OF THE CLIENT TO CHAT WITH
		printf("-> ");
		characters_read = getline(&buffer, &size, stdin);
		write(fd, buffer, size);
		buffer[characters_read] = '\0';
		
		//HERE WE SAVE THE USERNAME TO CHAT WITH 
		strcpy(user_to_chat, buffer);
		strtok(user_to_chat, "\n");
	
		//THIS IS USED IF THE CLIENT WANTS TO LEAVE THE CHAT OPTION
		if(strcmp(buffer, "exit\n") == 0)
		{
		leave = 1;
		break;
		}
	
		// HERE THE CLIENT READS FROM THE SERVER IF THE USERNAME HE CHOSE IS INDEED ONLINE AND IF IT IS POSSIBLE TO CREATE A CHAT WITH AND GET IT'S SOCKET_ID FROM "online.txt" 
		nread = read(fd, buffer_rec, BUF_SIZE-1);
		buffer_rec[nread] = '\0';
	
		
		// HERE IS BECAUSE WE GOT A POSITIVE RESPONSE FROM THE SERVER AND THE USER IS ONLINE
		if(strcmp(buffer_rec, "1") == 0)
		{
		system("clear");
		strtok(username, "\n");
		printf("You are now chatting with: %s\n", user_to_chat);
		
		// HERE WE SEND TO THE SERVER THE CLIENT'S USERNAME SO IT CAN GET THE SOCKET_ID	FROM "online.txt"	 
		write(fd, username, strlen(username));
		buffer[characters_read] = '\0';
	
int num;
	
	//HERE EACH CLIENT IN THE CHAT WILL RECEIVE A ROLE, ONE WILL RECEIVE 1 AND WILL START WITH A "WRITE" AND THE OTHER WILL RECEIVE A 2 AND WILL START WITH A "READ"
	nread = read(fd, buffer_rec, BUF_SIZE-1);
	buffer_rec[nread] = '\0';
	num = atoi(buffer_rec);
	
	

	//printf("NUM VALUE: %d\n", num);    //DEBUG
	
	// CLEAR BOTH BUFFERS
	memset(buffer, 0, sizeof(buffer));
	memset(buffer_rec, 0, sizeof(buffer));
	
	while(1) // CHAT INTERFACE WHILE
	{


if(num == 1) //DIFFERENT ROLES - WRITE
{
	//printf("Enter message: ");
        printf("-> ");
	characters_read = getline(&buffer, &size, stdin);

        // Send message to server
        write(fd, buffer, strlen(buffer));

num = 2; //KEEP SWITCHING
} 

if(num == 2)  //DIFFERENT ROLES - READ
{

	
        // Read message from server        
        nread = read(fd, buffer_rec, BUF_SIZE);
        buffer_rec[nread] = '\0';

        printf("Answer: %s\n", buffer_rec);

num = 1; //KEEP SWITCHING
}
	
	
	
	
	
	
	} // END OF CHAT INTERFACE WHILE
	
	//LEAVE THE WHILE CHAT
	//
	if(leave == 1)
	{
	write(fd, "exit\n", 6);
	break;
	}
	
	
	} // END OF IF THERE IS AN ONLINE USER WITH THAT NAME
	
	
	
	// IF THERE'S NOT AN ONLINE USER WITH THAT NAME
	if(strcmp(buffer_rec, "0") == 0)
	{
	system("clear");
	strtok(buffer, "\n");
	printf("There is no user name called '%s' online right now, please try another name or write 'exit' to leave\n", buffer); 
	}
	
	
	} // END OF CHAT WHILE
	
	
	
	
	
	}
	
	
	// LOGOUT
	if(strcmp(buffer, "8\n") == 0)
	{
	system("clear");
	printf("Welcome to ChatRC\nHere are some commands for you:\n1-register  2-login  3-clear  4-exit\n"); //BACK TO THE FIRST MENU
	memset(username, 0, sizeof(username)); //RESET THE USERNAME
	break;
	}
	
	
	
	//BAN USERS, WRITES TO THE SERVER THE USERNAME TO BAN 
	if(strcmp(buffer, "6\n") == 0)
	{
	system("clear");
	printf("Username to ban: ");
	characters_read = getline(&buffer, &size, stdin);
	write(fd, buffer, size);
	buffer[characters_read] = '\0';
	
	}
	
	//BLOCK WORDS, WRITES TO THE SERVER THE WORD TO BLOCK FROM BEING SHOWN IN THE CHAT
	if(strcmp(buffer, "7\n") == 0)
	{
	system("clear");
	printf("Word to block: ");
	characters_read = getline(&buffer, &size, stdin);
	write(fd, buffer, size);
	buffer[characters_read] = '\0';
	
	}
	

	
	
    	}// END OF WHILE MENU
	
    	}// END OF IF LOGIN SUCCESSFUL
    	else //THIS ELSE IS IF THE LOGIN IS NOT SUCCESSFUL
    	{
    		if(strcmp(buffer_rec, "2") == 0) //HERE WE GOT THE SERVER RESPONSE FROM THE LOGIN PART, IF IT RECEIVES 2, THAN THE USER IS BANNED
    		{
    		printf("\nYOU ARE BANNED");
    		}
    		
    	printf("\nLogin not successful --> Username and password don't match\n\n");
    	}

    	
    


}

////////////////////////////////////// LOGIN END ////////////////////////////////////////






   
} //MAIN WHILE
 
close(fd); //CLOSES THE CONECTION TO THE SERVER


// MAIN END
}



// WORK IN PROGRESS
/*
char* read_last_line(FILE *file) {
    if (file == NULL) {
        fprintf(stderr, "Error: File pointer is NULL\n");
        exit(1);
    }
    
    char line[MAX_LINE_LENGTH];
    char *last_line = NULL;
    
    // Read lines until end of file
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        // If a line is read, store it
        if (last_line != NULL) {
            free(last_line);
        }
        last_line = strdup(line);
    }
    
    return last_line;
}
*/




// NO LONGER USED SINCE WE REMOVED THE FILE VERIFYING BY THE CLIENT BUT MAY BE USEFUL LATER
/*
bool fileExists(const char *filename) {
    if (access(filename, F_OK) != -1) {
        return true; // File exists
    } else {
        return false; // File doesn't exist
    }
}
*/



void erro(char *msg)
{
printf("Error: %s\n", msg);
exit(-1);
}
