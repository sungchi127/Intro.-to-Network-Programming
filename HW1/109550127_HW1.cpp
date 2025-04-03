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
//1 2 6 7 8 9 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void hello(int cnt,string name,int sockfd);

/* include fig01 */
int main(int argc, char **argv)
{
    auto start = std::chrono::system_clock::now();
    time_t end_time = chrono::system_clock::to_time_t(start);
    int MAXLINE=4096;
	int					i, maxi, maxfd, listenfd, connfd, sockfd,cnt,chnum=0;
	int					nready, client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	map<int,string> m;
	map<string,int> topicb;
	map<string,string> topic;
	map<string,set<string>> chset;
	set<string> chlist;
	set<string> nameset;
	string addr[2000];
	signal(SIGPIPE,SIG_IGN);
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	int x=0;
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
		int cnt;
		if (FD_ISSET(listenfd, &rset)){	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
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
			// cout<<maxi<<"\n";
			//計算在線人數
			int num=1;

			for(int i = 0; i <= maxi; i++){
				if (FD_ISSET(client[i], &allset)){
					if (client[i] != listenfd && client[i] != connfd) {
						num++;
					}
				}
			}
			cnt=num;
			///////////////////////////////////////////////////////

			if (--nready <= 0)
				continue;				/* no more readable descriptors */
			
		}
		
		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0){
				continue;
            }
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
					/*4 connection closed by client */

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
					cout<<buf<<"----------"<<"\n";
					//CAP
					if(buf[0]=='C' && buf[1]=='A'){
						string s=":mircd 421  CAP :Unknown command\n";
						char* cc=(char*)s.c_str();
						send(sockfd, cc, s.length(),  MSG_NOSIGNAL);//buffer!!
						// string name=buf;
						// int nindex=name.find("\n");
						// name=name.substr(nindex+1);//NICK開始
						// int rindex=name.find("\r");
						// int num=rindex-5;
						// name = name.substr(5,num);
						// m[sockfd]=name;
						// hello(cnt, m[sockfd] ,sockfd);
						// bzero(buf , sizeof(buf));						
						continue;
					}
					//NICK
					if(buf[0]=='N' && buf[1]=='I' ){
						// (431) ERR_NONICKNAMEGIVEN
						string s=buf;
						if(s.length()==5){
							string s=":mircd 431 :No nickname given\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(),  MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						string name=buf;
						int rindex=name.find("\r");
						int num=rindex-5;
						name = name.substr(5,num);
						if(name[name.length()-1]=='\n'){
							name=name.erase(name.length()-1);
						}
						// (436) ERR_NICKCOLLISION
						// cout<<name<<name.length();
						if(s.length()==11 && x==1){
							string s=":mircd 436 "+ name + ":Nickname collision KILL\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(),  MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;						
						}
						// if(name[name.length()-1]=='\n'){
						// 	name=name.erase(name.length()-1);
						// }
						m[sockfd]=name;
						nameset.insert(name);
						//hello message user online
						hello(cnt, m[sockfd] ,sockfd);
						bzero(buf , sizeof(buf));
						cout<<"1111111"<<"\n";
						x++;
						continue;
					}
					//USER
					if(buf[0]=='U' && buf[4]==' '){
						// (461) ERR_NEEDMOREPARAMS
						cout<<"22222222"<<"\n";
						string s=buf;
						if(s.length()==5){
							string s=":mircd 461 "+ m[sockfd] + " USER :Not enought parameters\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(),  MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;	
						}
						cout<<"333333333"<<"\n";
						bzero(buf , sizeof(buf));
						continue;
					}
					//PING
					if(buf[0]=='P' && buf[1]=='I' && buf[2]=='N' && buf[3]=='G'){
						string serv=buf;
						serv=serv.substr(5);
						serv=serv.erase(serv.length()-2);
						string s="PONG "+serv+"\n";
						char* c=(char*)s.c_str();
						send(sockfd, c, s.length(),  MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));
						continue;
					}
					//USERS
					if(buf[0]=='U' && buf[4]=='S'){
						string s=":mircd 392 " + m[sockfd] + " :UserID                           Terminal  Host\n";
						char* c=(char*)s.c_str();
						send(sockfd, c, s.length(),  MSG_NOSIGNAL);
						for(int j = 0; j <= maxi; j++){
							if (FD_ISSET(client[j], &allset)){
								s=":mircd 393 " + m[sockfd] + " :" + m[client[j]] + "                            -         localhost\n";
								char* c=(char*)s.c_str();
								send(sockfd, c, s.length(),  MSG_NOSIGNAL);
							}
						}
						s=":mircd 394 " + m[sockfd] + " :End of users\n";
						char* cc=(char*)s.c_str();
						send(sockfd, cc, s.length(),  MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));
						continue;
					}
					//LIST
					if(buf[0]=='L' && buf[1]=='I'){
						string s=":mircd 321 " + m[sockfd] + " Channel :Users Name\n";
						char* c=(char*)s.c_str();
						send(sockfd, c, s.length(), MSG_NOSIGNAL);
						// bzero(buf , sizeof(buf));
						for(auto &it : chlist){
							cout<<"it:"<<it<<" topic[it]:"<<topic[it] <<"////////// "<<"\n";
							s=":mircd 322 " + m[sockfd] + " #" + it + " " + to_string(chset[it].size()) + " :" + topic[it] + "\n"; 
							char* cc=(char*)s.c_str();
							send(sockfd, cc, s.length(), MSG_NOSIGNAL);
							// bzero(buf , sizeof(buf));
						}
						s=":mircd 323 " + m[sockfd] +" :End of Liset\n";
						char* cc=(char*)s.c_str();
						send(sockfd, cc, s.length(), MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));
						continue;
					}
					//JOIN
					if(buf[0]=='J' && buf[1]=='O'){
						// :user2 JOIN #sao
						string chan=buf;
						chan=chan.substr(6);
						chan=chan.erase(chan.length()-2);
						string s=":" + m[sockfd] +" JOIN #"+ chan +"\n";
						char* c=(char*)s.c_str();
						send(sockfd,c, s.length(), MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));
						
						//將新增的channel插入channel list裡			
						chlist.insert(chan);
						//如果使用者進入某一個channel就把它放進那個channel的set
						chset[chan].insert(m[sockfd]);

						if(topicb[chan]==1){
							// :mircd 332 test1 #123 :test 1
							string s=":mircd 332 " + m[sockfd] + " #" + chan + " :" + topic[chan] + "\n";
							char* cc=(char*)s.c_str();
							send(sockfd, cc, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
						}
						else{
							// :mircd 331 user2 #sao :No topic is set
							string s=":mircd 331 " + m[sockfd] + " #" + chan + " :No topic is set\n";
							char* cc=(char*)s.c_str();
							send(sockfd, cc, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
						}
						// :mircd 353 user2 #sao :user2
						// :mircd 366 user2 #sao :End of Names List
						string list;
						for(auto &it : chset[chan]){
							list=list+it+" ";
						}
						s=":mircd 353 " + m[sockfd] + " #" + chan + " :" + list + "\n";
						char* cc=(char*)s.c_str();
						send(sockfd, cc, s.length(), MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));

						s=":mircd 366 " + m[sockfd] + " #" + chan + " :End of Names List\n";
						char* ccc=(char*)s.c_str();
						send(sockfd, ccc, s.length(), MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));
						continue;
					}
					//TOPIC
					if(buf[0]=='T' && buf[1]=='O'){
						// :mircd 332 user2 #sao :sao
						string t=buf,s;
						t=t.substr(6);
						int colon=t.find(":");
						t=t.erase(t.length()-2);
						// cout<<t[colon]<<" "<<t[colon+1]<<" "<<colon<<"\n"; 
						if(t[colon+1]!='h'){
							string chan=t,meg=t;
							chan=chan.substr(1,colon-2);
							meg=meg.substr(colon+1);
							cout<<"chan:"<<chan<<"meg:"<<meg;
							s=":mircd 332 " + m[sockfd] + " #" + chan+ " :"+ topic[chan]  + "\n";
							char* cc=(char*)s.c_str();
							send(sockfd, cc, s.length(), MSG_NOSIGNAL);
							cout<<"YES"<<"\n";
							continue;
						}
						// t=t.substr(6);
						// t=t.erase(t.length()-2);
						s=":mircd 332 " + m[sockfd] + " " + t + "\n";
						char* cc=(char*)s.c_str();
						send(sockfd, cc, s.length(), MSG_NOSIGNAL);
						bzero(buf , sizeof(buf));
						// int colon=t.find(":");
						cout<<"colon:"<<colon<<"\n";
						cout<<"t:"<<t<<"\n";
						string chan=t,meg=t;
						chan=chan.substr(1,colon-2);
						// cout<<"chan:"<<chan<<"\n";
						meg=meg.substr(colon+1);
						// (442) ERR_NOTONCHANNEL
						if(chset[chan].find(m[sockfd])==chset[chan].end()){
							string s=":mircd 442 " + m[sockfd] + " #" + chan + " :You are not on that channel\n";
							char* cc=(char*)s.c_str();
							send(sockfd, cc, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						// cout<<"meg:"<<meg<<"\n";
						topicb[chan] = 1;
						topic[chan] = meg;
						cout<<"topic[chan]:"<<topic[chan]<<"\n";
						continue;
					}
					//PART
					if(buf[0]=='P' && buf[1]=='A' && buf[2]=='R' && buf[3]=='T'){
						string channel=buf;
						int colon=channel.find(":");
						channel=channel.substr(5,colon-6);//有#
						cout<<"m[sockfd]:"<<m[sockfd]<<" channel:"<<channel<<"\n";
						// (403) ERR_NOSUCHCHANNE
						if(chlist.find(channel)==chlist.end()){
							string s=":mircd 403 " + m[sockfd]+ " " + channel + " :No such channel\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						// (442) ERR_NOTONCHANNEL
						if(chset[channel].find(m[sockfd])==chset[channel].end()){
							string s=":mircd 442 " + m[sockfd] + " #" + channel + " :You are not on that channel\n";
							char* cc=(char*)s.c_str();
							send(sockfd, cc, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						string s=":" + m[sockfd] + " PART :" + channel +"\n";
						char* c=(char*)s.c_str();
						send(sockfd, c, s.length(), MSG_NOSIGNAL);

						bzero(buf , sizeof(buf));
						continue;
					}
					// PRIVMSG
					if(buf[0]=='P' && buf[1]=='R' && buf[2]=='I' && buf[3]=='V'){
						//打字聊天
						// (411) ERR_NORECIPIENT
						if(buf[8]!='#'){
							string s=":mircd 411 "+ m[sockfd] + " :No recipient given (PRIVMSG)\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						string meg=buf,chan=buf;
						chan=chan.substr(9);
						int colon=chan.find(":");
						chan=chan.substr(0,colon-1);
						// (412) ERR_NOTEXTTOSEND
						if(colon==string::npos ){
							string s=":mircd 412 "+ m[sockfd] + " :No text to send\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						meg=meg.substr(8);
						meg=meg.erase(meg.length()-2);
						// :user2 PRIVMSG #t :ss
						cout<<"chan:"<<chan<<"\n";
						// (401) ERR_NOSUCHNICK
						if(chlist.find(chan)==chlist.end()){
							string s=":mircd 401 "+ m[sockfd]+ " #" + chan + " :No such nick/channel\n";
							char* c=(char*)s.c_str();
							send(sockfd, c, s.length(), MSG_NOSIGNAL);
							bzero(buf , sizeof(buf));
							continue;
						}
						else{
							string s=":" + m[sockfd] + " PRIVMSG " + meg +"\n";
							char* cc=(char*)s.c_str();
							for(int j = 0; j <= maxi; j++) {
								if (FD_ISSET(client[j], &allset)) {
									if (chset[chan].find(m[client[j]])!=chset[chan].end() && client[j]!=sockfd) {
										cout<<m[client[j]]<<"\n";
										send(client[j] ,cc, s.length(), MSG_NOSIGNAL);
									}
								}
							}
						}
						bzero(buf , sizeof(buf));
						continue;
					}
					else{
						// (421) ERR_UNKNOWNCOMMAND
						string com=buf;
						com=com.erase(com.length()-2);
						string s=":mircd 421 "+ m[sockfd]+ " " + com + " :Unknown command\n";
						char* c=(char*)s.c_str();
						send(sockfd, c, s.length(), MSG_NOSIGNAL);
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

void hello(int cnt,string name,int sockfd){
	string s[20];
	// cout<<"cnt:"<<cnt<<"\n";
	s[0]=":mircd 001 "+ name + " :Welcome to the minimized IRC daemon!\n";
	s[1]=":mircd 251 "+ name + " :There are " + to_string(cnt) + " users and 0 invisible on 1 server\n";
	s[2]=":mircd 375 " + name + " :- mircd Message of the day -\n";
	s[3]=":mircd 372 " + name + " :-  Hello, World!\n";
	s[4]=":mircd 372 " + name + " :-               @                    _ \n";
	s[5]=":mircd 372 " + name + " :-   ____  ___   _   _ _   ____.     | |\n";
	s[6]=":mircd 372 " + name + " :-  /  _ `'_  \\ | | | '_/ /  __|  ___| |\n";
	s[7]=":mircd 372 " + name + " :-  | | | | | | | | | |   | |    /  _  |\n";
	s[8]=":mircd 372 " + name + " :-  | | | | | | | | | |   | |__  | |_| |\n";
	s[9]=":mircd 372 " + name + " :-  |_| |_| |_| |_| |_|   \\____| \\___,_|\n";
	s[10]=":mircd 372 " + name + " :-  minimized internet relay chat daemon\n";
	s[11]=":mircd 372 " + name + " :-\n";
	s[12]=":mircd 376 " + name + " :End of message of the day\n";
	for(int i=0; i<13 ;i++){
		char* c=(char*)s[i].c_str();
		send(sockfd, c, s[i].length(), MSG_NOSIGNAL);
	}
}