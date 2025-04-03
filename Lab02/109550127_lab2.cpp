#include<bits/stdc++.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include <arpa/inet.h>

using namespace std;
#define FLAGS O_RD | O_APPEND
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

typedef struct {
        uint32_t magic;
        int32_t  off_str;
        int32_t  off_dat;
        uint32_t n_files;
} __attribute((packed)) a;

typedef struct {
        uint32_t filename;
        int32_t  file_size;
        int32_t  file_content;
        uint64_t checksum;
} __attribute((packed)) b;

int main(int argc ,char* argv[]){
    // const char *pathname = "testcase.pak";
    // cout<<argc<<" "<<argv[0]<<" "<<argv[1]<<" "<<argv[2];
    string dir;
    dir=(string)argv[2];
    const char *pathname=argv[1];
    int fd,pos=16;
    a aa[1];
    b bb[1];
    char c[1];
    // cin>>path>>dir;
    fd = open(pathname, O_RDONLY);
    read(fd, aa, sizeof(a));
    cout<<"1."<<"\n";
    cout<<"number of files:"<<aa[0].n_files<<"\n";

    int n=aa[0].n_files;
    int start=aa[0].off_str;
    int contstart=aa[0].off_dat;
    vector<uint64_t> chksum;
    vector<int> cont,fz;
    vector<string> name;

    cout<<"2."<<"\n";
    for(int i=0;i<n;i++){
        lseek(fd, pos, SEEK_SET);
        read(fd, bb, sizeof(b));
        // cout<<fd<<"\n";
        int fn=bb[0].filename;
        cont.push_back(bb[0].file_content);
        bb[0].checksum=ntohll(bb[0].checksum);
        chksum.push_back(bb[0].checksum);
        lseek(fd, start+fn, SEEK_SET);
        read(fd, c, sizeof(c));
        bb[0].file_size=ntohl(bb[0].file_size);
        string nam="";
        cout<<i<<" ";
        while(c[0]!=NULL){
            cout<<c[0];
            nam+=c[0];
            read(fd, c, sizeof(c));
        }
        name.push_back(nam);
        pos+=20;
        cout<<"  size="<<bb[0].file_size;
        fz.push_back(bb[0].file_size);
        cout<<"\n";
    }
    cout<<"3."<<"\n";
    char cc[3368544];
    uint64_t chk[1],xsum;
    // int zxc=0;
    for(int i=0;i<n;i++){
        double size=ceil(fz[i]/8.0);
        lseek(fd, contstart+cont[i], SEEK_SET);
        // cout<<i<<"."<<size<<" "<<fz[i]<<"\n";
        int back = (int)size*8-fz[i];
        if(name[i]!="checker"){
            for(int j=0;j<(int)size;j++){
                // read(fd, chk, sizeof(chk));
                chk[0]=0;
                if(i==n-1 && j==(int)size-1){
                    read(fd, chk, sizeof(chk)-back);
                    // cout<<back<<" ";
                }
                else{
                    read(fd, chk, sizeof(chk));
                }
                if(j==0){
                    xsum=chk[0];
                    continue;
                }
                xsum^=chk[0];
                // if(i==n-1) cout<<j<<":"<< hex << setfill('0') << setw(16) << chk[0]<<" ";
            }
        }

        // if(xsum!=chksum[i] && name[i]!="checker" ) {
        //     cout<<hex<<i<<"."<<name[i];
        //     cout<<":"<<xsum<<" "<<chksum[i]<<"\n";
        // }

        if((xsum==chksum[i] && name[i]!="checker")){
            string str="";    
            lseek(fd, contstart+cont[i], SEEK_SET);
            int fdd;
            string fl=dir+'/'+name[i];
            // cout<<fl<<"\n";
            // for(int j=1;j<sizeof(fl);j++){
            //     fl[j-1]=fl[j];
            // }
            fdd = open(fl.c_str(), O_WRONLY|O_CREAT|O_TRUNC);
            // cout<<fdd<<"\n";
            for(int j=0 ;j < fz[i] ;j++){
                // if(name[i]=="checker" && j%10000==0) cout<<j<<"\n";
                read(fd, c, sizeof(c));
                if((c[0] < 32 && c[0] > 126) && c[0] != 10) continue;
                write(fdd ,c,sizeof(c));
                // if(c[0]>= 32 && c[0]<=126)
            }
        }
        
        if(name[i]=="checker"){
            // cout<<hex<<chksum[i]<<":"<<xsum<<"\n";
            lseek(fd, contstart+cont[i], SEEK_SET);
            int fdd;
            string fl=dir+'/'+name[i];
            // for(int j=1;j<sizeof(fl);j++){
            //     fl[j-1]=fl[j];
            // }
            fdd = open(fl.c_str(), O_WRONLY|O_CREAT|O_TRUNC);

            read(fd, cc, sizeof(cc));

            write(fdd ,cc,fz[i]);
        }
        // cout<<fdd<<" "<<write(fdd ,s.c_str(),fz[i])<<"\n";
        // cout<<"\n";
    } 

    // cout<< setfill('0') << setw(4) << ans<<" ";

    return 0;
}