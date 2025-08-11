#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_FILENAME 256

int main(){
	int sock;
    	struct sockaddr_in server_addr;
    	char buffer[BUFFER_SIZE];
	char r_buffer[BUFFER_SIZE];

	sock = socket(AF_INET, SOCK_STREAM, 0);
    	if (sock < 0) {
        	perror("Socket creation failed");
        	exit(EXIT_FAILURE);
    	}

    	server_addr.sin_family = AF_INET;
    	server_addr.sin_port = htons(PORT);
    	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        	perror("Connection failed");
        	close(sock);
        	exit(EXIT_FAILURE);
    	}

    	printf("Connected to server. Enter commands:\n");
	FILE *file;
	while (1) {
        	printf("> ");
        	memset(buffer, 0, sizeof(buffer));
		memset(r_buffer, 0, sizeof(r_buffer));
        	fgets(buffer, sizeof(buffer), stdin);
		fflush(stdin);
        	buffer[strcspn(buffer, "\n")] = 0;  

        	if (strcmp(buffer, "exit") == 0) {
            		break;
        	}
		else if(strncmp(buffer, "create", 6) == 0){
			send(sock, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof(buffer));
        		recv(sock, r_buffer, BUFFER_SIZE, 0);
        		printf("Server: %s\n", buffer);
		}
		else if(strncmp(buffer, "read", 4) == 0){
			send(sock, buffer, strlen(buffer), 0);
			
			recv(sock, r_buffer, BUFFER_SIZE, 0);

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
			if(strncmp(r_buffer, "114", 3) == 0){
				printf("Server: read file\n");
				
				file = fopen(second_word, "r");
				if (file == NULL) {
        				perror("Error opening file");
        				//return EXIT_FAILURE;
    				}
				printf("File contents:\n");
				
    				while ((ch = fgetc(file)) != EOF) {
        				putchar(ch);
							
    				}
			}

			else if(strncmp(r_buffer, "Can" ,3) ==0){
				printf("No such a file\n");
			}
			else{
				printf("Permission Denied\n");
			}
			memset(buffer, 0, sizeof(buffer));
		}
		else if(strncmp(buffer, "mode", 4) == 0){
			send(sock, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof(buffer));
			recv(sock, r_buffer, BUFFER_SIZE, 0);
			printf("Server: %s\n", r_buffer);
		}
		else if(strncmp(buffer, "write", 5) == 0){
			send(sock, buffer, strlen(buffer), 0);
			//memset(buffer, 0, sizeof(buffer));
			recv(sock, r_buffer, BUFFER_SIZE, 0);
		
			printf("Server: %s\n", r_buffer);	
			if(strncmp(r_buffer, "119", 3) == 0){
				printf("Write success\n");
			}
			else if(strncmp(r_buffer, "Enter" , 5) == 0){
				printf("\n");
			}
			else if(strncmp(r_buffer, "Can" ,3) ==0){
				printf("No such a file\n");
			}
			else{
				printf("Permission Denied\n");
			}
			
		}
		else if(strncmp(buffer, "show", 4) == 0){
			send(sock, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof(buffer));
			recv(sock, r_buffer, BUFFER_SIZE, 0);
			printf("Server: %s\n", r_buffer);
		}
		else {
			send(sock, buffer, strlen(buffer), 0);
			memset(buffer, 0, sizeof(buffer));
			recv(sock, r_buffer, BUFFER_SIZE, 0);
			printf("Server: %s\n", r_buffer);
			if(strncmp(r_buffer, "119", 3) == 0){
				printf("Write success\n");
			}
			else if(strncmp(r_buffer, "Can" ,3) ==0){
				printf("No such a file\n");
			}
			else if(strncmp(r_buffer, "Wrong" ,5) ==0){
				;
			}
			else{
				printf("Permission Denied\n");
			}
		}

        	
		
    	}


	return 0;
}
