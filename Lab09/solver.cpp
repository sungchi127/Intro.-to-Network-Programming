#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include<bits/stdc++.h>
using namespace std;
static const char* socket_path = "/sudoku.sock";
static const unsigned int s_recv_len = 2000;
static const unsigned int s_send_len = 100;
int table[9][9];
const int N = 9;
bool isSafe(int grid[N][N], int row,int col, int num)
{

    for (int x = 0; x <= 8; x++)
        if (grid[row][x] == num)
            return false;
 

    for (int x = 0; x <= 8; x++)
        if (grid[x][col] == num)
            return false;

    int startRow = row - row % 3,
            startCol = col - col % 3;
   
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j +
                            startCol] == num)
                return false;
 
    return true;
}

bool solveSudoku(int grid[N][N], int row, int col)
{
    if (row == N - 1 && col == N)
        return true;

    if (col == N) {
        row++;
        col = 0;
    }
   
    if (grid[row][col] > 0)
        return solveSudoku(grid, row, col + 1);
 
    for (int num = 1; num <= N; num++)
    {
        if (isSafe(grid, row, col, num))
        {
            grid[row][col] = num;

            if (solveSudoku(grid, row, col + 1))
                return true;
        }
    
        grid[row][col] = 0;
    }
    return false;
}

int main()
{
	int sock = 0;
	int data_len = 0;
	struct sockaddr_un remote;
	char recv_msg[s_recv_len];
	char send_msg[s_send_len];

	memset(recv_msg, 0, s_recv_len*sizeof(char));
	memset(send_msg, 0, s_send_len*sizeof(char));

	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	remote.sun_family = AF_UNIX;
	strcpy( remote.sun_path, socket_path );
	data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	connect(sock, (struct sockaddr*)&remote, data_len);


	printf("Client: Connected \n");
    // recv(sock, recv_msg, 1000,0);
    string ss="S";
    char* cc=(char*)ss.c_str();
    send(sock, cc, ss.length()*sizeof(char), 0 );
    recv(sock, recv_msg, 200,0);

    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            // printf("ans: %c\n", recv_msg[9*i+j+4]);
            if(recv_msg[9*i+j+4]=='.'){
                table[i][j]=0;
            }
            else{
                table[i][j]=recv_msg[9*i+j+4]-'0';
            }
        }
    }
    
    solveSudoku(table,0,0);
    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            printf("%d", table[i][j]);
        }
        printf("\n");
    }
    // printf("serial: %s\n", serial);

    memset(send_msg, 0, s_send_len*sizeof(char));
    // memset(recv_msg, 0, s_recv_len*sizeof(char));
    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            // printf("ans: %c\n", serial[cnt]);
            if(recv_msg[9*i+j+4]=='.'){
                string s="V "+to_string(i)+" "+to_string(j)+" "+to_string(table[i][j]);
                char* c=(char*)s.c_str();
                send(sock, c, s.length()*sizeof(char), 0 );
                // data_len = recv(sock, recv_msg, s_recv_len, 0);
                // printf("Client: Data received: %s \n", recv_msg);0
                usleep(200*1000);
            }
        }

	}
    // string s="P";
    // char* c=(char*)s.c_str();
    // send(sock, c, s.length()*sizeof(char), 0 );
    // memset(recv_msg, 0, s_recv_len*sizeof(char));
    // recv(sock, recv_msg, 2000, 0);
    // printf("Client: Data received: %s \n", recv_msg);

    string sss="C";
    char* ccc=(char*)sss.c_str();
    send(sock, ccc, sss.length()*sizeof(char), 0 );

    printf("Client: bye! \n");
	return 0;
}


 