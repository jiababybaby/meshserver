#include <strings.h>
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include "../include/data_solve.h"
#include "../include/hash.h"
sem_t *sem_tcp_recv, *sem_shm_tcp;
const char *shm_name = "sh_mem1";
const int shm_size = 1024;
const char *shm2_name = "sh_mem2";
const int shm2_size = 10240;
int msg_id = -1;
#define OPEN_MAX 100
void Stop(int signo) {
    printf("oops! stop!!!\n");
    sem_close(sem_tcp_recv);
    sem_close(sem_shm_tcp);
    sem_unlink("sem_tcp_recv");
    sem_unlink("sem_shm_tcp");
    shm_unlink(shm_name);
    shm_unlink(shm2_name);
    if(msgctl(msg_id,IPC_RMID,NULL) < 0)
	{
		printf("del msg error \n");
	}
    _exit(0);

}
int tcp_fd = -1;
int tcp_epoll_fd = -1;
void TCP_handler(int signum)
{
    //信号的响应操作  
    printf("recive signal %d\n", signum);  
    //打印sigval的参数
    if(tcp_fd > 0)
    {
        close(tcp_fd);
    }
    if(tcp_epoll_fd > 0)
    {
        close(tcp_epoll_fd);
    }
    exit(0);
}
int mesh_tcp_init(int tcp_port)
{
    signal(SIGINT, Stop); 
	mymesg ckxmsg;
	msg_id = msgget(0x01,IPC_CREAT | 0666);
	if(msg_id == -1)
	{
		printf("create msg error \n");
		return 0;
	}
    sem_tcp_recv = sem_open("sem_tcp_recv", O_CREAT, 0666, 0);
    sem_shm_tcp = sem_open("sem_shm_tcp", O_CREAT, 0666, 1);
    int shmfd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if(shmfd == -1){
        printf("shm_open error(%d)\n",errno);
        exit(1);
    }
    if (ftruncate(shmfd, shm_size) == -1){
        printf("ftruncate error(%d)\n",errno);
        exit(1);
    }
    struct stat buf;
    if(fstat(shmfd,&buf) == -1){
        printf("fstat error(%d)\n",errno);
        exit(1);
    }
    printf("shm size:(%ld)\n",buf.st_size);
    int shm2fd = shm_open(shm2_name, O_CREAT | O_RDWR, 0666);
    if(shm2fd == -1){
        printf("shm_open error(%d)\n",errno);
        exit(1);
    }
    if (ftruncate(shm2fd, shm_size) == -1){
        printf("ftruncate error(%d)\n",errno);
        exit(1);
    }
    struct stat buf2;
    if(fstat(shm2fd,&buf2) == -1){
        printf("fstat error(%d)\n",errno);
        exit(1);
    }
    printf("shm2 size:(%ld)\n",buf2.st_size);
    //HashTable* ht = hash_table_new();

    char *shm_addr=NULL;
    shm_addr= mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    sn2fd *shm2_addr=NULL;
    shm2_addr= mmap(NULL, shm2_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm2fd, 0);
    
    struct epoll_event event;   // 告诉内核要监听什么事件  
    struct epoll_event wait_event; //内核监听完的结果
	
	//1.创建tcp监听套接字
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//2.绑定sockfd
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(tcp_port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int socket_ret = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if(socket_ret < 0){
        perror("socket bind error.\n");
    }
    int rc;
    int val=1;
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&val, sizeof(val));

        
	//3.监听listen
	listen(sockfd, 100);
	 
	//4.epoll相应参数准备
	int fd[OPEN_MAX];
	int i = 0, maxi = 0;
	memset(fd,-1, sizeof(fd));
	fd[0] = sockfd;
	
	int epfd = epoll_create(10); // 创建一个 epoll 的句柄，参数要大于 0， 没有太大意义  
    if( -1 == epfd ){  
        perror ("epoll_create");  
        return -1;  
    }  
      
    event.data.fd = sockfd;     //监听套接字  
    event.events = EPOLLIN; // 表示对应的文件描述符可以读
	
	//5.事件注册函数，将监听套接字描述符 sockfd 加入监听事件  
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);  
    if(-1 == ret){  
        perror("epoll_ctl");  
        return -1;  
    }
    //todo read
    while(1)
    {
        printf("waiting ....\n");
        // 监视并等待多个文件（标准输入，udp套接字）描述符的属性变化（是否可读）
        // 没有属性变化，这个函数会阻塞，直到有变化才往下执行，这里没有设置超时
        ret = epoll_wait(epfd, &wait_event, maxi+1, -1); 
        
        //6.1监测sockfd(监听套接字)是否存在连接
        if(( sockfd == wait_event.data.fd )   
            && ( EPOLLIN == wait_event.events & EPOLLIN ) )
        {
            struct sockaddr_in cli_addr;
            int clilen = sizeof(cli_addr);
            
            //6.1.1 从tcp完成连接中提取客户端
            int connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            rc = setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&val, sizeof(val));
            printf("accept %d\n", connfd);
            int pid_r = fork();
            if (pid_r == 0)
            {
                close(sockfd);
                close(epfd);
                int epfd_t = epoll_create(10); // 创建一个 epoll 的句柄，参数要大于 0， 没有太大意义  
                if( -1 == epfd_t ){  
                    perror ("epoll_create");  
                    return -1;  
                }  
                event.data.fd = connfd; //监听套接字
                event.events = EPOLLIN|EPOLLRDHUP; // 表示对应的文件描述符可以读
                //6.1.3.事件注册函数，将监听套接字描述符 connfd 加入监听事件  
                ret = epoll_ctl(epfd_t, EPOLL_CTL_ADD, connfd, &event);
                tcp_epoll_fd = epfd_t;
                tcp_fd = connfd;
                int fd[2];
                pipe(fd);//fd[0]是读，fd[1]是写
                int write_pid = fork();
                if(write_pid==0){
                    signal(SIGINT, TCP_handler);
                    shm_addr= mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
                    char tcp_deal[1024] = {0};
                    int fd_i = -1;
                    //todo write
                    close(fd[1]);
                    read(fd[0],&fd_i,sizeof(int));
                    printf("receive fd_i %d\n", fd_i);
                    char buf[1024]={0};
                    while (1)
                    {
                        memset(buf, 0, sizeof(buf));
                        int ret = msgrcv(msg_id, buf, sizeof(buf), fd_i+1, 0);
                        if(ret == -1){
                            printf("msgrcv error.\n");
                            exit(0);
                        }
                        printf("msgrcv %d ret: %s\n",ret,buf+8);
                        ret = write(connfd, buf+sizeof(long int), strlen(buf+sizeof(long int)));
                        printf("send: %d\n",ret);
                    }
                }else if(write_pid>0){
                    int fd_i = -1;
                    close(fd[0]);
                    while (1)
                    {
                        printf("watting at %d\n", epfd_t);
                        ret = epoll_wait(epfd_t, &wait_event, maxi+1, -1); 
                        printf("watting at %d\n", write_pid);
                        printf("epoll: %x\n",wait_event.events);
                        if(( connfd == wait_event.data.fd )   
                        && ( EPOLLIN == wait_event.events & (EPOLLIN|EPOLLERR) ))
                        {
                            int len = 0;
                            char buf[1024] = {0};
                            
                            //6.2.1接受客户端数据
                            if((len = recv(connfd, buf, sizeof(buf), 0)) < 0)
                            {
                                printf("超时。\n");
                                close(connfd);
                                close(epfd_t);
                                connfd = -1;
                                kill(write_pid, SIGKILL);
                                waitpid(write_pid,NULL,-1);
                                exit(0);
                            }
                            else if(len == 0)//客户端关闭连接
                            {
                                close(connfd);
                                close(epfd_t);
                                connfd = -1;
                                printf("断开连接1\n");
                                kill(write_pid, SIGKILL);
                                waitpid(write_pid,NULL,-1);
                                exit(0);
                            }
                            else//正常接收到服务器的数据
                            {
                                printf("recv %s\n", buf);
                                // sem_wait(sem_shm_tcp);
                                printf("tcp deal process\n");
                                Data_TCP_Data_Parse_Json(buf, shm2_addr, &fd_i, connfd);
                                write(fd[1], &fd_i, sizeof(int));
                                printf("i: %d\n", fd_i);
                                //send(connfd, buf, len, 0);
                                printf("tcp read process.\r\n");
                            }
                        }else if(( connfd == wait_event.data.fd )   
                        && ( (EPOLLRDHUP) == (wait_event.events & EPOLLRDHUP) ))
                        {
                            printf("%x\n",wait_event.events);
                            int i = 0;
                            if(fd_i >= 0)
                            {
                                memset(&shm2_addr[fd_i], 0, strlen((shm2_addr[fd_i].sn)));
                                for (i = 0; i < 10; i++)
                                {
                                    printf("i: %d, sn: %s\n", i, &shm2_addr[i]);
                                }
                            }
                            close(connfd);
                            close(epfd_t);
                            connfd = -1;
                            printf("断开连接...\n");
                            kill(write_pid, SIGINT);
                            waitpid(write_pid,NULL,-1);
                            exit(0);
                        }else
                        {
                            printf("wfd: %d,event: %x, fd: %d\n",wait_event.data.fd, wait_event.events, connfd);
                        }
                    }
                }else
                {
                    printf("fork error\n");
                }
            }
            else if (pid_r > 0)
            {
                close(connfd);
            }
            else
            {
                printf("fork error\n");
            }
            // //6.1.2 将提取到的connfd放入fd数组中，以便下面轮询客户端套接字
            // for(i=1; i<OPEN_MAX; i++)
            // {
            //     if(fd[i] < 0)
            //     {
            //         fd[i] = connfd;
            //         event.data.fd = connfd; //监听套接字  
            //         event.events = EPOLLIN; // 表示对应的文件描述符可以读
                    
            //         //6.1.3.事件注册函数，将监听套接字描述符 connfd 加入监听事件  
            //         ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);  
            //         if(-1 == ret){  
            //             perror("epoll_ctl");  
            //             return -1;  
            //         } 
                    
            //         break;
            //     }
            // }
            
            // //6.1.4 maxi更新
            // if(i > maxi)
            //     maxi = i;
                
            //6.1.5 如果没有就绪的描述符，就继续epoll监测，否则继续向下看
            // if(--ret <= 0)
            //     continue;
        }
        
        // //6.2继续响应就绪的描述符
        // for(i=1; i<=maxi; i++)
        // {
        //     if(fd[i] < 0)
        //         continue;
            
        //     if(( fd[i] == wait_event.data.fd )   
        //     && ( EPOLLIN == wait_event.events & (EPOLLIN|EPOLLERR) ))
        //     {
        //         int len = 0;
        //         char buf[128] = "";
                
        //         //6.2.1接受客户端数据
        //         if((len = recv(fd[i], buf, sizeof(buf), 0)) < 0)
        //         {
        //             if(errno == ECONNRESET)//tcp连接超时、RST
        //             {
        //                 close(fd[i]);
        //                 fd[i] = -1;
        //             }
        //             else
        //                 perror("read error:");
        //         }
        //         else if(len == 0)//客户端关闭连接
        //         {
        //             close(fd[i]);
        //             fd[i] = -1;
        //         }
        //         else//正常接收到服务器的数据
        //         {
        //             sem_wait(sem_shm_tcp); 
        //             memcpy(shm_addr,buf,len);
        //             sem_post(sem_shm_tcp);
        //             sem_post(sem_tcp_recv);
        //             send(fd[i], buf, len, 0);
        //             printf("tcp read process.\r\n");
        //         }
        //         //6.2.2所有的就绪描述符处理完了，就退出当前的for循环，继续poll监测
        //         if(--ret <= 0)
        //             break;
        //     }
        // }
    }
    return 0;
}
