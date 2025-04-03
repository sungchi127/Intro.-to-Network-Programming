#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#define SIZE 1024
#define PCKT_LEN 8192

using namespace std;


int main(int argc, char const *argv[]) {
    u_int16_t src_port, dst_port;
    u_int32_t src_addr, dst_addr;
    src_addr = inet_addr(argv[3]);
    dst_addr = inet_addr(argv[3]);

    int sd;
    char buffer[PCKT_LEN];
    memset(buffer, 0, PCKT_LEN);
    struct iphdr *ip = (struct iphdr *)buffer;


    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(sin);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(dst_port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((sd = socket(AF_INET, SOCK_RAW, 161)) < 0) {
        perror("socket() error");
        exit(2);
    }

    int enable = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    // ----------------------------------------------
    int n = 0, current_number;
    string number, msg;
    FILE *fp;
    for (int i = 0; i < atoi(argv[2]); i++) {
        current_number = 0;

        recvfrom(sd, buffer,1300 + sizeof(struct iphdr) , 0,(struct sockaddr *)&sin, &addrlen);
        char *data = buffer + sizeof(struct iphdr);
        char* name =data;
        fp = fopen(name, "w");

        while (1) {
            number.clear();
            recvfrom(sd, buffer,1300 + sizeof(struct iphdr), 0,(struct sockaddr *)&sin, &addrlen);
            char *data = buffer + sizeof(struct iphdr);
            msg = string(data);

            if (msg == "END\n") break;
            for (n = 0; msg[n] != ' '; n++) number += msg[n];
            if (current_number != stoi(number)) continue;
            if (msg.back() == '\x07') msg.pop_back();

            msg = msg.substr(n + 1);

            fprintf(fp, "%s", msg.c_str());

            current_number++;
        }
        fclose(fp);
    }
    close(sd);
    return 0;
}