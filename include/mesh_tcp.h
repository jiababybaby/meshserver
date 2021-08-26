#ifndef _MESH_TCP_H
#define _MESH_TCP_H
typedef struct tcp_fd{
    char sn[100];
    sem_t *sem_write;
} tcp_fd;
int mesh_tcp_init(int tcp_port);

#endif
