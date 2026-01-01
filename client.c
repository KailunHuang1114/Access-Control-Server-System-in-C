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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
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

    printf("Connected to server. Enter commands (create/read/write/mode/show/exit):\n");
    
    while (1) {
        printf("> ");
        memset(buffer, 0, sizeof(buffer));
        memset(r_buffer, 0, sizeof(r_buffer));
        
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break; // Handle EOF
        }
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            send(sock, buffer, strlen(buffer), 0);
            break;
        }

        send(sock, buffer, strlen(buffer), 0);

        int valread = recv(sock, r_buffer, sizeof(r_buffer) - 1, 0);
        if (valread > 0) {
            r_buffer[valread] = '\0';
        } else if (valread == 0) {
            printf("Server disconnected.\n");
            break;
        } else {
            perror("Recv failed");
            break;
        }

        printf("Server: %s\n", r_buffer);


        if (strncmp(buffer, "read", 4) == 0) {

            char second_word[BUFFER_SIZE] = {0};
            char temp_buf[BUFFER_SIZE];
            strncpy(temp_buf, buffer, sizeof(temp_buf)); 
            
            char *token = strtok(temp_buf, " ");
            token = strtok(NULL, " "); 
            if (token != NULL) {
                strncpy(second_word, token, sizeof(second_word) - 1);
            }

            if (strncmp(r_buffer, "114", 3) == 0) {
                if (strlen(second_word) > 0) {
                    FILE *file = fopen(second_word, "r");
                 
                    if (file == NULL) {
                        printf("Error: Could not open local file '%s'.\n", second_word);
                    } else {
                        printf("Local File Contents:\n");
                        char ch;
                        while ((ch = fgetc(file)) != EOF) {
                            putchar(ch);
                        }
                        printf("\n");
                      
                        fclose(file);
                    }
                } else {
                    printf("Error: No filename parsed.\n");
                }
            }
        }

        else if (strncmp(buffer, "write", 5) == 0) {
         
            if (strncmp(r_buffer, "Enter", 5) == 0) {
               
                char content[BUFFER_SIZE];
               
                if (fgets(content, sizeof(content), stdin) != NULL) {
                    content[strcspn(content, "\n")] = 0; 
    
                    send(sock, content, strlen(content), 0);

             
                    memset(r_buffer, 0, sizeof(r_buffer));
                    valread = recv(sock, r_buffer, sizeof(r_buffer) - 1, 0);
                    if (valread > 0) {
                        r_buffer[valread] = '\0';
                        printf("Server: %s\n", r_buffer);
                    }
                }
            }
            
         
            if (strncmp(r_buffer, "119", 3) == 0) {
                printf("Write permission confirmed / Success.\n");
            }
        }
    }

    close(sock);
    return 0;
}
