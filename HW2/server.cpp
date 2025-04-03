#include <bits/stdc++.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>   
#include <sys/socket.h>   
#include <netinet/in.h>   
#include <unistd.h>   
#include <errno.h>   
#include <string.h>
#include <string>   
#include <stdlib.h>
#include <set>
#include <cassert>
#include <arpa/inet.h>

using namespace std;

//Types of DNS resource records :)
#define T_A 1 //Ipv4 address
#define T_AAAA 28 //Ipv6 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
// #define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server
#define T_TXT 16 //Mail server


//Function Prototypes
void ngethostbyname(unsigned char*, int);
void ChangetoDnsNameFormat(unsigned char*, unsigned char*);
unsigned char* ReadName(unsigned char*, unsigned char*, int*);
void readConfigFile(string filepath);
struct RES_RECORD text_to_record(string text, vector<string> &domainName);
void reply_to_client(); 
void set_dns_header();
void set_payload();
string nameServer;

//DNS header structure
struct DNS_HEADER {
	unsigned short id; // identification number

    unsigned char qr :1; // query/response flag
    unsigned char opcode :4; // purpose of message
    unsigned char tc :1; // truncated message
    unsigned char rd :1; // recursion desired
    unsigned char z :1; // its z! reserved
    unsigned char ad :1; // authenticated data
    unsigned char aa :1; // authoritive answer

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION {
	unsigned short qtype;
	unsigned short qclass;
};

//Structure of a Query
typedef struct {
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;


//Pointers to resource record contents
struct RES_RECORD {
	vector<string> domainName;
    uint32_t ttl;
    uint16_t IN = 1;
    uint16_t DNS_type;
    uint16_t data_length = 0;
	vector<unsigned short> record_data;
    vector<string> subDomain;
};

struct Domain_Name{
    vector<string> Domain;
    vector<RES_RECORD> NSRecord;
    vector<RES_RECORD> SOARecord;
    vector<RES_RECORD> RRecord;
};

vector<Domain_Name> total_domain;
vector<RES_RECORD> matchRecord;
int sock_fd, pos=0 ,len=0;
string reply;
vector<unsigned short> payload;
struct DNS_HEADER reply_header;
struct sockaddr_in addr_serv,addr_client, reply_client,DNS_serv;

int main(int argc, char *argv[]) {
	set_payload();
    string path = argv[2];
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0) {
        perror("socket");  
        exit(1);  
    }
    
    // struct sockaddr_in addr_serv;  
    memset(&addr_serv, 0, sizeof(struct sockaddr_in));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_port = htons(1053);
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    len = sizeof(addr_serv);

    if (bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0) {  
        perror("bind error:");  
        exit(1);  
    }  
        
    // struct sockaddr_in addr_client, reply_client;

    // struct sockaddr_in DNS_serv;
    memset(&DNS_serv, 0, sizeof(struct sockaddr_in));
    bzero(&DNS_serv, sizeof(DNS_serv));
    DNS_serv.sin_family = AF_INET;
    DNS_serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    DNS_serv.sin_port = htons(53);
    //讀設定檔
    readConfigFile("test/config.txt");
	set_dns_header();
    while (true) {
        unsigned char buf[1300]={}, *qname, *reader,*name_pos;
        int stop=0,pos=0,anscnt=0,authcnt=0,addcnt=0;
		matchRecord.clear();
        memset (buf, 0, sizeof(buf));
        struct RES_RECORD answers[20], auth[20], addit[20]; //the replies from the DNS server
        struct DNS_HEADER *dns = NULL;
	    struct QUESTION *qinfo = NULL;
        //接收詢問query
        int data_len = recvfrom(sock_fd, buf, 1300, 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);
        printf("get message from [%s:%d]: \n", inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
        cout << "recv\n";

        //填header
        dns = (struct DNS_HEADER*) buf;
		cout<<dns->id<<"\n";
		cout<<htons(dns->id)<<"\n";
        //填hostname
        name_pos = &buf[sizeof(struct DNS_HEADER)];
        qname = ReadName(name_pos, buf, &stop);//3www6google3com -> www.google.com
        //填class跟type
        qinfo = (struct QUESTION*) &buf[sizeof(struct DNS_HEADER) + (strlen((const char*) qname)+2 )];
		qinfo->qclass=ntohs(qinfo->qclass);
        qinfo->qtype=ntohs(qinfo->qtype);

        printf("\nThe response contains : ");
        printf("\n %d Questions.", ntohs(dns->q_count));
        printf("\n %d Answers.", ntohs(dns->ans_count));
        printf("\n %d Authoritative Servers.", ntohs(dns->auth_count));
        printf("\n %d Additional records.\n\n", ntohs(dns->add_count));
		anscnt=ntohs(dns->ans_count);
		authcnt=ntohs(dns->auth_count);
		addcnt=ntohs(dns->add_count);
		//www.google.com -> com google www
		vector<string> vqname;
		string strqname = (char*)qname, tmp="";
		for(int i = 0; i < strqname.size(); i++){
			if (strqname[i]  == '.') {
                vqname.push_back(tmp);
                tmp = "";
            }
            else {
                tmp.push_back(strqname[i]);
            }
		}
		vqname.push_back(tmp);
		reverse(vqname.begin(), vqname.end());
		int flag=0;
		for(auto it:total_domain){
			if((int)sizeof(vqname) == (int)sizeof(it.Domain) && vqname[0]==it.Domain[0] && vqname[1]==it.Domain[1]){
				flag=1;
				if(qinfo->qtype==T_A || qinfo->qtype==T_SOA){
					reply="";
					reply_header.id=htons(dns->id);
					reply_header.auth_count=htons(1);
					reply.append((char*)&reply_header,sizeof(reply_header));
					reply.append((char*)qname);
					reply.append((char*)&qinfo,sizeof(qinfo));
					for(int i=0;i<reply.length();i++){
						printf ("%x, ", reply[i]);
					}
					// cout<<reply<<"\n";
					matchRecord.push_back(it.SOARecord[0]);
					reply_to_client();
				}
			}
			
		}				
		// int maxj=0,flag=0;
		// map<int,int> match;
		// for(int i = 0; i < (int)sizeof(vqname); i++){//詢問從 com google www
		// 	for(int j = 0; j < (int)sizeof(total_domain); j++){//每個domainname找一遍
		// 		for(int k = 0; k < (int)sizeof(total_domain[j].Domain)<k++){//domainname的vector每個元素依序比對
		// 			if(vqname[i]!=total_domain[j].Domain[k]){
		// 				break;
		// 			}
		// 			else{
		// 				match[j]++;
		// 			}
		// 		}
		// 		if(match[j] >= match[maxj] && match[j]==(int)sizeof(total_domain[j].Domain)){
		// 			maxj = j;
		// 			flag=1;
		// 		}
		// 	}
		// }





        // //sendto
        // sendto(sock_fd, ask, data_len, 0, (struct sockaddr *)&DNS_serv, len);
        // data_len = recvfrom(sock_fd, buf, 1200, 0, (struct sockaddr *)&reply_client, (socklen_t *)&len);
        // printf("get message from [%s:%d]: \n", inet_ntoa(reply_client.sin_addr), ntohs(reply_client.sin_port));
        // unsigned char reply[data_len + 5];
        // for (int i = 0; i < data_len; i++) {
        //     reply[i] = buf[i];
        //     printf ("%x, ", buf[i]);
        // }
        // sendto(sock_fd, reply, data_len, 0, (struct sockaddr *)&addr_client, len);
        // printf ("---\n");
    }
    close(sock_fd);
    
    return 0;  
}

void readConfigFile(string filepath) {
    ifstream config, txt;
    stringstream s, ss;
    string output;

    config.open(filepath);

    getline(config, nameServer);//8.8.8.8

    while (getline(config, output)) {
        s.clear(), s << output;
        getline(s, output, ',');

        Domain_Name domainName;
        ss.clear(), ss << output;
        while (getline(ss, output, '.')) {
            domainName.Domain.push_back(output);
			
        }
        reverse(domainName.Domain.begin(), domainName.Domain.end());
		

        getline(s, output, '\r');
        txt.open(output);

        getline(txt, output);
        while (getline(txt, output)) {
            s.clear(), s << output;
            getline(s, output, '\r');

            RES_RECORD RD = text_to_record(output, domainName.Domain);
            if (RD.DNS_type == T_NS) {
                domainName.NSRecord.push_back(RD);
            } else if (RD.DNS_type == T_SOA) {
                domainName.SOARecord.push_back(RD);
            }
            domainName.RRecord.push_back(RD);
        }
        total_domain.push_back(domainName);
        txt.close();
		
    }
    config.close();
}

RES_RECORD text_to_record(string text, vector<string> &domainName) {
    RES_RECORD now;
    now.domainName = domainName;
    vector<string> temp;
    string tmp = "";
    for (int i = 0; i < text.size(); i++) {
        if (text[i] == ',') {
            temp.push_back(tmp);
            tmp = "";
        }
        else {
            tmp.push_back(text[i]);
        }
    }
    temp.push_back(tmp);
    string record_type = temp[3];
    now.ttl = atoi(temp[1].c_str());
    if (temp[0] != "@") now.domainName.push_back(temp[0]);
    if (record_type == "A") {
        now.DNS_type = T_A;   
        uint32_t ipv4 = inet_addr(temp[4].c_str());
        now.data_length = 4;
        for (int i = 0; i < 4; i++) {
            now.record_data.push_back(ipv4 % 256);
            ipv4 /= 256;
        }
    }
    else if (record_type == "AAAA") {
        now.DNS_type = T_AAAA;
        now.data_length = 16;
        temp[4].push_back(':');
        int ipv6 = 0;
        int doublecolon = 16;
        vector<unsigned char> ttmp;
        for (int i = 0; i < temp[4].size(); i++) {
            if (temp[4][i] == ':') {
                if (ipv6 == 0) doublecolon = ttmp.size();
                ttmp.push_back(ipv6 / 256);
                ttmp.push_back(ipv6 % 256);
                ipv6 = 0;
            }
            else {
                int add = 0;
                if (temp[4][i] >= 'a' && temp[4][i] <= 'f') {
                    add = temp[4][i] - 'a' + 10;//轉成 10 11 12....
                }
                else {
                    add = temp[4][i] - '0';
                }
                ipv6 = ipv6 * 16 + add;
            }
        }
        for (int i = 0; i < doublecolon; i++) {
            now.record_data.push_back(ttmp[i]);
        }
        for (int i = 0; i < 16 - ttmp.size(); i++) {
            now.record_data.push_back(0);
        } 
        for (int i = doublecolon; i < ttmp.size(); i++) {
            now.record_data.push_back(ttmp[i]);
        }
    }
    else if (record_type == "NS")  {
        now.DNS_type = T_NS;
        tmp = "";
        for (int i = 0; i < temp[4].size(); i++) {
            if (temp[4][i]  == '.') {
                now.subDomain.push_back(tmp);
                tmp = "";
            }
            else {
                tmp.push_back(temp[4][i]);
            }
        }
        for (string x : now.subDomain) {
            now.data_length += (1 + x.size());
            now.record_data.push_back(x.size());
            for (int i = 0; i < x.size(); i++) {
                now.record_data.push_back(x[i]);
            }
        }
        now.data_length += 1;
        now.record_data.push_back(0);
    }
    else if (record_type == "CNAME") {
        now.DNS_type = T_CNAME;
        tmp = "";
        for (int i = 0; i < temp[4].size(); i++) {
            if (temp[4][i]  == '.') {
                now.subDomain.push_back(tmp);
                tmp = "";
            }
            else {
                tmp.push_back(temp[4][i]);
            }
        }
        for (string x : now.subDomain) {
            now.data_length += (1 + x.size());
            now.record_data.push_back(x.size());
            for (int i = 0; i < x.size(); i++) {
                now.record_data.push_back(x[i]);
            }
        }
        now.data_length += 1;
        now.record_data.push_back(0);
    }
    else if (record_type == "TXT") {
        now.DNS_type = T_TXT;
        now.data_length = temp[4].size() - 1;
        now.record_data.push_back(temp[4].size() - 2);
        for (int i = 1; i < temp[4].size() - 1; i++) {
            now.record_data.push_back(temp[4][i]);
        }
    }
    else if (record_type == "SOA") {
        now.DNS_type = T_SOA;
        tmp = "";
        vector<string> SOAInfo;
        for (int i = 0; i < temp[4].size(); i++) {
            if (temp[4][i]  == ' ') {
                SOAInfo.push_back(tmp);
                tmp = "";
            }
            else {
                tmp.push_back(temp[4][i]);
            }
        }
        SOAInfo.push_back(tmp);
        for (int i = 0; i < 2; i++) {
            tmp = "";
            for (int j = 0; j < SOAInfo[i].size(); j++) {
                if (SOAInfo[i][j] == '.') {
                    now.data_length += (tmp.size() + 1);
                    now.record_data.push_back(tmp.size());
                    for (char c : tmp) {
                        now.record_data.push_back(c);
                    }
                    tmp = "";
                }
                else {
                    tmp.push_back(SOAInfo[i][j]);
                }
            }
            now.data_length += 1;
            now.record_data.push_back(0);
        }
        for (int i = 2; i < 7; i++) {
            uint32_t SOAVal = 0;
            for (char c : SOAInfo[i]) SOAVal = SOAVal * 10 + (c - '0');
            vector<uint8_t> val_tmp;
            for (int j = 0; j < 4; j++) {
                val_tmp.push_back(SOAVal % 256);
                SOAVal /= 256;
            }
            for (int j = 3; j >= 0; j--) now.record_data.push_back(val_tmp[j]);
            now.data_length += 4;
        }
    }
    else {
        now.DNS_type = T_MX;
        vector<string> mxInfo;
        tmp = "";
        for (int i = 0; i < temp[4].size(); i++) {
            if (temp[4][i]  == ' ') {
                mxInfo.push_back(tmp);
                tmp = "";
            }
            else {
                tmp.push_back(temp[4][i]);
            }
        }
        mxInfo.push_back(tmp);
        tmp = "";
        now.data_length += 2;
        uint16_t prefer = atoi(mxInfo[0].c_str());
        now.record_data.push_back(prefer / 256);
        now.record_data.push_back(prefer % 256);
        for (int i = 0; i < mxInfo[1].size(); i++) {
            if (mxInfo[1][i]  == '.') {
                now.subDomain.push_back(tmp);
                tmp = "";
            }
            else {
                tmp.push_back(mxInfo[1][i]);
            }
        }
        for (string x : now.subDomain) {
            now.data_length += (1 + x.size());
            now.record_data.push_back(x.size());
            for (int i = 0; i < x.size(); i++) {
                now.record_data.push_back(x[i]);
            }
        }
        now.data_length += 1;
        now.record_data.push_back(0);
    }
    return now;
}

void reply_to_client() {
    vector<unsigned char> Mes;
    for (RES_RECORD RD : matchRecord) {
        for (int i = RD.domainName.size() - 1; i >= 0; i--) {
            string x = RD.domainName[i];
            Mes.push_back(x.size());
            for (char c : x) Mes.push_back(c);
        }
        Mes.push_back(0);
        Mes.push_back(RD.DNS_type / 256);
        Mes.push_back(RD.DNS_type % 256);
        Mes.push_back(RD.IN / 256);
        Mes.push_back(RD.IN % 256);
        uint32_t ttl_temp = RD.ttl;
        uint8_t arr[4];
        for (int i = 3; i >= 0; i--) {
            arr[i] = ttl_temp % 256;
            ttl_temp /= 256;
        }
        for (int i = 0; i < 4; i++) Mes.push_back(arr[i]);
        Mes.push_back(RD.data_length / 256);
        Mes.push_back(RD.data_length % 256);
        for (int i = 0; i < RD.record_data.size(); i++) {
            Mes.push_back(RD.record_data[i]);
        }
    }
    pos++;
    for (int i = 0; i < Mes.size(); i++) {
        reply.append((char*)&Mes[i]);
        pos = pos + 1;
    }
    for (int i = 0; i < 11; i++) {
        reply.append((char*)&payload[i]);
        pos = pos + 1;
    }
    
    sendto(sock_fd, reply.c_str(), reply.length(), 0, (struct sockaddr *)&addr_client, len);
    
}

// void ngethostbyname(unsigned char *host, int query_type) {
// 	unsigned char buf[65536], *qname, *reader;
// 	int i, j, stop, s;

// 	struct sockaddr_in a;

// 	struct RES_RECORD answers[20], auth[20], addit[20]; //the replies from the DNS server
// 	struct sockaddr_in dest;

// 	struct DNS_HEADER *dns = NULL;
// 	struct QUESTION *qinfo = NULL;

// 	printf("Resolving %s", host);

// 	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP packet for DNS queries

// 	dest.sin_family = AF_INET;
// 	dest.sin_port = htons(53);
// 	dest.sin_addr.s_addr = inet_addr(dns_servers[0]); //dns servers

// 	//Set the DNS structure to standard queries
// 	dns = (struct DNS_HEADER *) &buf;

// 	dns->id = (unsigned short) htons(getpid());
// 	dns->qr = 0; //This is a query
// 	dns->opcode = 0; //This is a standard query
// 	dns->aa = 0; //Not Authoritative
// 	dns->tc = 0; //This message is not truncated
// 	dns->rd = 1; //Recursion Desired
// 	dns->ra = 0; //Recursion not available! hey we dont have it (lol)
// 	dns->z = 0;
// 	dns->ad = 0;
// 	dns->cd = 0;
// 	dns->rcode = 0;
// 	dns->q_count = htons(1); //we have only 1 question
// 	dns->ans_count = 0;
// 	dns->auth_count = 0;
// 	dns->add_count = 0;

// 	//point to the query portion
// 	qname = (unsigned char*) &buf[sizeof(struct DNS_HEADER)];

// 	ChangetoDnsNameFormat(qname, host);
// 	qinfo = (struct QUESTION*) &buf[sizeof(struct DNS_HEADER)
// 			+ (strlen((const char*) qname) + 1)]; //fill it

// 	qinfo->qtype = htons(query_type); //type of the query , A , MX , CNAME , NS etc
// 	qinfo->qclass = htons(1); //its internet (lol)

// 	printf("\nSending Packet...");
// 	if (sendto(s, (char*) buf,
// 			sizeof(struct DNS_HEADER) + (strlen((const char*) qname) + 1)
// 					+ sizeof(struct QUESTION), 0, (struct sockaddr*) &dest,
// 			sizeof(dest)) < 0) {
// 		perror("sendto failed");
// 	}
// 	printf("Done");

// 	//Receive the answer
// 	i = sizeof(dest);
// 	printf("\nReceiving answer...");
// 	if (recvfrom(s, (char*) buf, 65536, 0, (struct sockaddr*) &dest,
// 			(socklen_t*) &i) < 0) {
// 		perror("recvfrom failed");
// 	}
// 	printf("Done");

// 	dns = (struct DNS_HEADER*) buf;

// 	//move ahead of the dns header and the query field
// 	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*) qname) + 1)
// 			+ sizeof(struct QUESTION)];

// 	// printf("\nThe response contains : ");
// 	// printf("\n %d Questions.", ntohs(dns->q_count));
// 	// printf("\n %d Answers.", ntohs(dns->ans_count));
// 	// printf("\n %d Authoritative Servers.", ntohs(dns->auth_count));
// 	// printf("\n %d Additional records.\n\n", ntohs(dns->add_count));

// 	//Start reading answers
// 	stop = 0;

// 	for (i = 0; i < ntohs(dns->ans_count); i++) {
// 		answers[i].name = ReadName(reader, buf, &stop);
// 		reader = reader + stop;

// 		answers[i].resource = (struct R_DATA*) (reader);
// 		reader = reader + sizeof(struct R_DATA);

// 		if (ntohs(answers[i].resource->type) == 1) //if its an ipv4 address
// 				{
// 			answers[i].rdata = (unsigned char*) malloc(
// 					ntohs(answers[i].resource->data_len));

// 			for (j = 0; j < ntohs(answers[i].resource->data_len); j++) {
// 				answers[i].rdata[j] = reader[j];
// 			}

// 			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

// 			reader = reader + ntohs(answers[i].resource->data_len);
// 		} else {
// 			answers[i].rdata = ReadName(reader, buf, &stop);
// 			reader = reader + stop;
// 		}
// 	}

// 	//read authorities
// 	for (i = 0; i < ntohs(dns->auth_count); i++) {
// 		auth[i].name = ReadName(reader, buf, &stop);
// 		reader += stop;

// 		auth[i].resource = (struct R_DATA*) (reader);
// 		reader += sizeof(struct R_DATA);

// 		auth[i].rdata = ReadName(reader, buf, &stop);
// 		reader += stop;
// 	}

// 	//read additional
// 	for (i = 0; i < ntohs(dns->add_count); i++) {
// 		addit[i].name = ReadName(reader, buf, &stop);
// 		reader += stop;

// 		addit[i].resource = (struct R_DATA*) (reader);
// 		reader += sizeof(struct R_DATA);

// 		if (ntohs(addit[i].resource->type) == 1) {
// 			addit[i].rdata = (unsigned char*) malloc(
// 					ntohs(addit[i].resource->data_len));
// 			for (j = 0; j < ntohs(addit[i].resource->data_len); j++)
// 				addit[i].rdata[j] = reader[j];

// 			addit[i].rdata[ntohs(addit[i].resource->data_len)] = '\0';
// 			reader += ntohs(addit[i].resource->data_len);
// 		} else {
// 			addit[i].rdata = ReadName(reader, buf, &stop);
// 			reader += stop;
// 		}
// 	}

// 	return;
// }

/*
 *
 * */
u_char* ReadName(unsigned char* reader, unsigned char* buffer, int* count) {
	unsigned char *name;
	unsigned int p = 0, jumped = 0, offset;
	int i, j;

	*count = 1;
	name = (unsigned char*) malloc(256);

	name[0] = '\0';

	//read the names in 3www6google3com format
	while (*reader != 0) {
		if (*reader >= 192) {
			offset = (*reader) * 256 + *(reader + 1) - 49152; //49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; //we have jumped to another location so counting wont go up!
		} else {
			name[p++] = *reader;
		}

		reader = reader + 1;

		if (jumped == 0) {
			*count = *count + 1; //if we havent jumped to another location then we can count up
		}
	}

	name[p] = '\0'; //string complete
	if (jumped == 1) {
		*count = *count + 1; //number of steps we actually moved forward in the packet
	}

	//now convert 3www6google3com0 to www.google.com
	for (i = 0; i < (int) strlen((const char*) name); i++) {
		p = name[i];
		for (j = 0; j < (int) p; j++) {
			name[i] = name[i + 1];
			i = i + 1;
		}
		name[i] = '.';
	}
	name[i - 1] = '\0'; //remove the last dot
	return name;
}


/*
 * This will convert www.google.com to 3www6google3com
 * got it :)
 * */
void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) {
	int lock = 0, i;
	strcat((char*) host, ".");

	for (i = 0; i < strlen((char*) host); i++) {
		if (host[i] == '.') {
			*dns++ = i - lock;
			for (; lock < i; lock++) {
				*dns++ = host[lock];
			}
			lock++; //or lock=i+1;
		}
	}
	*dns++ = '\0';
}

void set_payload(){
	// 00 00 00 00 29 10 00 00 00 00 00 00 00
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(41);
	payload.push_back(16);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	payload.push_back(0);
	return ;
}

void set_dns_header(){
	reply_header.id = 0;
	reply_header.qr = 1; //This is a query
	reply_header.opcode = 0; //This is a standard query
	reply_header.aa = 0; //Not Authoritative
	reply_header.tc = 0; //This message is not truncated
	reply_header.rd = 1; //Recursion Desired
	reply_header.ra = 1; //Recursion available! hey we dont have it (lol)
	reply_header.z = 0;
	reply_header.ad = 0;
	reply_header.cd = 0;
	reply_header.rcode = 0;
	reply_header.q_count = htons(1); //we have only 1 question
	reply_header.ans_count = 0;
	reply_header.auth_count = 0;
	reply_header.add_count = 0;
}

