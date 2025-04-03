#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h> 
#include <unistd.h>  
#include <cstring>
#include<fcntl.h>
#include <bits/stdc++.h> 
using namespace std; 

static struct timeval _t0;
static unsigned long long bytesent = 0;

double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

void handler(int s) {
	struct timeval _t1;
	double t0, t1;
	gettimeofday(&_t1, NULL);
	t0 = tv2s(&_t0);
	t1 = tv2s(&_t1);
	fprintf(stderr, "\n%lu.%06lu %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
		_t1.tv_sec, _t1.tv_usec, bytesent, t1-t0, 8.0*(bytesent/1000000.0)/(t1-t0), (bytesent/1000000.0)/(t1-t0));
	exit(0);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in server;  
  	int sock;
	char buf[300];
	float times=atof(argv[1]);
	signal(SIGINT,  handler);
	signal(SIGTERM, handler);
	/* 製作 socket */  
	sock = socket(AF_INET, SOCK_STREAM, 0);  
	
	/* 準備連線端指定用的 struct 資料 */  
	server.sin_family = AF_INET;  
	server.sin_port = htons(10003);  
	
	inet_pton(AF_INET, "140.113.213.213", &server.sin_addr.s_addr);  
	// inet_pton(AF_INET, "127.0.0.1", &server.sin_addr.s_addr);  
	
	/* 與 server 端連線 */  
	connect(sock, (struct sockaddr *)&server, sizeof(server));  
	/* 從伺服器接受資料 */  
	memset(buf, 0, sizeof(buf));  
	read(sock, buf, sizeof(buf)); 

	gettimeofday(&_t0, NULL);
	char c[1000];
	fill(c,c+1000,'!');
	clock_t begin,end;
	while(1) {
		begin=clock();
		// struct timespec t = { 0, 1 };
		for(int i=0;i<times*1000;i++){
			send(sock,(char*)&c,950,0);
		}
		end=clock();
		sleep((1-((begin-end)/CLOCKS_PER_SEC)));
	}

	return 0;
}
