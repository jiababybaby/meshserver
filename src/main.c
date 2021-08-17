#include "stdio.h"
#include "stdlib.h"
#include "getopt.h"
#include "../include/mesh_tcp.h"
#include "../include/mesh_ws.h"
#include "../include/mesh_sql.h"
#include <getopt.h>
#include <string.h>
#include <sys/prctl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
struct globalArgs_t {
    int tcp_port;                /* -c option */
    int ws_port;             /* -w option */
} globalArgs = {
    .tcp_port = 10086,
    .ws_port = 10087
};

static const char *optString = "t:w:h";
 
static const struct option longOpts[] = {
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};
int get_opt(int argc, char *argv[]){
    int opt=0;
    int option_index = 0;
    opt = getopt_long(argc, argv, optString, longOpts, &option_index);
    while( opt != -1 ) {
        switch( opt ) {
            case 't':
                globalArgs.tcp_port=atoi(optarg);
                break;

            case 'w':
                globalArgs.ws_port=atoi(optarg);
                break;
            case 0:     /* long option without a short arg */
                break;
            case 'h':
                printf("基于物联网多协议智能家居管控系统服务器程序设计\r\n");
                printf("输入命令指定网络端口：\r\n");
                printf("./meshserver.out -t [tcp_port] -w [ws_port]\r\n");

                break;
            default:
                /* You won't actually get here. */
                break;
        }
         
        opt = getopt_long( argc, argv, optString, longOpts, &option_index );
    }
}
int main(int argc,char *argv[]){
    get_opt(argc,argv);
    printf("**************************\r\n");
    printf("**************************\r\n");
    printf("**************************\r\n");
    printf("****mesh_server run...****\r\n");
    printf("**************************\r\n");   
    printf("**************************\r\n");
    printf("**************************\r\n"); 
    printf("tcp_port: %d\r\n ws_port: %d\r\n",globalArgs.tcp_port,globalArgs.ws_port);
    mesh_sql_v.init();

    int pid_ws = fork();
    if(pid_ws==0){
        printf("ws process create success.\r\n");
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        mesh_ws_init(globalArgs.ws_port);
        exit(0);
    }
    else if (pid_ws > 0)
    {
        int pid_tcp = fork();
        if(pid_tcp==0){
            printf("tcp process create success.\r\n");
            prctl(PR_SET_PDEATHSIG, SIGHUP);
            mesh_tcp_init(globalArgs.tcp_port);
            exit(0);
        }
        else if (pid_ws > 0)
        {
            printf("main process.\r\n");
        }
        else
        {
            printf("fork error.\r\n");
        }
    }
    else
    {
        printf("fork error.\r\n");
    }
    waitpid(-1, NULL, 0);
    printf("program %d end.\r\n", getpid());
}