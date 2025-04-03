#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <memory>
#include <array>
#include <vector>
using namespace std;

const int maxn=100;
char msg[maxn];

string Time(){
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    string T = string(asctime(timeinfo));
    T.pop_back();
    return T;
}

void Send(int fd, string data){
    memset(&msg, 0, sizeof(msg));
    strcpy(msg, data.c_str());
    send(fd, (char*)&msg, strlen(msg), MSG_NOSIGNAL);
    bzero(msg , sizeof(msg));
}

void sigCatcher(int n) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char *argv[])
{
    struct  timeval start, end;
    unsigned long long total_bytes = 0;
    int i, j, k, leave, client_num = 0;
    int port = atoi(argv[1]);
    int cmd_listenfd, connectfd, data_listenfd, connectfd2, sockfd;
    // char msg[1500];
    int nready, client_fd[FD_SETSIZE], client_port[FD_SETSIZE], client_type[FD_SETSIZE];
    vector<string> client_name(FD_SETSIZE, "");
    vector<string> client_ip(FD_SETSIZE, "");
    string data, leave_msg;
    unsigned long long  n;
    fd_set	rset, allset;


    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    sockaddr_in servData;
    bzero((char*)&servData, sizeof(servData));
    servData.sin_family = AF_INET;
    servData.sin_addr.s_addr = htonl(INADDR_ANY);
    servData.sin_port = htons(port + 1);
 
    cmd_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cmd_listenfd < 0){
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    data_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(data_listenfd < 0){
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    
    const int enable = 1;
    if (setsockopt(cmd_listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        cerr << "setsockopt(SO_REUSEADDR) failed" << endl;
    if (setsockopt(data_listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        cerr << "setsockopt(SO_REUSEADDR) failed" << endl;

    if(bind(cmd_listenfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    if(bind(data_listenfd, (struct sockaddr*) &servData, sizeof(servData)) < 0){
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }

    signal(SIGCHLD, sigCatcher);
    signal(SIGPIPE, SIG_IGN);

    listen(cmd_listenfd, 20);
    listen(data_listenfd, 20);

    int maxi = -1;					
	for (j = 0; j < FD_SETSIZE; j++){
		client_fd[j] = -1;	
        client_type[j] = 0;		
    }
	FD_ZERO(&allset);
	FD_SET(cmd_listenfd, &allset);
    FD_SET(data_listenfd, &allset);

    

    int maxfd = max(cmd_listenfd, data_listenfd);
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    while(1){
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(cmd_listenfd, &rset)) {
			connectfd = accept(cmd_listenfd, (sockaddr *)&newSockAddr, &newSockAddrSize);
            
            client_fd[connectfd] = connectfd;
            // client_type[connectfd] = 1;

            for (i = 0; i < FD_SETSIZE; i++)
                if (client_fd[i] < 0) { 
                    client_fd[i] = connectfd;	/* save descriptor */
                    break;
            }
            client_type[i] = 1;
			if (i == FD_SETSIZE)
				perror("too many clients");

			FD_SET(connectfd, &allset);	/* add new descriptor to set */
            
			if (connectfd > maxfd)
				maxfd = connectfd;	/* for select */

			if (i > maxi)
				maxi = i;           /* max index in client[] array */
            
            cout<<"godd9998"<<"\n";
			if (--nready <= 0)
				continue;
		}
        
        if (FD_ISSET(data_listenfd, &rset)) {
			connectfd = accept(data_listenfd, (sockaddr *)&newSockAddr, &newSockAddrSize);
            client_num++;
            
            client_fd[connectfd] = connectfd;
            // client_type[connectfd] = 0;

            for (i = 0; i < FD_SETSIZE; i++)
                if (client_fd[i] < 0) { 
                    client_fd[i] = connectfd;	/* save descriptor */
                    break;
            }
            client_type[i] = 0;
			if (connectfd == FD_SETSIZE)
				perror("too many clients");

			FD_SET(connectfd, &allset);	
            
			if (connectfd > maxfd)
				maxfd = connectfd;	

			if (i > maxi)
				maxi = i; 

            cout<<"good9999"<<"\n";
			if (--nready <= 0)
				continue;
		}

        for (i = 0; i <= maxi; i++) {	
			if ((sockfd = client_fd[i]) < 0)
				continue;

			if (FD_ISSET(sockfd, &rset)){
                memset(&msg, 0, sizeof(msg));
				if ((n = read(sockfd, msg, maxn)) == 0){
                    client_fd[i] = -1;
                    if(client_type[i] == 0){
                        client_num--;
                        cout << "data link is disconnected" << endl;
                    }
                    close(sockfd);
					FD_CLR(sockfd, &allset);
                }
                else{
                    cout << msg ;
                    if(client_type[i] == 1){
                        data = string(msg);
                        if(data == "/reset\n"){
                            gettimeofday(&start,NULL);
                            data = to_string(start.tv_sec) + "." + to_string(start.tv_usec) + " RESET " + to_string(total_bytes) + "\n";
                            Send(client_fd[i], data);
                            total_bytes = 0;
                        }
                        else if(data == "/ping\n"){
                            gettimeofday(&end,NULL);
                            data = to_string(end.tv_sec) + "." + to_string(end.tv_usec) + " PONG\n";
                            Send(client_fd[i], data);
                        }
                        else if(data == "/report\n"){
                            gettimeofday(&end,NULL);
                            data = to_string(end.tv_sec) + "." + to_string(end.tv_usec) + " REPORT " + to_string(total_bytes) + " " + to_string(8.0*total_bytes/1000000.0/(end.tv_sec - start.tv_sec)) + "\n";
                            data += "total_bytes=" + to_string(total_bytes) + ", time = " + to_string(end.tv_sec - start.tv_sec) + "\n";
                            Send(client_fd[i], data);
                        }
                        else if(data == "/clients\n"){
                            gettimeofday(&end,NULL);
                            data = to_string(end.tv_sec) + "." + to_string(end.tv_usec) + " CLIENTS " + to_string(client_num) + "\n";
                            Send(client_fd[i], data);
                        }
                        else{
                            cout << "cmd?" <<"\n";
                            bzero(msg , sizeof(msg));
                        }
                    }
                    else{
                        total_bytes += n;
                        continue;
                    }
                }
            }
				if (--nready <= 0)
					break;
		}
		}
    
    return 0;   
}