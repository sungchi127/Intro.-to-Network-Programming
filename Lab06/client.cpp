#include <sys/signal.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h> 
#include <unistd.h>  
#include <cstring>
#include<fcntl.h>
#include <bits/stdc++.h> 
#include<sys/time.h>

using namespace std; 
const int maxn=100;
static struct timeval start;
static unsigned long long bytesent = 0;
char c[maxn];
double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

void handler(int s) {
	struct timeval _t1;
	double t0, t1;
	gettimeofday(&_t1, NULL);
	t0 = tv2s(&start);
	t1 = tv2s(&_t1);
	fprintf(stderr, "\n%lu.%06lu %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
		_t1.tv_sec, _t1.tv_usec, bytesent, t1-t0, 8.0*(bytesent/1000000.0)/(t1-t0), (bytesent/1000000.0)/(t1-t0));
	exit(0);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server;  
  	int cmd_sock,data_sock[10];
    int port = atoi(argv[2]);
	// int times=atoi(argv[3]);
	char buf[50];
	signal(SIGINT,  handler);
	signal(SIGTERM, handler);
	/* 製作 socket */  
	cmd_sock = socket(AF_INET, SOCK_STREAM, 0);  
	server.sin_family = AF_INET;
	server.sin_port = htons(port);  
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);  
	connect(cmd_sock, (struct sockaddr *)&server, sizeof(server));  

	sleep(2);
	//data send
	server.sin_family = AF_INET;
	server.sin_port = htons(port+1);
	for(int i=0;i<10;i++){
		data_sock[i] = socket(AF_INET, SOCK_STREAM, 0);  
		inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);   
		connect(data_sock[i], (struct sockaddr *)&server, sizeof(server));
	}
		/* 傳送reset */  
	string s="/reset\n";
	char *res=(char*)s.c_str();
	send(cmd_sock, res, s.length(), 0);
	sleep(1);
	memset(buf, 0, sizeof(buf));
	recv(cmd_sock, (char*)&buf, 50, 0);
	cout<<buf<<"\n";
	memset(buf, 0, sizeof(buf)); 

	gettimeofday(&start, NULL);
	sleep(1);
	fill(c,c+(int)sizeof(c)-1,'!');
	c[(int)sizeof(c)-1]='\n';
	// clock_t begin,end;
	while(1) {
		for(int i=0;i<10;i++)
	    	send(data_sock[i],(char*)&c,sizeof(c),0);
        // bzero(c , sizeof(c));
	}

	return 0;
}
