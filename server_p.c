/*******************************************************************
* SERVER on port 9000, listening for new customers. when they arise
* new customers, the data sent by them is read and downloaded on the screen.
*******************************************************************/
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
#include <pthread.h>

#define PORT 9000
#define BUF_SIZE 1024
#define MAX_CLIENTS 20
#define MAX_IP_LENGTH 	16 // Including null terminator
#define FILENAME "online.txt"
#define MAX_LINE_LENGTH 100

struct client_info 
{
	char user[BUF_SIZE];
	int PID;
	int socket_id;
	char ip_address[MAX_IP_LENGTH];
};


void process_client(int client_fd, int sem_id, char ip_addr[]);
void error(char *msg);
int signup(char *username, char *password);
int login(char *username, char *password);
void menu(int client_fd, char *username);
void remove_line(const char username[BUF_SIZE]);
char* extract_names();
int isWordInString(char str[], char word[]);
int getPIDByUsername(char username[20]);
int get_socket_by_name(char username[20]);
int get_socket(int line_number);
int checkWord(char buffer[BUF_SIZE]);
void replaceBlockedWord(char *sentence);


int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address, client_addr;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    int client[10];
    char ip_addr[MAX_IP_LENGTH];

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 9000
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 9000
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int client_sockets[MAX_CLIENTS] = {0};
    int num_clients = 0;


// Create semaphore
    	key_t sem_key = ftok("server_p.c", 'R');
    	int sem_id = semget(sem_key, 1, 0666 | IPC_CREAT);
   	if (sem_id == -1)
        	error("semget");

	semctl(sem_id, 0, SETVAL, 1);

    // Clears the file since when the server is initiated, there's no on online
    FILE *file = fopen("online.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    fclose(file);


// removes the file where the server saves identifications of the clients in a chat,
//
// DEPOIS DE FALAR NA AULA COM O PROFESSOR, DECIDI FAZER UMA ALTERAÇAO DE MODO A QUE ESTE FICHEIRO NAO SEJA NECESSARIO PARA O CLIENTE, FAZENDO-O GUARDAR APENAS VARIAVEIS PARA O SERVIDOR LER QUE ESTÁ EXPLICADO MAIS A FRENTE.
//
remove("ver.txt");




////////////////////////////////////////////////////////////////////////
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
	if (new_socket > 0) {
        
        // Add new client socket to the socket array
        client_sockets[num_clients++] = new_socket;
        
        strncpy(ip_addr, inet_ntoa(client_addr.sin_addr), MAX_IP_LENGTH);

	if (fork() == 0) 
 	{
 	//WE KEEP CHANGING THE SOCKET_ID's PF THE CLIENTS, IT STARTS WITH 4 BEING ASSIGNED TO THE FIRST CLIENT THAT CONNECTS AND SO ON
 	process_client(3+num_clients, sem_id, ip_addr);
 	exit(0);
 	}

    	}
    }




return 0;
}


			
void process_client(int client_fd, int sem_id, char ip_addr[])
{
int nread = 0;
char buffer[BUF_SIZE];
char buffer_rec[BUF_SIZE];

// SENDS THE OPTION MENU TO THE CLIENT
strcpy(buffer, "Welcome to ChatRC\nHere are some commands for you:\n1-register  2-login  3-clear  4-exit\n");
write(client_fd, buffer, sizeof(buffer) - 1);

while(1)
{
// WAITS FOR COMMANDS, FOR EXAMPLE '1' TO REGISTER
nread = read(client_fd, buffer, BUF_SIZE-1); 
buffer[nread] = '\0';

char username[BUF_SIZE];
char password[BUF_SIZE];


////////////////////////////////////// REGISTER //////////////////////////////////////

	if(strcmp(buffer, "1\n") == 0)
	{


	//GETS THE USERNAME
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	strcpy(username, buffer);

	//GETS THE PASSWORD
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	strcpy(password, buffer);
	
	// VALUE THAT WILL DEFINE IT THE REGISTER WAS SUCCESSFUL OR NOT
	int value_reg = signup(username, password);

		
		if(value_reg == 0)
		{
		write(client_fd, "\nRegister not successful --> Username already exists", strlen("\nRegister not successful --> Username already exists"));
		}
	
		if(value_reg == 1)
		{
		write(client_fd, "\nRegister successful", strlen("\nRegister successful"));
		}
	
	
	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	
	}//fim if register
	
////////////////////////////////////// REGISTER //////////////////////////////////////


////////////////////////////////////// LOGIN //////////////////////////////////////
	if(strcmp(buffer, "2\n") == 0)
	{

	memset(username, 0, sizeof(username));
	memset(password, 0, sizeof(password));
	
	//USERNAME
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	strcpy(username, buffer);


	//PASSWORD
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	strcpy(password, buffer);
	

	//VARIABLE TO CHECK THE SUCCESS OF THE LOGIN
	int value_log = login(username, password);
	int PID = 0;
	
	//THIS MEANS THAT THE USER IS BANNED FROM THE SERVER
	if(value_log == 2)
	{
	write(client_fd, "2", strlen("2"));
	}
	
	if(value_log == 0)
	{
        write(client_fd, "\nLogin not successful --> Username and password don't match", strlen("\nLogin not successful --> Username and password don't match"));
	}
	
	if(value_log == 1)
	{
	write(client_fd, "\nLogin successful", strlen("\nLogin successful"));
	
		// SO WE HAVE THE NAME WITHOUT THE \n
		strtok(username, "\n");
	
		//HERE WE GET THE PID FROM THE CLIENT
		nread = read(client_fd, buffer, BUF_SIZE-1);
		buffer[nread] = '\0';      // TIMING 1
		PID = atoi(buffer);	
		
		
		// FILE TO KEEP THE INFO ABOUT THE ONLINE USERS
		FILE *online = fopen("online.txt", "a");
	
		if (online == NULL) {
        	perror("Erro ao abrir o arquivo");
        	}
	
	
		// Lock semaphore before accessing file
        	struct sembuf sem_lock = {0, -1, SEM_UNDO};
        	if (semop(sem_id, &sem_lock, 1) == -1) 
       			error("semop");
                
        	// Open file for writing
        	int fd;
        	if ((fd = open("online.txt", O_WRONLY | O_APPEND | O_CREAT, 0666)) == -1)
            		error("open");
	
		//VARIABLES THAT ARE THEN SAVED IN THE "online.txt"
		struct client_info new_client;
  		new_client.socket_id = client_fd;
  		strncpy(new_client.ip_address, ip_addr, MAX_IP_LENGTH);
  		strcpy(new_client.user, username);
  		new_client.PID = PID;
        
        	//SAVING LIKE: NAME PID SOCKET_ID IP_ADDRESS
       		fprintf(online, "%s %d %d %s\n", new_client.user, new_client.PID, new_client.socket_id, new_client.ip_address);


		// Close file and unlock semaphore
  		fclose(online);
  		struct sembuf sem_unlock = {0, 1, SEM_UNDO};
  		if (semop(sem_id, &sem_unlock, 1) == -1)
            		error("semop");


char user_check[BUF_SIZE] = "0";
char name_check[BUF_SIZE] = "0";


// MENU WHILE
	while(1){
	
	// HERE WE READ THE OPTION THE CLIENT CHOOSES AFTER BEING LOGGED IN
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	
	
	// BAN USERS, WRITES IN THE "banned.txt" THE BANNED USERS
	if(strcmp(buffer, "6\n") == 0)
	{
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	
	FILE *file = fopen("banned.txt", "a");
	fprintf(file, "%s", buffer);
	fclose(file);

	}
	
	
	
	// BLOCK WORDS, WRITES IN THE "blocked.txt" THE BLOCKED WORDS
	if(strcmp(buffer, "7\n") == 0)
	{
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	
	FILE *file = fopen("blocked.txt", "a");
	fprintf(file, "%s", buffer);
	fclose(file);

	}
	
	
	
	
	
	// LOGOUT
	if(strcmp(buffer, "8\n") == 0)
	{
	const char *filename = "online.txt";
    	char username_to_remove[BUF_SIZE];
	
	//WE SAVED THE USERNAME BEFORE, SO NOW WE PASS IT TO A VARIABLE AND CALL THE FUNCTION
    	strcpy(username_to_remove, username);
 	
 	//REMOVES THE LINE WHERE THE FIRST WORD IS THE USERNAME OF THE CLIENT THAT LOGGED OUT
    	remove_line(username_to_remove);
	
	// TO LEAVE THE MENU WHILE
	break;
	}

	// CHAT OPTION
	if(strcmp(buffer, "5\n") == 0)
	{
	
// HERE WE GET THE NAMES THAT ARE STORED IN THE "online.txt" FILE AND PASS THEM TO THE CLIENT AS A STRING
char* names = extract_names();
char name_send[BUF_SIZE];

strcpy(name_send, names);
strcpy(name_check, name_send);

		//PASSES THE ONLINE PEOPLE LIST STRING TO THE CLIENT IF THERE ARE ANY
    		if (names != NULL) 
    		{
        	write(client_fd, name_send, strlen(name_send));
        	}
        	else
        	{
        	write(client_fd, "0", strlen("0"));
        	}


	// CHAT WHILE
	while(1){
	
int client_1;  
int client_2;
int client_1_file;
int client_2_file;
	
	//HERE WE GET A STRING FROM THE CLIENT WITH THE NAME HE WANTS TO CHAT WITH
	nread = read(client_fd, buffer, BUF_SIZE-1);
	buffer[nread] = '\0';
	
	strcpy(user_check, buffer);
	strtok(user_check, "\n");

	//printf("buf: %s", buffer);

	// IF ITS EXIT IT LEAVES THE CHAT MENU 
	if(strcmp(buffer, "exit\n") == 0)
	{	
	break;
	}


		// CHECK IF THE USER TO CREATE A CHAT WITH IS ONLINE
		if(isWordInString(name_check, user_check) != 1)
		{
		write(client_fd, "0", strlen("0"));
		}

		if(isWordInString(name_check, user_check) == 1)
		{
		write(client_fd, "1", strlen("1"));
		
		
		
		//FINDS THE SOCKET_ID OF THE CLIENTS WHO WANT TO CHAT
		client_1 = get_socket_by_name(user_check);
		
		//THE SOCKET_ID OF THE CLIENT REQUESTING
		nread = read(client_fd, buffer, BUF_SIZE-1);
		buffer[nread] = '\0';
		client_2 = get_socket_by_name(buffer);
	
		//HERE WE SAVE TO THE SERVER IN A FILE, WICH ONE WILL BE THE FIRST ONE TO WRITE AND WICH ONE WILL STAY ON READ. IF THE FILE IS ALREADY CREATED, THEN IT WONT "OVERWRITE" WHAT'S BEEN WRITTEN BY ANOTHER CLIENT BEFORE, SO IT KEEPS THE RIGHT SOCKET_ID's IT NEEDS TO SEND THE "1" 'first one to write' AND "2" 'first one to read'
		if (access("ver.txt", F_OK) == -1) 
		{		
		FILE * file = fopen("ver.txt", "w");
		fprintf(file, "%s %d\n", user_check, client_1);
		fprintf(file, "%s %d", buffer, client_2);
		fclose(file);
		}
		
		//HERE WE GO TO THE FILE WHERE WE SAVED THE RIGHT SOCKETS AND USE THEM IN THE SERVER SIDE TO KEEP TRACK OF WICH CLIENT IN THE RIGHT WAY
		client_1_file = get_socket(1);
		client_2_file = get_socket(2);
		//printf("CLIENT 1: %d\n", client_1_file);
		//printf("CLIENT 2: %d\n", client_2_file);
		
		
		//client_1_file = 5;
		//client_2_file = 4;
		
		//HERE WE SEND THE INSTRUCTIONS TO EACH CLIENT
		write(client_2_file, "1", strlen("1"));
		write(client_1_file, "2", strlen("2"));
		
		// CLEANING BOTH BUFFERS
		memset(buffer, 0, sizeof(buffer));
		memset(buffer_rec, 0, sizeof(buffer));
		

		
		while(1) // CHAT INTERFACE WHILE
		{
	
		
		
	// Read message from client2
        nread = read(client_2_file, buffer_rec, BUF_SIZE); 
	buffer_rec[nread] = '\0';
	
	strcpy(buffer, buffer_rec);
        //printf("Client2: %s", buffer);

        // Send message to client1
	if(checkWord(buffer) == 1)
	{
        write(client_1_file, buffer, strlen(buffer));
 	}
 	else{
	write(client_1_file, "Sentence sent by Client1 has a blocked word", strlen("Sentence sent by Client1 has a blocked word"));
	}
        
		

		
	// Read message from client1
        nread = read(client_1_file, buffer_rec, BUF_SIZE);
	buffer_rec[nread] = '\0';
	
	strcpy(buffer, buffer_rec);
        //printf("Client1: %s", buffer);

        // Send message to client2
  	if(checkWord(buffer) == 1)
	{
        write(client_2_file, buffer, strlen(buffer));
 	}
 	else{
	write(client_2_file, "Sentence sent by Client1 has a blocked word", strlen("Sentence sent by Client1 has a blocked word"));
	}
        
   
		
	
		
		} // END OF CHAT INTERFACE WHILE
		
		//DELETES THE FILE SO NEXT CHAT WONT BE BUGGED WITH OLD INFO
		remove("ver.txt");

		} 



} // END OF CHAT WHILE

}// IF CHOSE 'CHAT' 




}// END OF WHILE MENU

}// END OF SUCCESSFUL LOGIN

// SETTING THEM AS ZERO SO IT WONT BUG THE LOGIN OR REGISTER PARTS
memset(username, 0, sizeof(username));
memset(password, 0, sizeof(password));
	
}// END OF if login	


////////////////////////////////////// LEAVE //////////////////////////////////////
	// HERE WE LEAVE THE PROGRM, SO IT DISCONECTS THE CLIENT
	if(strcmp(buffer, "4\n") == 0)
	{
	close(client_fd);
	}
////////////////////////////////////// LEAVE //////////////////////////////////////



fflush(stdout);
}// END OF THE MAIN WHILE


close(client_fd);
}// END OF PROCESS_CLIENT










///////----------------------------- FUNCTIONS -----------------------------///////



////////////////////////////////////// LOGIN //////////////////////////////////////

int login(char username[BUF_SIZE], char password[BUF_SIZE]) {
    char check[BUF_SIZE];
    FILE *cred_file = fopen("cred.txt", "r");  // FILE WITH THE SAVED LOGINS
    FILE *banned_file = fopen("banned.txt", "r");  // FILE WITH THE BANNED USERNAMES

	//CHECKS IF IT IS POSSIBLE TO OPEN THE FILES
    if (cred_file == NULL || banned_file == NULL) {
        //printf("Error: File opening\n");
        return 0;
    }
	
    // HERE WE MAKE A STRING THAT IS EQUAL IN FORMAT TO THE STRINGS IN THE "creds.txt" SO ITS EASIER TO COMPARE EACH LOGIN
    char comp_string[BUF_SIZE];
    strcpy(comp_string, username); 
    strtok(comp_string, "\n");
    strcat(comp_string, " ");
    strcat(comp_string, password); 
    strtok(comp_string, "\n"); 

	// RESETS THE FILE POINTER TO THE BEGGINING
    rewind(banned_file); 

    // HERE WE CHECK IF THE USER HAS BEEN BANNED, WE GO THROUGH THE FILE AND COMPARE IF THE USERNAME THAT WANTS TO LOGIN IS STORED IN THE BANNED FILE. IT KEEPS GOING UNTIL IT REACHES AN EMPTY LINE OR A NAME THAT IS ON THE "BAN LIST"
    while (fgets(check, BUF_SIZE, banned_file) != NULL) {
        char *stored_banned_username = strtok(check, "\n");  // BOTH OF THESE strtok() REMOVE THE \n SO WE CAN COMPARE THEM 
        strtok(username, "\n");
        if (stored_banned_username != NULL && strcmp(username, stored_banned_username) == 0) {
            fclose(cred_file); // CLOSE THE FILES
            fclose(banned_file);
            return 2; // RETURNS 2 IF THE USERNAME IS BANNED 
        }
    }

    rewind(cred_file); //SINCE WE WENT THROUGH THE "creds.txt" FILE, WE NEED TO SET THE POINTER BACK TO THE BEGINNING

  
    while (fgets(check, BUF_SIZE, cred_file) != NULL) {
        char *stored_username = strtok(check, "\n");
        if (stored_username != NULL && strcmp(comp_string, stored_username) == 0) {  //HERE WE COMPARE THE STRING WITH THE USERNAME AND PASSWORD (THAT WE MADE BEFORE) TO THE MULTIPLE FILE LINES THAT CONTAINS VALID CREDENTIALS
            fclose(cred_file);
            fclose(banned_file);
            return 1; // Username and password match --> login successful
        }
    }

    fclose(cred_file);
    fclose(banned_file);
    return 0; // Username and password combination doesn't match --> login failed // IF WE DO NOT FIND ANY MATCH, THEN THE LOGIN FAILS
}

////////////////////////////////////// LOGIN //////////////////////////////////////









////////////////////////////////////// REGISTER //////////////////////////////////////
int signup(char username[BUF_SIZE], char password[BUF_SIZE]){

char buffer[BUF_SIZE];
char line[BUF_SIZE];

size_t username_length = strcspn(username, "\n"); // HERE WE GET THE SIZE OF THE USERNAME
username[username_length] = '\0';

	

    FILE *file = fopen("cred.txt", "r");


    if(file == NULL){
        //printf("Error: file doesn't open");
        return 0;
    }



    while (fgets(line, sizeof(line), file)) { // CORRE LINHA A LINHA
        //TIRAR APENAS O USERNAME
        char *username_check = strtok(line, " ");

        // COMPARAR USERNAMES QUE JA ESTAO REGISTADOS COM O QUE SE QUERE REGISTAR
        if (strcmp(username_check, username) == 0) {
            fclose(file);
            return 0; // Existe o username, logo register falha
        }
    }
	// CHEGANDO AQUI, O REGISTER FOI BEM SUCEDIDO, O USERNAME_CHECK NAO DEU MATCH COM NENHUM USERNAME
    file = fopen("cred.txt", "a");
    fprintf(file, "%s ", username); //guardar em "creds.txt"
    fprintf(file, "%s", password); 
    fclose(file);
    
    return 1;
}
////////////////////////////////////// REGISTER //////////////////////////////////////


//FUNCTION TO GO THROUGH THE STRING, FINDS A BLOCKED WORD THAT IS IN THE FILE, AND THEN REPLACES THE FIRST 3 CHARACTERS OF THE WORD BY '*', THIS IS STILL WORK IN PROGRESS
/*
void replaceBlockedWord(char *sentence) {
    FILE *file = fopen("blocked.txt", "r");
    if (file == NULL) {
        return;
    }

    char blockedWord[BUF_SIZE];
    while (fscanf(file, "%s", blockedWord) == 1) {
        char *found = strstr(sentence, blockedWord);
        if (found != NULL) {
            // Replace the blocked word with "***o"
            memset(found, '*', 1);
            memset(found + 1, '*', 1);
            memset(found + 2, '*', 1);
            //memset(found + 3, '?', 1);
        }
    }

    fclose(file);
}
*/







// CHECKS IF THE WORD THAT WAS SENT TO THE SERVER BY A CLIENT IS IN THE "blocked.txt", SO ITS A FORBIDEN WORD
int checkWord(char buffer[BUF_SIZE]) {
    char check[BUF_SIZE];
    FILE *blocked_file = fopen("blocked.txt", "r");

    if (blocked_file == NULL) {
        return -1;
    }

    rewind(blocked_file); // Reset file pointer to beginning

 	// JUST GOES THROUGH THE BLOCKED FILE AND COMPARES THE WORDS TO WHAT WAS SENT BY THE CLIENT
    while (fgets(check, BUF_SIZE, blocked_file) != NULL) {
        char *stored_word = strtok(check, "\n");
        strtok(buffer, "\n");
        if (stored_word != NULL && strcmp(buffer, stored_word) == 0) {
            fclose(blocked_file);
            return 0; // IF THE WORD IS FOUND IN THE STRING
        }
    }

    fclose(blocked_file);
    return 1; // WORD WAS NOT FOUND
}













// CHECKS THE "ver.txt" FILE AND RETURNS THE SOCKET NUMBER STORED IN THE LINE WE WANT, SINCE WE HAVE 2 USERS IN THE CHAT, THE FIRST AND SECOND LINES ARE THE ONES WE WANT
int get_socket(int line_number) {
    FILE *file = fopen("ver.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    char buffer[100]; // Adjust buffer size as needed
    int current_line = 1;
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (current_line == line_number) {
            fclose(file);
            int socket;
            sscanf(buffer, "%*s %d", &socket); // READS THE SOCKET VALUE, THE FILE FORMAT IS: NAME SOCKET_ID, SO THAT'S WHY WE HAVE %s %D
            return socket;
        }
        current_line++; //SO WE CAN GET TO THE LINE THAT WE WANT
    }
    
    fclose(file);
    return -1; //EMPTY FILE OR NUMBER BIGGER THAN THE AMOUNT OF LINES IN THE FILE
}









// GIVES BACK THE SOCKET OF THE DESIRED USERNAME
int get_socket_by_name(char username[20]) {
    FILE *file = fopen(FILENAME, "r");// WE DEFINE FILENAME AS "online.txt" WICH SAVES CLIENT INFORMATION LIKE THIS: Username ProcessID SocketID IP
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", FILENAME);
        return -1; // Return -1 to indicate an error
    }

    char line[100]; // Assuming each line in the file won't exceed 100 characters
    int socket = -1; // Initialize socket to -1 (not found)

    // GOES THROUGH THE LINES
    while (fgets(line, sizeof(line), file) != NULL) {
        // INITIALIZE VARIABLES SO WE CAN USE THE SAME FORMAT THAT THE "online.txt" HAS
        char fileUsername[20]; 
        int filePort;
        int fileSocket; 
        char ip[20]; 
        
        //THE SAME FORMAT SO WE JUST "EXTRACT" THE SocketID
        if (sscanf(line, "%s %d %d %s", fileUsername, &filePort, &fileSocket, ip) == 4) { 
            // CHECKS IF THE USERNAMES MATCH SO WE CAN SAVE ITS ASSOCIATED SOCKET
            if (strcmp(fileUsername, username) == 0) {
                socket = fileSocket; // FOUND SOCKET
                break; // EXIT THE LOOP SINCE WE FOUND THE SOCKET
            }
        }
    }

    fclose(file);
    return socket; 
}






//THIS IS USED TO SEE IF THE USER THE CLIENT WANTS TO CHAT WITH IS ONLINE, SO IF HE WRITES "username1" IT CHECKS IF IT IS IN THE STRING THAT CONTAINS EACH USER ONLINE (STRING GENERATED BY extract_names() )
int isWordInString(char str[], char word[]) {
    // LENGTH OF THE STRING AND THE WORD
    int len_str = strlen(str);
    int len_word = strlen(word);
    
    // GO THROUGH THE STRING
    for (int i = 0; i <= len_str - len_word; ++i) {
        // CHECKS IF THE WORD MATCHES IN EACH POSITION
        int match = 1;
        for (int j = 0; j < len_word; ++j) {
            if (str[i + j] != word[j]) {
                match = 0;
                break;
            }
        }
        // WORD MATCHES
        if(match) 
        {
        return 1;
        }
    }
    // WORD NOT FOUND
    return 0;
}









// GETS THE PROCESS ID, FUNCTION HAS THE SAME WORKING MECHANISM AS THE FUNCTION TO GET THE SOCKET
int getPIDByUsername(char username[20]) {
    
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", FILENAME);
        return -1; 
    }

    char line[100]; 
    int pid = -1; 

    // Read each line in the file
 while (fgets(line, sizeof(line), file) != NULL) {
        char fileUsername[20];
        int filePID;
        char socket[20]; 
        char ip[20]; 
        
      // SAME FORMAT AS THE INFO IS STORES IN THE FILE ("online.txt")
        if (sscanf(line, "%s %d %s %s", fileUsername, &filePID, socket, ip) == 4) {
            // IF THE USERNAME MATCHES ANY OF THE USERNAMES IN THE FILE
            if (strcmp(fileUsername, username) == 0) {
                pid = filePID; // FOUND PID
                break; // EXIT SINCE WE FOUND THE DESIRED PID
            }
        }
    }

    fclose(file);
extract_names ;
   return pid; 
}













// MAKES A STRING WITH ALL THE ONLINE USERS, SO THE NAMES THAT ARE STORES IN THE FILE "online.txt"
char* extract_names() {
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", FILENAME);
        return NULL;
    }

    // ALLOCATE MEMORY FOR THE RESULTING STRING
    char* names = (char*)malloc(MAX_LINE_LENGTH * sizeof(char));
    if (names == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(file);
        return NULL;
    }
    names[0] = '\0'; // INITIALIZE AS AN EMPTY STRING

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, " "); //SAVES EACH NAME, SINCE THE NAME IS THE FIRST ELEMENT OF THE LINE, WE JUST "CUT" EVERYTHING ELSE OUT ON THAT LINE
        if (token != NULL) {
            strcat(names, token); //KEEPS SAVING ONLINE NAMES IN THE STRING
            strcat(names, " ");  //INSERT AN EMPTY SPACE BETWEEN EACH USER
        }
    }

    fclose(file);
    return names;
}









// REMOVES THE LINE FROM THE FILE "online.txt" OF A USER THAT LOGGED OUT SO IT DOESNT SHOW UP AS ONLINE ANYMORE
void remove_line(const char username[BUF_SIZE]) {
    FILE *fileptr1, *fileptr2;
    char line[MAX_LINE_LENGTH], temp[] = "temp.txt"; //WE NEED TO CREATE A NEW FILE, AND STORE EVERYTHING THAT THE "OLD" FILE HAD EXCEPT FROM WHAT WE WANT TO ERASE, SO IN OUR CASE THE LINE WITH THE INFO OF THE USER THAT LOGGED OUT
    
    //READS THE ORIGINAL FILE
    fileptr1 = fopen(FILENAME, "r");
    if (fileptr1 == NULL) {
        return;
    }
    
    //OPENS THE TEMPORARY FILE IN WRITE MODE
    fileptr2 = fopen(temp, "w");
    if (fileptr2 == NULL) {
        fclose(fileptr1);
        return;
    }
    
    // READS EACH LINE OF THE ORIGINAL FILE
    while (fgets(line, MAX_LINE_LENGTH, fileptr1) != NULL) {
        //CHECKS IF THE LINE STARTS WITH THE USERNAME TO ERASE
        if (strncmp(line, username, strlen(username)) != 0) {
            //IT IT DOESNT WRITE IT IN THE TEMPORARY FILE
            fputs(line, fileptr2);
        }
    }
    
    fclose(fileptr1);
    fclose(fileptr2);
    
    // ELIMINATES THE ORIGINAL FILE
    remove(FILENAME);
    
    //RENAMES THE TEMPORARY FILE TO "online.txt" SO IT NOW CONTAINS EVERY THAT WAS ON THE ORIGINAL EXCEPT WHAT WE WANTED TO ERASE
    rename(temp, FILENAME);
    
}







void error(char *msg)
{
printf("Error: %s\n", msg);
exit(-1);
}

