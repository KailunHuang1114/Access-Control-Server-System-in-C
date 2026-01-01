#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_FILENAME 256
#define MAX_FILES 50


pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
	
    	char filename[MAX_FILENAME];
    	char owner[50];
    	char group[50];//odd and even
    	char permissions[10];
	int is_writing;
	pthread_mutex_t lock;
		
} FileCapability;

FileCapability clist[MAX_FILES];
int file_count = 0;

void *handle_client(void *arg) {
	int client_sock = *(int *)arg;
    	char buffer[BUFFER_SIZE];
	char command[BUFFER_SIZE];
	FILE *file;
    	while(1){
		memset(buffer, 0, sizeof(buffer));
		memset(command,0,sizeof(buffer));
        	int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        	if (bytes_received <= 0) {
            		printf("Client disconnected\n");
           	 	close(client_sock);
           		return NULL;
        	}
		printf("Received command from client %d : %s\n",client_sock, buffer);
		pthread_mutex_lock(&file_mutex);

		if (strncmp(buffer, "create", 6) == 0) {

           		printf("Creating file...\n");
			
			char second_word[BUFFER_SIZE];
			char third_word[BUFFER_SIZE];
    			char *token = strtok(buffer, " ");
    			int word_count = 0;
			while (token != NULL) {
        			word_count++;
       		 		if (word_count == 2) {
            				// Copy the second word into second_word
            				strncpy(second_word, token, BUFFER_SIZE);
            				second_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				
        			}
				if (word_count == 3) {
            				// Copy the third word into third_word
            				strncpy(third_word, token, BUFFER_SIZE);
            				third_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				break;  // Stop after the third word
        			}
		
        			token = strtok(NULL, " ");
    			}
			file = fopen(second_word, "w");
			if (file == NULL) {
        			perror("Error creating file");
        			return EXIT_FAILURE;
    			}
			
			strncpy(clist[file_count].filename, second_word, strlen(second_word));
			if(client_sock%2==0){
				strncpy(clist[file_count].group, "AOS-students", strlen("AOS-students"));
			}
			else{
				strncpy(clist[file_count].group, "CSE-students", strlen("CSE-students"));
			}
			strncpy(clist[file_count].permissions, third_word, strlen(third_word));
			sprintf(clist[file_count].owner,"%d",client_sock);
			clist[file_count].is_writing = 0;
			pthread_mutex_init(&clist[file_count].lock, NULL);
			fclose(file);
			file_count++;
			printf("Creating file success\n");
			strncpy(command, "Create success\n", strlen("Create success\n"));


        	}

		else if(strncmp(buffer, "read", 4) == 0){
			
			printf("Reading file...\n");
			char second_word[BUFFER_SIZE];
    			char *token = strtok(buffer, " ");
			char ch;
    			int word_count = 0;
			int found=0;
			int can_read = 0;
			while (token != NULL) {
        			word_count++;
       		 		if (word_count == 2) {
            				// Copy the second word into second_word
            				strncpy(second_word, token, BUFFER_SIZE);
            				second_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				break;
        			}
        			token = strtok(NULL, " ");
    			}
		
			for (int i = 0; i < file_count; i++) {
                		if (strcmp(clist[i].filename, second_word) == 0) {
                    			pthread_mutex_lock(&clist[i].lock);
                    			if (clist[i].is_writing) {
                        			snprintf(command, sizeof(command), "File %s is being written. Cannot read now.\n", second_word);
                    			} 
					else{
                        			
                        			snprintf(command, sizeof(command), "Reading file %s...\n", second_word);
						memset(command,0,sizeof(buffer));
						file = fopen(second_word, "r");
						if (file == NULL) {
        						perror("Error opening file");
        						return EXIT_FAILURE;
    						}
						printf("File contents:\n");
    						while ((ch = fgetc(file)) != EOF) {
        						putchar(ch);
							
    						}
							fclose(file);
						
						char client_user[50];
            					sprintf(client_user, "%d", client_sock);
            					if (strcmp(clist[i].owner, client_user) == 0) {
                					can_read = clist[i].permissions[0] & 'r'; // Check owner's read permission
            					} 
						else if ((client_sock%2==0 && strcmp(clist[i].group,"AOS-students")==0)||(client_sock%2==1&&strcmp(clist[i].group,"CSE-students")==0)){
                					can_read = clist[i].permissions[2] & 'r'; // Check group's read permission
            					} 
						else {
                					can_read = clist[i].permissions[4] & 'r'; // Check others' read permission
            					}
                        			// Simulate reading
                        			sleep(2);
                        			
                    			}
                    			pthread_mutex_unlock(&clist[i].lock);
                    			found = 1;
                    			break;
                		}
            		}
			

			if (!found) {
                		strncpy(command,"Cannot find the file",strlen("Cannot find the file"));
            		}
			else{
				sprintf(command ,"%d",can_read);
				//strncpy(command,"read file\n",strlen("read file\n"));
			}
			
			

		}
		else if(strncmp(buffer, "write", 5) == 0) {
			char second_word[BUFFER_SIZE];
			char third_word[BUFFER_SIZE];
			char message[256];
			char input[256];
    			char *token = strtok(buffer, " ");
    			int word_count = 0;
			int found =0;
			int can_read =0 ;
			while (token != NULL) {
        			word_count++;
       		 		if (word_count == 2) {
            				// Copy the second word into second_word
            				strncpy(second_word, token, BUFFER_SIZE);
            				second_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				
        			}
				if (word_count == 3) {
            				// Copy the third word into third_word
            				strncpy(third_word, token, BUFFER_SIZE);
            				third_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				break;  // Stop after the third word
        			}
		
        			token = strtok(NULL, " ");
    			}
			for (int i = 0; i < file_count; i++) {
                		if (strcmp(clist[i].filename, second_word) == 0) {
                    			pthread_mutex_lock(&clist[i].lock);
                    			if (clist[i].is_writing) {
                        			snprintf(command, sizeof(command), "File %s is being accessed. Cannot write now.\n", second_word);
                    			} 
					else {
                        			clist[i].is_writing = 1;
                        			snprintf(command, sizeof(command), "Writing to file %s in %s mode...\n", second_word, third_word);
						memset(command,0,sizeof(buffer));
                        			
				
                        			file = fopen(second_word, third_word);
						if (file == NULL) {
        						perror("Error opening file");
							clist[i].is_writing = 0;
							pthread_mutex_unlock(&clist[i].lock);
        						return EXIT_FAILURE;
    						}
						send(client_sock, "Enter the content to write: ", strlen("Enter the content to write: "), 0);
    						char content[1024] = {0};
                				int valread = read(client_sock, content, sizeof(content) - 1);
                				if (valread > 0) {
                    					content[valread] = '\0'; 
                    					fprintf(file, "%s", content);
                    					snprintf(command, sizeof(command), "Content written to %s successfully.\n", second_word);
							memset(command,0,sizeof(buffer));
                				} 
						else {
                    					snprintf(command, sizeof(command), "No content received.\n");
							memset(command,0,sizeof(buffer));
                				}fclose(file);
                        			clist[i].is_writing = 0;

						char client_user[50];
            					sprintf(client_user, "%d", client_sock);
            					if (strcmp(clist[i].owner, client_user) == 0) {
                					can_read = clist[i].permissions[1] & 'w'; // Check owner's read permission
            					} 
						else if ((client_sock%2==0 && strcmp(clist[i].group,"AOS-students")==0)||(client_sock%2==1&&strcmp(clist[i].group,"CSE-students")==0)){
                					can_read = clist[i].permissions[3] & 'w'; // Check group's read permission
            					} 
						else {
                					can_read = clist[i].permissions[5] & 'w'; // Check others' read permission
            					}
                    			}
                    			pthread_mutex_unlock(&clist[i].lock);
                    			found = 1;
                    			break;
                		}
            		}
            		if (!found) {
                		snprintf(command, sizeof(command), "File %s not found\n", second_word);
            		}
			else{
				sprintf(command ,"%d",can_read);
				//strncpy(command,"read file\n",strlen("read file\n"));
			}
			//strncpy(command,"write file\n",strlen("write file\n"));


		}
		else if(strncmp(buffer, "mode", 4) == 0){
			char second_word[BUFFER_SIZE];
			char third_word[BUFFER_SIZE];
    			char *token = strtok(buffer, " ");
    			int word_count = 0;
			int found=0;
			int can_read = 0;
			while (token != NULL) {
        			word_count++;
       		 		if (word_count == 2) {
            				// Copy the second word into second_word
            				strncpy(second_word, token, BUFFER_SIZE);
            				second_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				
        			}
				if (word_count == 3) {
            				// Copy the third word into third_word
            				strncpy(third_word, token, BUFFER_SIZE);
            				third_word[BUFFER_SIZE - 1] = '\0';  // Ensure null termination
            				break;  // Stop after the third word
        			}
		
        			token = strtok(NULL, " ");
    			}
			for (int i = 0; i < file_count; i++) {
				if (strcmp(clist[i].filename, second_word) == 0) {
                    			pthread_mutex_lock(&clist[i].lock);
                    			if (clist[i].is_writing) {
                        			snprintf(command, sizeof(command), "File %s is being written. Cannot mode now.\n", second_word);
                    			} 
					else{
                        			
                        			snprintf(command, sizeof(command), "mode file %s...\n", second_word);
						memset(command,0,sizeof(buffer));
						strncpy(clist[i].permissions, third_word, strlen(third_word));
                        			
                    			}
                    			pthread_mutex_unlock(&clist[i].lock);
                    			found = 1;
                    			break;
                		}
			}
			if (!found) {
                		strncpy(command,"Cannot find the file",strlen("Cannot find the file"));
            		}
			else{
				strncpy(command,"mode file\n",strlen("mode file\n"));
			}
			
			
			
		}
		else if(strncmp(buffer, "show", 4) == 0){
			for(int i=0; i< file_count;i++){
				printf("%s ",clist[i].filename);
				printf("%s ",clist[i].owner);
				printf("%s ",clist[i].group);
				printf("%s\n ",clist[i].permissions);
			}
			strncpy(command,"show clist\n",strlen("show clist\n"));
		}
		else{
			printf("Wrong command\n");
			strncpy(command, "Wrong command\n",strlen("Wrong command\n"));
		}
		pthread_mutex_unlock(&file_mutex);
		send(client_sock, command, strlen(command), 0);
	}
}
int main(void){
	int server_sock;
	int *client_sock;
    	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
    	socklen_t client_len = sizeof(client_addr);

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
    	if (server_sock < 0){
        	perror("Socket creation failed\n");
        	exit(EXIT_FAILURE);
    	}
	else{
		printf("Socket creation succeed\n");
	}
	server_addr.sin_family = AF_INET;
    	server_addr.sin_addr.s_addr = INADDR_ANY;
    	server_addr.sin_port = htons(PORT);
	
	if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        	perror("Bind failed");
        	exit(EXIT_FAILURE);
    	}
	else{
		printf("Bind succeed\n");
	}



	if (listen(server_sock, MAX_CLIENTS) < 0) {
        	perror("Listen failed");
        	exit(EXIT_FAILURE);
    	}
	printf("Server listening on port %d\n", PORT);


	while(1){

		client_sock = malloc(sizeof(int));
        	*client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        	if (*client_sock < 0) {
            		perror("Accept failed");
            		free(client_sock);
            		continue;
        	}
        	printf("Client connected\n");
		pthread_t thread;
        	if (pthread_create(&thread, NULL, handle_client, client_sock) != 0) {
            		perror("Thread creation failed");
            		close(*client_sock);
            		free(client_sock);
        	}

	}




	close(server_sock);
	return 0;
}
