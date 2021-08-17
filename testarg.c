#include "stdarg.h"
#include "stdio.h"
#include "string.h"
int add_quo(char *str,...){
    va_list args;
    char *args1;
    va_start(args,str);
    args1 = va_arg(args,char *);
    while(args1 != NULL){
        strcat(str, args1);
        args1 = va_arg(args,char *);
    }
    va_end(args);
    return 0;
}

int main(){
    char command[20] = {0};
    add_quo(command, "132", "980", "756");
    printf("%s\r\n",command);
}