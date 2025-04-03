#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    pid_t  childpid;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // char *cmd;
    //參數處理
    string current_exec_name = argv[0]; // Name of the current exec program
    int cnt=0;
    while(argv[cnt] != NULL){
        cnt++;
    }

    portno = atoi(argv[1]);
    //bind and listen
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(sockfd,5);
    
    while(1){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        
        if((childpid = fork()) == 0){
            close(sockfd);
            printf("server: got connection from %s port %d\n",inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            dup2(newsockfd,0);
            // /path/to/an/invalid/executable
            dup2(newsockfd,1);
            // dup2(newsockfd,2);
            if(execvp(argv[2],argv+2)<0){
                if(argv[2]=="ls"){
                    dup2(newsockfd,2);
                    error("ERROR on exec");
                }
                else{
                    error("ERROR on exec");
                }
            }
            // dup2(newsockfd, 2);
            // fprintf(stderr,"this is second!\n");
			exit(0);
        }
        close(newsockfd);  
    }
     return 0; 
}