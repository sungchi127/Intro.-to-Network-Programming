#include <arpa/inet.h>
#include <dirent.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>

#define SIZE 1024
#define PCKT_LEN 8192

using namespace std;

int sd;
char buffer[PCKT_LEN];
struct iphdr *ip = (struct iphdr *)buffer;
char *dat = (char *)buffer + sizeof(struct iphdr) ;
char name[14] ;

unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--) sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void Send(char msg[], struct sockaddr_in sin) {
    strcpy(dat, msg);
    sendto(sd, buffer, ip->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
    usleep(1000);
}

void send_file_data(FILE *fp, struct sockaddr_in sin) {
    int cnt = 0;
    char buffer[SIZE];
    bzero(buffer, SIZE);
    string msg;
    while (fgets(buffer, SIZE, fp)){
        msg = to_string(cnt) + " " + string(buffer);
        Send((char*)msg.c_str(), sin);
        bzero(buffer, SIZE);
        cnt++;
    }
    msg = "END\n";
    Send((char*)msg.c_str(), sin);
}

int main(int argc, char const *argv[]) {
    u_int16_t src_port, dst_port;
    u_int32_t src_addr, dst_addr;
    src_addr = inet_addr(argv[3]);
    dst_addr = inet_addr(argv[3]);

    if ((sd = socket(AF_INET, SOCK_RAW, 161)) < 0) {
        perror("socket() error");
        exit(2);
    }

    int enable = 1;
    if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &enable, sizeof(enable)) < 0)
        perror("setsockopt() error");

    if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0)
        perror("setsockopt");


    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(dst_port);
    sin.sin_addr.s_addr = dst_addr;

    memset(buffer, 0, PCKT_LEN);

    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 16;
    ip->tot_len = sizeof(struct iphdr) + 1300;
    ip->id = htons(54321);
    ip->ttl = 64;
    ip->protocol = 161;
    ip->saddr = src_addr;
    ip->daddr = dst_addr;
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
    for(int i=0;i<1000;i++) {
        FILE *fp;
        fp = fopen(name, "r");
        Send(name, sin);

        send_file_data(fp, sin);

        int num = i+1;
        int t = 12;
        while(num){//use to change open file name
            int a = num%10;
            name[t] = a + '0';
            t--;
            num/=10;
        }
        fclose(fp);
    }

    sleep(2);
    close(sd);
    return 0;
}