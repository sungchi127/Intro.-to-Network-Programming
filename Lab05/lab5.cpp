#include<bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <chrono>
#include <ctime> 
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


/* include fig01 */
int main(int argc, char **argv)
{
    auto start = std::chrono::system_clock::now();
    time_t end_time = chrono::system_clock::to_time_t(start);
    int MAXLINE=4096;
	int					i, maxi, maxfd, listenfd, connfd, sockfd,name=0;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	map<int,string> m;
	string addr[2000];
	signal(SIGPIPE,SIG_IGN);
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	int portno = atoi(argv[1]);
	servaddr.sin_port        = htons(portno);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, 500);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
/* end fig01 */

/* include fig02 */
	while(1) {
		rset = allset;		/* structure assignment */
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(listenfd, &rset)){	/* new client connection */
			//connfd=name+3
			name++;
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

			// printf("%s *** User %d has just landed on the server\n",dt,name);
			printf("* client connected from %s:%d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
			// cout<<name<<"\n";
            //hello message
            send(connfd, ctime(&end_time), 24,  MSG_NOSIGNAL);
			string s = " *** Welcome to the simple CHAT server\n";
            char* c=(char*)s.c_str();
            send(connfd, c, 40,  MSG_NOSIGNAL);
			
			m[connfd]=to_string(name);//descriptor map 到 name
			string address=inet_ntoa(cliaddr.sin_addr);
			addr[connfd]=address+':'+to_string(ntohs(cliaddr.sin_port));//client's address+port

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) { 
					client[i] = connfd;	/* save descriptor */
					break;
				}
			if (i == FD_SETSIZE)
				error("too many clients");

			FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd;			/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			//計算在線人數
			int cnt=1;
			// for (int i = 0; i < maxi; i++)
			// {
			// 	if(client[i]>=0) cnt++;
			// }
			for(int i = 0; i <= maxi; i++){
				if (FD_ISSET(client[i], &allset)){
					if (client[i] != listenfd && client[i] != connfd) {
						cnt++;
					}
				}
			}

			//hello message user online
            send(connfd, ctime(&end_time), 24,  MSG_NOSIGNAL);
            s=" *** Total " + to_string(cnt) + " users online now. Your name is " + to_string(name) + "\n";
            char* cc=(char*)s.c_str();
            send(connfd, cc, 48,  MSG_NOSIGNAL);//buffer!!

			//告訴其他user有誰連線
			s=" *** User " + to_string(name) + " has just landed on the server\n";
			cc=(char*)s.c_str();
			for(int j = 0; j <= maxi; j++){
				if (FD_ISSET(client[j], &allset)){
					if (client[j] != listenfd && client[j] != connfd) {
						send(client[j], ctime(&end_time), 24,  MSG_NOSIGNAL);
						if (send(client[j], cc, s.length(),  MSG_NOSIGNAL) == -1) {
							//perror("send");
						}
					}
				}
			}
			///////////////////////////////////////////////////////

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
            // send(connfd, ctime(&end_time), 24, 0);
		}
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0){
				continue;
            }
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
					/*4connection closed by client */
					//告訴其他user誰斷線了
					string s=" *** User " + m[sockfd] + " has left the server\n";
					char* cc=(char*)s.c_str();
					for(int j = 0; j <= maxi; j++){
						if (FD_ISSET(client[j], &allset)){
							if (client[j] != listenfd && client[j] != sockfd) {
								send(client[j], ctime(&end_time), 24, MSG_NOSIGNAL);
								if (send(client[j], cc, s.length(),  MSG_NOSIGNAL) == -1) {
									//perror("send");
								}
							}
						}
					}
					// * client 140.113.1.1:38864 disconnected
					// printf("* client %s:%d disconnected\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
					char *addport = (char*)addr[sockfd].c_str();
					printf("* client %s disconnected\n",addport);
					close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} 
				else{
					//全頻聊天
					// cout<<n<<"\n";
					// /name\n
					if(buf[0]=='/'){
						if(buf[1]=='n' && buf[2]=='a' && buf[3]=='m' && buf[4]=='e'){
							// /name 
							if(n == 6 && buf[5] == '\n'){
								send(sockfd, ctime(&end_time), 24,  MSG_NOSIGNAL);
								string error=buf;
								cout<<error<<"\n";
								string s=" *** Unknown or incomplete command :"+error;
								char* cc=(char*)s.c_str();
								send(sockfd,cc, s.length(),  MSG_NOSIGNAL);
							}
							else if(n == 7 && buf[5] == ' '){
								// 2022-10-05 16:13:40 *** Unknown or incomplete command </name>
								send(sockfd, ctime(&end_time), 24,  MSG_NOSIGNAL);
								string s=" *** No nickname given \n";
								char* cc=(char*)s.c_str();
								send(sockfd,cc, s.length(),  MSG_NOSIGNAL);
							}
							else{
								string s,oldname,newname;
								send(sockfd, ctime(&end_time), 24,  MSG_NOSIGNAL);
								oldname=m[sockfd];
								newname=buf;
								newname=newname.substr(6);
								newname=newname.erase(newname.length()-1);
								m[sockfd]=newname;
								s=" *** Nickname changed to "+newname+'\n';
								char* cc=(char*)s.c_str();
								send(sockfd,cc, s.length(),  MSG_NOSIGNAL);
								//傳給其他人
								for(int j = 0; j <= maxi; j++){
									if (FD_ISSET(client[j], &allset) && client[j]!=sockfd){
											send(client[j], ctime(&end_time), 24,  MSG_NOSIGNAL);
											s=" *** User "+oldname+" renamed to "+newname+'\n';
											char* ccc=(char*)s.c_str();
											send(client[j],ccc, s.length(), MSG_NOSIGNAL);
									}
								}
							}
						}
						else if (buf[1]=='w' && buf[2]=='h' && buf[3]=='o'){
							// /who
							send(sockfd,"--------------------------------------------------\n", 52,  MSG_NOSIGNAL);
							for(int j = 0; j <= maxi; j++){
								if (FD_ISSET(client[j], &allset)){
									string s;
									if(client[j]==sockfd) s='*'+m[client[j]]+"       "+addr[client[j]]+'\n';
									else s=' '+m[client[j]]+"       "+addr[client[j]]+'\n';
									char* cc=(char*)s.c_str();
									send(sockfd,cc, s.length(),  MSG_NOSIGNAL);
								}
							}
							send(sockfd,"--------------------------------------------------\n", 52,  MSG_NOSIGNAL);
						}
						else{
							//Unknown or incomplete command
							send(sockfd, ctime(&end_time), 24,  MSG_NOSIGNAL);
							string error=buf;
							string s=" *** Unknown or incomplete command :"+error;
							char* ccc=(char*)s.c_str();
							send(sockfd,ccc, s.length(),  MSG_NOSIGNAL);
						}
					}
					else{
						for(int j = 0; j <= maxi; j++) {
							if (FD_ISSET(client[j], &allset)) {
								if (client[j] != listenfd && client[j] != sockfd) {
									send(client[j], ctime(&end_time), 24, MSG_NOSIGNAL);
									string s=' '+m[sockfd]+' ';
									char* cc=(char*)s.c_str();
									send(client[j],cc, s.length(), MSG_NOSIGNAL);
									if (send(client[j], buf, n,  MSG_NOSIGNAL) == -1) {
										//perror("send");
									}
								}
							}
						}
						bzero(buf , sizeof(buf));
					}
				}
				if (--nready <= 0)
					break;				/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */

