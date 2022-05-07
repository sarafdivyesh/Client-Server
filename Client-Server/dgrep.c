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
            perror("Error in sending file.");
            exit(1);
        }
        bzero(data, SIZE);
    }
    strcpy(data, "END");
    send(sockfd, data, sizeof(data), 0);
    fclose(fp);
}



int main(int argc, char *argv[]){
    char message[1000];
    char *ip = "127.0.0.1";
    int port = 8080;
    int e;

    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    char *file1=argv[2];
    char *file2 = argv[3];
    char *word=argv[1];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }
    printf("Server socket created successfully.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(e == -1) {
        perror("Error in socket");
        exit(1);
    }
	printf("Connected to Server.\n");

    fp = fopen(file2, "r");
    if (fp == NULL) {
        perror("Error in reading file.");
        exit(1);
    }

    send_file(fp, sockfd);
    printf("%s data sent successfully to server.\n",file2);

     /* Send word to be searched to client */
    write(sockfd, word, strlen(word)+1);

    /* Run grep command at client side */
    FILE *fp2;
    char data[SIZE] = {0};
    if(fork()==0)
    {   
        int file = open("client_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	    if(file == -1){
		    return 2;
	    }
        int file2 = dup2(file, 1);
        close(file);
        int err = execlp("egrep","egrep","-i","--color=always",word,file1,NULL);
    	if(err == -1){
		    printf("could not find program to execute");
		    return 3;
	    }  
    }
    else{
        wait(NULL);
        fp2 = fopen("client_output.txt", "r");
        while(1) {
            if(fgets(data, SIZE, fp2) != NULL)
            {
                printf("%s:%s",file1,data);
            }
            else{
                break;
            }
        }
    }
  
    /* Receive final data from server and display on screen */
    int n;
    FILE *fp1;
    char *filename1 = "recv_client.txt";
    char buffer[SIZE];

    fp1 = fopen(filename1, "w+");
    while (1) {
        n = recv(sockfd, buffer, SIZE, 0);
        if (strcmp(buffer, "END") == 0)
        {
            break;
        }
        printf("%s:%s",file2 ,buffer);
        fprintf(fp, "%s", buffer);
        bzero(buffer, SIZE);
    }
 
    
    fclose(fp);
    return;
}