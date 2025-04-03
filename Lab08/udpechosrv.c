/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/stat.h>
int check[1000];// check seq num
#define datasize 1000
#define err_quit(m) { perror(m); exit(-1); }
struct data
{
        int s;
        int id;
        int ed;
        int size;
        unsigned int check;
        char buf[datasize+1];
};
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
//	while(sum>>16)sum = (sum>>16) + (sum&0x00ff);
	return (unsigned int)(sum);
}
int main(int argc, char *argv[]) {
	int s;
	struct sockaddr_in sin;

	if(argc < 2) {
		return -fprintf(stderr, "usage: %s ... <port>\n", argv[0]);
	}

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-1], NULL, 0));

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0)
		err_quit("bind");
        int num=0;
        char name[14];
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

	while(1) {
	
	    if(num==atoi(argv[3]))break;
		struct sockaddr_in csin;
		socklen_t csinlen = sizeof(csin);
		char buf[2048];
		int rlen;
		struct data tp;
		if((rlen = recvfrom(s, &tp, sizeof(tp), 0, (struct sockaddr*) &csin, &csinlen)) < 0) {
			perror("recvfrom");
			break;
		}
		
		if(tp.s==0)continue;//if start to get new file
		//{
		        FILE *w;
		        //printf("%s\n",name);
		        w = fopen(name,"w");
		        if(w==NULL)
		        {
		            printf("nofile\n");
		            continue;
		        }
		        
		    	while(1)
		    	{
		    		char buff[2048];
		    		int rrlen;
		    		struct data tmp;
		    		rrlen = recvfrom(s, &tmp, sizeof(tmp), 0, (struct sockaddr*) &csin, &csinlen);
		    		
		    		if(tmp.ed==1)break;//get end data
		    		if(tmp.s==1 || check[tmp.id]==1)continue;//get start data or seq that already get
		    		unsigned int checkd = check_sum((short *)tmp.buf, tmp.size);
	    			unsigned int cc = (checkd & tmp.check);
		    		if(cc)//not ok checksum
		    		{
		    			//printf("incorrect check sum\n");
		    			printf("%u",cc);
		    			//printf("id %d receive %u, get %u\n",tmp.id,tmp.check, checkd);
		    		    continue;
		    		}
		    		
		    		check[tmp.id] = 1;//get seq num
		    		
		    		fwrite(tmp.buf,1,tmp.size,w);
		    	}
		    	//printf("here\n");
		        num++;
		        int tmp = num;
		        
		        int t = 12;
		        //int t = 5;
		        fclose(w);
		        struct stat st;
		        stat(name,&st);
		        int size = st.st_size;
		        printf("%s %d\n",name,size);
		        while(num)//use to change write file name
		        {
		            int a = num%10;
		            name[t] = a + '0';
		            t--;
		            num/=10;
		        }
		        num = tmp;
		        for(int i=0;i<1000;i++)check[i] = 0;//reset seq 
		       // printf("%s",name);
		//}
		
	}

	close(s);
}
