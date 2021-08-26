#ifndef _DATA_SOLVE_H
#define _DATA_SOLVE_H
#include <semaphore.h>
typedef struct sn2fd{
    char sn[30];
    int fd;
} sn2fd;
typedef struct mymesg{
	long int mtype;
	char mtext[1024];
}mymesg;
extern const char *shm2_name;
extern const int shm2_size;
int Data_TCP_Data_Parse_Json(char *tcp_data, sn2fd *shm2_addr, int *num, int fd);
int ws_data_Parse_Json(char *ws_data, char *reply_data);
#endif