#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_FILENAME 256
#define MAX_FILES 50

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char filename[MAX_FILENAME];
    char owner[50];
    char group[50];
    char permissions[10];
    int is_writing;
    pthread_mutex_t lock; 
} FileCapability;

FileCapability clist[MAX_FILES];
int file_count = 0;

void *handle_client(void *arg) {
   
    int client_sock = *(int *)arg;
    free(arg);

  
    pthread_detach(pthread_self());

    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    char *saveptr;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(command, 0, sizeof(command));

        
        int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Client disconnected (Sock: %d)\n", client_sock);
            close(client_sock);
            return NULL;
        }
        buffer[bytes_received] = '\0'; 

        printf("Received command from client %d : %s\n", client_sock, buffer);

       
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }
        else if (strncmp(buffer, "create", 6) == 0) {
            char second_word[BUFFER_SIZE] = {0};
            char third_word[BUFFER_SIZE] = {0};

            
            char *token = strtok_r(buffer, " ", &saveptr); 
            token = strtok_r(NULL, " ", &saveptr); 
            if (token) strncpy(second_word, token, sizeof(second_word) - 1);
            
            token = strtok_r(NULL, " ", &saveptr); 
            if (token) strncpy(third_word, token, sizeof(third_word) - 1);

            pthread_mutex_lock(&file_mutex);
           
            if (file_count >= MAX_FILES) {
                snprintf(command, sizeof(command), "Server full: Cannot create more files.\n");
                pthread_mutex_unlock(&file_mutex);
            } else {
                printf("Creating file %s...\n", second_word);
                FILE *file = fopen(second_word, "w");
                if (file == NULL) {
                    perror("Error creating file");
                    snprintf(command, sizeof(command), "Error creating file.\n");
                    pthread_mutex_unlock(&file_mutex);
                } else {
                    fclose(file); 

                    strncpy(clist[file_count].filename, second_word, MAX_FILENAME - 1);
                    if (client_sock % 2 == 0) {
                        strncpy(clist[file_count].group, "AOS-students", 49);
                    } else {
                        strncpy(clist[file_count].group, "CSE-students", 49);
                    }
                    strncpy(clist[file_count].permissions, third_word, 9);
                    snprintf(clist[file_count].owner, 49, "%d", client_sock);
                    
                    clist[file_count].is_writing = 0;
                    pthread_mutex_init(&clist[file_count].lock, NULL);
                    
                    file_count++;
                    printf("Creating file success\n");
                    snprintf(command, sizeof(command), "Create success\n");
                    pthread_mutex_unlock(&file_mutex);
                }
            }
        }
        else if (strncmp(buffer, "read", 4) == 0) {
            char second_word[BUFFER_SIZE] = {0};
            char *token = strtok_r(buffer, " ", &saveptr);
            token = strtok_r(NULL, " ", &saveptr);
            if (token) strncpy(second_word, token, sizeof(second_word) - 1);

            int found = 0;
            int target_index = -1;

            pthread_mutex_lock(&file_mutex);
            for (int i = 0; i < file_count; i++) {
                if (strcmp(clist[i].filename, second_word) == 0) {
                    target_index = i;
                    found = 1;
                    break;
                }
            }

            if (found) {
              
                pthread_mutex_lock(&clist[target_index].lock);
                pthread_mutex_unlock(&file_mutex);

                if (clist[target_index].is_writing) {
                    snprintf(command, sizeof(command), "File %s is being written. Cannot read now.\n", second_word);
                } else {
                   
                    int can_read = 0;
                    char client_user[50];
                    sprintf(client_user, "%d", client_sock);
                    if (strcmp(clist[target_index].owner, client_user) == 0) {
                        can_read = clist[target_index].permissions[0] & 'r';
                    } else if ((client_sock % 2 == 0 && strcmp(clist[target_index].group, "AOS-students") == 0) ||
                               (client_sock % 2 == 1 && strcmp(clist[target_index].group, "CSE-students") == 0)) {
                        can_read = clist[target_index].permissions[2] & 'r';
                    } else {
                        can_read = clist[target_index].permissions[4] & 'r';
                    }

                    if (can_read == 'r') { // 'r' ASCII 114
                        FILE *file = fopen(second_word, "r");
                        if (file) {
                            printf("Server reading file contents:\n");
                            char ch;
                            while ((ch = fgetc(file)) != EOF) {
                                putchar(ch);
                            }
                            fclose(file); 
                            sleep(2); 
                        }
                        snprintf(command, sizeof(command), "%d", can_read); 
                    } else {
                        snprintf(command, sizeof(command), "Permission Denied\n");
                    }
                }
                pthread_mutex_unlock(&clist[target_index].lock);
            } else {
                pthread_mutex_unlock(&file_mutex);
                snprintf(command, sizeof(command), "Cannot find the file");
            }
        }
        else if (strncmp(buffer, "write", 5) == 0) {
            char second_word[BUFFER_SIZE] = {0};
            char third_word[BUFFER_SIZE] = {0};
            char *token = strtok_r(buffer, " ", &saveptr);
            token = strtok_r(NULL, " ", &saveptr); 
            if (token) strncpy(second_word, token, sizeof(second_word) - 1);
            token = strtok_r(NULL, " ", &saveptr); 
            if (token) strncpy(third_word, token, sizeof(third_word) - 1);

            int found = 0;
            int target_index = -1;

            pthread_mutex_lock(&file_mutex);
            for (int i = 0; i < file_count; i++) {
                if (strcmp(clist[i].filename, second_word) == 0) {
                    target_index = i;
                    found = 1;
                    break;
                }
            }

            if (found) {
                pthread_mutex_lock(&clist[target_index].lock);
                pthread_mutex_unlock(&file_mutex); 

                if (clist[target_index].is_writing) {
                    snprintf(command, sizeof(command), "File %s is being accessed. Cannot write now.\n", second_word);
                } else {
                    clist[target_index].is_writing = 1;
                    
                    
                    int can_write = 0;
                    char client_user[50];
                    sprintf(client_user, "%d", client_sock);
                    if (strcmp(clist[target_index].owner, client_user) == 0) {
                        can_write = clist[target_index].permissions[1] & 'w';
                    } else if ((client_sock % 2 == 0 && strcmp(clist[target_index].group, "AOS-students") == 0) ||
                               (client_sock % 2 == 1 && strcmp(clist[target_index].group, "CSE-students") == 0)) {
                        can_write = clist[target_index].permissions[3] & 'w';
                    } else {
                        can_write = clist[target_index].permissions[5] & 'w';
                    }

                    FILE *file = fopen(second_word, third_word);
                    if (file == NULL) {
                        perror("Error opening file for write");
                        clist[target_index].is_writing = 0;
                        snprintf(command, sizeof(command), "Error opening file\n");
                    } else {
                       
                        send(client_sock, "Enter the content to write: ", strlen("Enter the content to write: "), 0);
                        
                        char content[1024] = {0};
                        
                        int valread = read(client_sock, content, sizeof(content) - 1);
                        if (valread > 0) {
                            content[valread] = '\0';
                            fprintf(file, "%s", content);
                        }
                        fclose(file); 
                        
                        clist[target_index].is_writing = 0;
                        snprintf(command, sizeof(command), "%d", can_write); 
                    }
                }
                pthread_mutex_unlock(&clist[target_index].lock);
            } else {
                pthread_mutex_unlock(&file_mutex);
                snprintf(command, sizeof(command), "File %s not found\n", second_word);
            }
        }
        else if (strncmp(buffer, "mode", 4) == 0) {
            char second_word[BUFFER_SIZE] = {0};
            char third_word[BUFFER_SIZE] = {0};
            char *token = strtok_r(buffer, " ", &saveptr);
            token = strtok_r(NULL, " ", &saveptr);
            if (token) strncpy(second_word, token, sizeof(second_word) - 1);
            token = strtok_r(NULL, " ", &saveptr);
            if (token) strncpy(third_word, token, sizeof(third_word) - 1);

            int found = 0;
            pthread_mutex_lock(&file_mutex);
            
            int target_index = -1;
            for(int i=0; i<file_count; i++){
                if(strcmp(clist[i].filename, second_word) == 0){
                    target_index = i;
                    found = 1;
                    break;
                }
            }

            if(found){
                pthread_mutex_lock(&clist[target_index].lock);
                pthread_mutex_unlock(&file_mutex);

                if(clist[target_index].is_writing){
                     snprintf(command, sizeof(command), "File is busy.\n");
                } else {
                   
                    strncpy(clist[target_index].permissions, third_word, 9);
                    clist[target_index].permissions[9] = '\0';
                    snprintf(command, sizeof(command), "mode file\n");
                }
                pthread_mutex_unlock(&clist[target_index].lock);
            } else {
                pthread_mutex_unlock(&file_mutex);
                snprintf(command, sizeof(command), "Cannot find the file");
            }
        }
        else if (strncmp(buffer, "show", 4) == 0) {
            pthread_mutex_lock(&file_mutex);
            for (int i = 0; i < file_count; i++) {
                printf("%s %s %s %s\n", clist[i].filename, clist[i].owner, clist[i].group, clist[i].permissions);
            }
            pthread_mutex_unlock(&file_mutex);
            snprintf(command, sizeof(command), "show clist\n");
        }
        else {
            snprintf(command, sizeof(command), "Wrong command\n");
        }

        send(client_sock, command, strlen(command), 0);
    }
    close(client_sock);
    return NULL;
}

int main(void) {
    int server_sock;
    struct sockaddr_in server_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
  
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock); 
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int)); 

        if (client_sock == NULL) {
            perror("Malloc failed");
            continue;
        }

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
