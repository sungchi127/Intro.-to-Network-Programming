/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define err_quit(m) { perror(m); exit(-1); }

#define NIPQUAD(s)	((unsigned char *) &s)[0], \
					((unsigned char *) &s)[1], \
					((unsigned char *) &s)[2], \
					((unsigned char *) &s)[3]

#define sendrate 470 //use to decide how many times one packet need to send
#define datasize 1000

char name[14] ;
char *path;
static int s = -1;
static struct sockaddr_in sin;
static unsigned seq;
static unsigned count = 0;
struct data // the packet to send;
{
        int s; //s=1 to tell start to send;
        int id;// seq num
        int ed;// ed = 1 to tell no data to send;
        int size;// how many data
        unsigned int check;//checksum
        char buf[datasize+1];//data
};
typedef struct {
	unsigned seq;
	struct timeval tv;
}	ping_t;

double tv2ms(struct timeval *tv) {
	return 1000.0*tv->tv_sec + 0.001*tv->tv_usec;
}
// do checksum
unsigned int check_sum(unsigned short *a, int len)
{
	unsigned int sum = 0;
	while(len>1)
	{
		sum+=(*a);
		a++;
		len-=2;
	}
	if(len)sum+=*(unsigned char*)a;
	//while(sum>>16)sum = (sum>>16) + (sum&0x00ff);
	return (~sum);
}
//do send data
int stt ;
void do_send() {
	char buf[1024];
	FILE *o;
	struct stat st;
	o = fopen(name,"r");
	stat(name,&st);
	int size = st.st_size; //get file size
	
	if(o==NULL)
	{
	    printf("no file\n");
	    return;
	}
	printf("%s %d\n",name,size);
	
	int n;
	struct data tmp;
	int id = 0;
	int nu = size/datasize; // devide data to 100 bytes per packet;
	
	for(int k=0;k<nu;k++) // read data and send;
	{
	    fgets(tmp.buf,datasize+1,o);
	    // printf("%s\n",tmp.buf);
	    tmp.check = check_sum((short *)tmp.buf, datasize); //checksum
	    struct timeval start,end;
	    gettimeofday(&start,0);
	    int i = 0;
	    //send data
	    if(stt==0) i = -100000;
	    while(i<1000000/sendrate){
			tmp.id = id; 	
			tmp.s = 0;
			tmp.size = datasize;	    
		    i++;
		    tmp.ed = 0;
		    sendto(s, &tmp, sizeof(tmp), 0, (struct sockaddr*) &sin, sizeof(sin));
	    }
	    id++;
	}
	if(size%datasize!=0) // send last data
	{
	//printf("send\n");
		fgets(tmp.buf,size%datasize+1,o);
	    struct timeval start,end;
	    gettimeofday(&start,0);
	    int i = 0;
	    tmp.check = check_sum((short *)tmp.buf, size%datasize);
	    if(stt==0)i = -100000;
	    while(i<1000000/sendrate){
		 	tmp.id = id; 	
		 	tmp.s = 0;	    
		    i++;
		    tmp.ed = 0;
		    tmp.size = size%datasize;
		    sendto(s, &tmp, sizeof(tmp), 0, (struct sockaddr*) &sin, sizeof(sin));
	    }
	}
	
	int i = 0;
	struct timeval start,end;
	gettimeofday(&start,0);
	//send end data
	    while(i<1000000/sendrate){
			tmp.id = id; 	
		 	tmp.s = 0;	    
		    i++;
		    tmp.ed = 1;
		    sendto(s, &tmp, sizeof(tmp), 0, (struct sockaddr*) &sin, sizeof(sin));//file is all read 
	    }
	stt = 1;
	fclose(o);
	return;
}

int main(int argc, char *argv[]) {
	
        //file name
        name[0] = '/';
        name[1] = 'f';
        name[2] = 'i';
        name[3] = 'l';
        name[4] = 'e';
        name[5] = 's';
        name[6] = '/';
        name[7] = '0';
        name[8] = '0';
        name[9] = '0';
        name[10] = '0';
        name[11] = '0';
        name[12] = '0';
        name[13] = '\0';

	if(argc < 3) {
		return -fprintf(stderr, "usage: %s ... <port> <ip>\n", argv[0]);
	}
        
	srand(time(0) ^ getpid());

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-2], NULL, 0));
	if(inet_pton(AF_INET, argv[argc-1], &sin.sin_addr) != 1) {
		return -fprintf(stderr, "** cannot convert IPv4 address for %s\n", argv[argc-1]);
	}

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	for(int i=0;i<1000;i++) {
	        
		int rlen;
		struct sockaddr_in csin;
		socklen_t csinlen = sizeof(csin);
		path = argv[1];
		char buf[2048];
		struct timeval tv;
		ping_t *p = (ping_t *) buf;
		char c[1];
		c[0]= 's';
		struct timeval start,end;
		//gettimeofday(&start,0);
		struct data tmp;
	    int j=0;
	    //send start packet to tell start to send data
	    while(j < 1000000 / sendrate)
	    {
		//gettimeofday(&end,0);
		//int sec = end.tv_sec - start.tv_sec;
		//int usec = end.tv_usec - start.tv_usec;
			tmp.id = 0; 		    
		//if(usec  + sec* 1000000>1000)
		//{
		 //gettimeofday(&start,0);
		    j++;
		    tmp.s = 1;
		    sendto(s, &tmp, sizeof(tmp), 0, (struct sockaddr*) &sin, sizeof(sin));//file is all read 
		    //sendto(s, ok, 2, 0, (struct sockaddr*) &csin, sizeof(csin));
		//}
	    }

		//sendto(s, c, sizeof(c), 0, (struct sockaddr*) &sin, sizeof(sin)); //send start to server,new file need to write            
                //send data
		do_send();	
		int num = i+1;
		int t = 12;
		while(num){//use to change open file name
            int a = num%10;
	        name[t] = a + '0';
	        t--;
	        num/=10;
	    }
	}
	sleep(5);
	close(s);
}
