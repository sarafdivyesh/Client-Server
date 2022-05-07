#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SIZE 1024

void send_file(FILE *fp, int sockfd){
    int n;
    char data[SIZE] = {0};

    while(fgets(data, SIZE, fp) != NULL) {
        if (send(sockfd, data, sizeof(data), 0) == -1) {
        perror("[-]Error in sending file.");
        exit(1);
        }
        bzero(data, SIZE);
    }
}


void write_file(int sockfd){
    int n;
    FILE *fp;
    char *filename = "recv.txt";
    char buffer[SIZE];
  

    fp = fopen(filename, "w+");
    while (1) {
        n = recv(sockfd, buffer, SIZE, 0);
        if (strcmp(buffer, "END") == 0)
        {
            break;
        }
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
    fclose(fp);
    return;
}

int handle_client(int new_sock)
{
    char message[255];
    char word[255];
    FILE *fp1;
    write_file(new_sock);
    printf("File Data received from client.\n");

    /* Read word to be searched from client */
    if (read(new_sock, word, 255)<0){
        fprintf(stderr, "read() error\n");
        exit(3); 
    }
    fprintf(stderr, "Word received from client: %s\n", word);
    char* word1 = word;

    /* Run grep command at server side and send data to client */
    int f = fork();
    if(f==0)
    {
	    char data[SIZE] = {0};
	    if(fork()==0){
	        int file = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	        if(file == -1){
		        return 2;
	        }
	        int file1 = dup2(file, 1);
	        close(file);
    	    int err = execlp("egrep","egrep","-i","--color=always",word1,"recv.txt",NULL);
    	    if(err == -1){
		        printf("could not find program to execute");
		        return 3;
	        } 
        }
	    else{
	        wait(NULL);
	        fp1 = fopen("output.txt", "r");
 	        int n;
	        while(1) {
	            if(fgets(data, SIZE, fp1) != NULL)
	            {
	            printf("[SENDING Data]: %s", data);
	            n = send(new_sock, data, sizeof(data), 0);
    	        if ( n == -1) {
      	            perror("Error in sending file.");
     	            exit(1);
  	            }
	            }
	            else{
	                break;
                }
            }
            strcpy(data, "END");
            send(new_sock, data, sizeof(data), 0);
	        printf("Final data send successfully to client.\n");


	    }
    }
    else{
    wait(NULL);
    }
}


int main(){
    char *ip = "127.0.0.1";
    FILE *fp1;
    int port = 8080;
    int e,t;
    char c;

    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buffer[SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }
    printf("Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e < 0) {
        perror("Error in bind");
        exit(1);
    }
    printf("Binding successfull.\n");

    if(listen(sockfd, 10) == 0){
	}   
    else{
		perror("Error in listening");
        exit(1);
	}

    addr_size = sizeof(new_addr);

    while(1)
    {
         new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
         printf("\nGot a client\n");
         if(!fork())
            handle_client(new_sock);

        close(new_sock);
    }
    
}