#include "cjson/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/json_check.h"
#include "../include/mesh_sql.h"
#include "../include/data_solve.h"
/* Used by some code below as an example datatype. */
/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = cJSON_Print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("cJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
            printf("cJSON_Print result:\n%s\n", out);
            printf("cJSON_PrintPreallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
        printf("cJSON_Print result:\n%s\n", out);
        printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    free(out);
    free(buf_fail);
    free(buf);
    return 0;
}
int json_checker(const char *json_str) 
{
    printf("ready to check %s\n", json_str);
    JSON_checker jc = new_JSON_checker(20);

    int len = strlen(json_str);
    int tmp_i = 0;
 
    for (tmp_i = 0; tmp_i < len; tmp_i++) 
    {
        int next_char = json_str[tmp_i];
        if (next_char <= 0) 
        {
            break;
        }
        if (0 == JSON_checker_char(jc, next_char)) 
        {
            fprintf(stderr, "JSON_checker_char: syntax error\n");
            return -1;
        }
    }
    if (0 == JSON_checker_done(jc)) 
    {
        fprintf(stderr, "JSON_checker_end: syntax error\n");
        return -1;
    }
    return 0;
}
/*
将中文字符替换为'*' 用于json字符串合法性检查
instr:	原字符串(用来解析)
outstr:	替换后字符串(用来检查)
 */
int replace_character(char *instr, char* outstr)
{
    if (instr == NULL)
    {
        printf("No string buf...\n");
        return -1;
    }
 
    while(*instr != '\0')
    {
		//acsll范围00-7F
        if (((*instr) < 0x00) || ((*instr) > 0x7F))
        {
            *outstr++ = '*';
			instr++;
        }
		else
		{
			*outstr++ = *instr++;
		}
    }
 
    return 0;
}

int Data_TCP_Data_Parse_Json(char *tcp_data, sn2fd *shm2_addr, int *num, int fd)
{
    if(tcp_data == NULL || shm2_addr == NULL)
    {
        return -1;
    }
    char *json_src_string = NULL;
    char *json_chk_string = NULL;

    json_src_string = (char *) malloc(strlen(tcp_data)+1);
    memset(json_src_string, 0, strlen(tcp_data)+1);
    json_chk_string = (char *) malloc(strlen(tcp_data)+1);
    memset(json_chk_string, 0, strlen(tcp_data)+1);
    memcpy(json_src_string, tcp_data, strlen(tcp_data)+1);
    //替换中文字符为“*”
    replace_character(json_src_string, json_chk_string);
    int nRtn = json_checker(json_chk_string);
    if (0 != nRtn)
    {
        free(json_src_string);
        json_src_string = NULL;
        free(json_chk_string);
        json_chk_string = NULL;
        printf("tcp check error. %s\n", tcp_data);
        return -1;
        //解析json_src_string
        //to do
    }
    free(json_src_string);
    json_src_string = NULL;
    free(json_chk_string);
    json_chk_string = NULL;

    cJSON *root = NULL;
    root = cJSON_Parse(tcp_data);
    if(cJSON_IsInvalid(root) == cJSON_True){
        printf("tcp parse error. %s\n", tcp_data);
        return -1;
    }
    char* sn = cJSON_GetStringValue(cJSON_GetObjectItem(root,"sn"));
    char* mac = cJSON_GetStringValue(cJSON_GetObjectItem(root,"device_mac"));
    char* kind = cJSON_GetStringValue(cJSON_GetObjectItem(root,"device_kind"));
    char* id = cJSON_GetStringValue(cJSON_GetObjectItem(root,"device_id"));
    char* data = cJSON_GetStringValue(cJSON_GetObjectItem(root,"device_data"));
    device device_info;
    device_info.sn=sn;
    device_info.device_mac=mac;
    device_info.device_kind=kind;
    device_info.device_id=id;
    device_info.device_data=data;
    char *status = "1";
    device_info.device_status = status;
    printf("sn: %s, mac :%s, kind: %s, id: %s, data: %s, status: %s\r\n",device_info.sn,device_info.device_mac,
    device_info.device_kind,device_info.device_id,device_info.device_data,device_info.device_status);
    mesh_sql_v.add_device_data(device_info);
    if(*num == -1){
        int i = 0;
        while (strlen(shm2_addr[i].sn) != 0)
        {
            if(strcmp(shm2_addr[i].sn, sn) == 0)
            {
                break;
            }
            i++;
        }
        *num = i;
        strcpy(shm2_addr[i].sn, sn);
        shm2_addr->fd = fd;
        cJSON_Delete(root);
        for (i = 0; i < 10;i++)
        {
            printf("i: %d, sn: %s\n", i, &shm2_addr[i]);
        }
    }
    return 0;
}
int open_shm2(sn2fd **shm2_addr)
{
    int shm2fd = shm_open(shm2_name, O_CREAT | O_RDWR, 0666);
    if(shm2fd == -1){
        printf("shm_open error(%d)\n",errno);
        exit(1);
    }
    if (ftruncate(shm2fd, shm2_size) == -1){
        printf("ftruncate error(%d)\n",errno);
        exit(1);
    }
    struct stat buf2;
    if(fstat(shm2fd,&buf2) == -1){
        printf("fstat error(%d)\n",errno);
        exit(1);
    }
    printf("shm2 size:(%ld)\n", buf2.st_size);
    *shm2_addr= mmap(NULL, shm2_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm2fd, 0);
}
int send_to_device(char *sn, char *reply)
{
    if(sn == NULL || reply == NULL)
    {
        return -1;
    }
    sn2fd *shm2_addr = NULL;
    open_shm2(&shm2_addr);
    printf("shm2_addr%x\n", shm2_addr);
    printf("sn: %s\n", sn);
    for (int i = 0; i < 10; i++)
    {
        if(strcmp(sn, shm2_addr[i].sn) == 0){
            printf("i: %d, sn: %s\n",i, shm2_addr[i].sn);
            mymesg ckxmsg;
            memset(&ckxmsg, 0, sizeof(mymesg));
            ckxmsg.mtype = i+1;
            strcpy(ckxmsg.mtext, reply);
            int msg_id = msgget(0x01, IPC_CREAT | 0666);
            if(msg_id == -1)
            {
                printf("create msg error \n");
                return 0;
            }
            msgsnd(msg_id, &ckxmsg, sizeof(long)+strlen(ckxmsg.mtext), 0);
            //send(shm2_addr[i].fd, reply, strlen(reply), 0);
            break;
        }
    }
}
int ws_data_Parse_Json(char *ws_data, char *reply_data)
{
    char *json_src_string = NULL;
    char *json_chk_string = NULL;
    json_src_string = (char *) malloc(strlen(ws_data)+1);
    memset(json_src_string, 0, strlen(ws_data)+1);
    json_chk_string = (char *) malloc(strlen(ws_data)+1);
    memset(json_chk_string, 0, strlen(ws_data)+1);
    memcpy(json_src_string, ws_data, strlen(ws_data)+1);
    //替换中文字符为“*”
    replace_character(json_src_string, json_chk_string);
    int nRtn = json_checker(json_chk_string);
    if (0 != nRtn)
    {
        free(json_src_string);
        json_src_string = NULL;
        free(json_chk_string);
        json_chk_string = NULL;
        printf("ws check error. %s\n", ws_data);
        return -1;
        //解析json_src_string
        //to do
    }
    free(json_src_string);
    json_src_string = NULL;
    free(json_chk_string);
    json_chk_string = NULL;

    cJSON *root = NULL;
    root = cJSON_Parse(ws_data);
    if(cJSON_IsInvalid(root)==cJSON_True){
        printf("ws cjson error. ws: %s\n", ws_data);
        return -1;
    }
    char* method = cJSON_GetStringValue(cJSON_GetObjectItem(root,"method"));
    if(!strcmp(method, "add_sn")){
        char* opid= cJSON_GetStringValue(cJSON_GetObjectItem(root,"opid"));
        cJSON* SN = cJSON_GetObjectItem(root,"sn");   //gataway
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));
        user_gw gw_info;
        gw_info.opid=opid;
        gw_info.sn=sn;
        gw_info.sn_name=name;
        gw_info.sn_position=position;
        gw_info.sn_status="false";
        printf("ready add\n");
        mesh_sql_v.mysql_addsn(gw_info);
    }
    else if(!strcmp(method, "del_sn")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));
        cJSON* SN = cJSON_GetObjectItem(root, "sn");   //gataway
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));
        user_gw gw_info;
        gw_info.opid=opid;
        gw_info.sn=sn;
        gw_info.sn_name=name;
        gw_info.sn_position=position;
        gw_info.sn_status="false";
        mesh_sql_v.mysql_delsn(gw_info);
    }
    else if(!strcmp(method, "set_sn")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));

        cJSON* SN = cJSON_GetObjectItem(root, "sn");   //gataway
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));
        user_gw gw_info;
        gw_info.opid=opid;
        gw_info.sn=sn;
        gw_info.sn_name=name;
        gw_info.sn_position=position;
        gw_info.sn_status="false";
        mesh_sql_v.mysql_upsn(gw_info);
    }
    else if(!strcmp(method, "get_sn")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));

        cJSON* SN = cJSON_GetObjectItem(root, "sn");   //gataway
        if(cJSON_IsInvalid(SN)){
            printf("sn error\n");
            cJSON_Delete(root);
            return -1;
        }
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));


        user_gw gw_info;
        gw_info.opid = opid;
        gw_info.sn = sn;
        gw_info.sn_name = name;
        gw_info.sn_position = position;
        gw_info.sn_floor = floor;
        gw_info.sn_status = "false";
        sn_get_from_sql sn_get_from_sql_v;
        memset(&sn_get_from_sql_v, 0, sizeof(sn_get_from_sql));
        printf("sn_get_num: %d\n", sn_get_from_sql_v.num);
        if (mesh_sql_v.mysql_getsn(opid, &sn_get_from_sql_v) == -1)
        {
            printf("get sn failed.\n");
            cJSON_Delete(root);
            return -1;
        }
        
        cJSON* ans = NULL;
        cJSON* dec = NULL;
        ans = cJSON_CreateObject();
        dec = cJSON_CreateArray();
        int i = 0;
        cJSON_AddStringToObject(ans, "method", "get_sn");
        for (i = 0; i < sn_get_from_sql_v.num; i++)
        {
            cJSON* sn_temp;
            printf("%x\n",sn_get_from_sql_v.user_gw_v[i].sn);
            sn_temp = cJSON_CreateObject();
            cJSON_AddStringToObject(sn_temp, "name", sn_get_from_sql_v.user_gw_v[i].sn_name);
            cJSON_AddStringToObject(sn_temp, "position", sn_get_from_sql_v.user_gw_v[i].sn_position);
            cJSON_AddStringToObject(sn_temp, "floor", sn_get_from_sql_v.user_gw_v[i].sn_floor);
            cJSON_AddStringToObject(sn_temp, "status", sn_get_from_sql_v.user_gw_v[i].sn_status);
            cJSON_AddStringToObject(sn_temp, "sn", sn_get_from_sql_v.user_gw_v[i].sn);

            int ret = cJSON_AddItemToArray(dec, sn_temp);
        }
        cJSON_AddItemToObject(ans, "sn", dec);
        char *ans_str = cJSON_Print(ans);
        memset(reply_data, 0, sizeof(reply_data));
        strcpy(reply_data, ans_str);
        if(ans_str != NULL){
            free(ans_str);
        }
        for (int i = 0; i < sn_get_from_sql_v.num; i++)
        {
            sn_get_data_deinit(&sn_get_from_sql_v, i);
        }
        cJSON_Delete(ans);
    }else if(!strcmp(method, "get_device")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));
        char* sn, *position, *name, *floor;
        cJSON *SN = cJSON_GetObjectItem(root, "sn"); //gataway
        if(!cJSON_IsInvalid(SN))
        {
            position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
            sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
            name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
            floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));
        }
        char* device_sn, *device_mac, *device_kind, *device_id, *data_method, *data_num;   
        cJSON *Device = cJSON_GetObjectItem(root, "device"); //device
        if(!cJSON_IsInvalid(Device))
        {
            device_sn=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "sn"));
            device_mac=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_mac"));
            device_kind=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_kind"));
            device_id=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_id"));
            cJSON* device_data=cJSON_GetObjectItem(Device, "device_data");
            data_method=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_method"));
            data_num=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_num"));
        }
        user_gw gw_info;
        gw_info.opid=opid;
        gw_info.sn=sn;
        gw_info.sn_name=name;
        gw_info.sn_position=position;
        gw_info.sn_status="false";
        device_get_from_sql device_get_from_sql_v;
        memset(&device_get_from_sql_v, 0, sizeof(device_get_from_sql));
        printf("ready get device\n");
        if(mesh_sql_v.mysql_getdevice(gw_info, device_mac, &device_get_from_sql_v) == -1){
            printf("get device failed.\n");
            cJSON_Delete(root);
            return -1;
        }
        printf("get device success.\n");
        cJSON *ans;
        cJSON* dec;
        ans = cJSON_CreateObject();
        dec = cJSON_CreateArray();
        int i = 0;
        cJSON_AddStringToObject(ans, "method", "get_device");
        for (i = 0; i < device_get_from_sql_v.num; i++)
        {
            cJSON* device_temp;
            device_temp = cJSON_CreateObject();
            cJSON_AddStringToObject(device_temp, "device_mac", device_get_from_sql_v.device_v[i].device_mac);
            cJSON_AddStringToObject(device_temp, "device_name", device_get_from_sql_v.device_v[i].device_name);
            cJSON_AddStringToObject(device_temp, "device_kind", device_get_from_sql_v.device_v[i].device_kind);
            cJSON_AddStringToObject(device_temp, "device_id", device_get_from_sql_v.device_v[i].device_id);
            cJSON_AddStringToObject(device_temp, "device_data", device_get_from_sql_v.device_v[i].device_data);
            cJSON_AddStringToObject(device_temp, "device_status", device_get_from_sql_v.device_v[i].device_status);
            cJSON_AddStringToObject(device_temp, "sn", device_get_from_sql_v.device_v[i].sn);
            cJSON_AddItemToArray(dec, device_temp);
        }
        cJSON_AddItemToObject(ans, "device", dec);
        char *ans_str = cJSON_Print(ans);
        memset(reply_data, 0, sizeof(reply_data));
        strcpy(reply_data, ans_str);
        if(ans_str != NULL){
            free(ans_str);
        }
        for (i = 0; i < device_get_from_sql_v.num; i++){
            device_get_data_deinit(&device_get_from_sql_v, i);
        }
        // cJSON_Delete(dec);
        cJSON_Delete(ans);       
    }else if(!strcmp(method, "set_device")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));

        cJSON* SN = cJSON_GetObjectItem(root, "sn");   //gataway
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));

        cJSON* Device = cJSON_GetObjectItem(root, "device");   //device
        char* device_sn=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "sn"));
        char* device_mac=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_mac"));
        char* device_kind=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_kind"));
        char* device_id=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_id"));
        cJSON* device_data=cJSON_GetObjectItem(Device, "device_data");
        char* data_method=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_method"));
        char* data_num=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_num"));


        cJSON* ans, *ans_data;
        device device_info;
        device_info.device_mac=device_mac;
        device_info.sn=device_sn;
        device_info.device_id=device_id;
        device_info.device_kind=device_kind;
        ans = cJSON_CreateObject();
        ans_data = cJSON_CreateObject();

        cJSON_AddStringToObject(ans, "device_mac", device_info.device_mac);
        //cJSON_AddStringToObject(ans, "device_kind", device_info.device_kind);
        cJSON_AddStringToObject(ans, "device_id", device_info.device_id);
        cJSON_AddStringToObject(ans, "sn", device_info.sn);
        cJSON_AddStringToObject(ans_data, "data_method", data_method);
        cJSON_AddStringToObject(ans_data, "data_num", data_num);
        cJSON_AddItemToObject(ans, "device_data", ans_data);
        char *ans_str = cJSON_Print(ans);
        memset(reply_data, 0, sizeof(reply_data));
        strcpy(reply_data, ans_str);
        if(ans_str != NULL){
            free(ans_str);
        }
        cJSON_Delete(ans);  
        //TODO: 发送给设备
        printf("send to device %s",reply_data);
        send_to_device(device_sn, reply_data);
        // emit send_to_gw(device_sn,json_str);
        //            device_info.sn_name=name;
        //            device_info.sn_position=position;
        //            device_info.sn_status="false";
        //            mesh_sql_v.mysql_upsn(gw_info);
    }else if(!strcmp(method, "get_device_id")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));

        cJSON* SN = cJSON_GetObjectItem(root, "sn");   //gataway
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));

        cJSON* Device = cJSON_GetObjectItem(root, "device");   //device
        char* device_sn=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "sn"));
        char* device_mac=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_mac"));
        char* device_kind=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_kind"));
        char* device_id=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_id"));
        cJSON* device_data=cJSON_GetObjectItem(Device, "device_data");
        char* data_method=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_method"));
        char* data_num=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_num"));


        device device_info;
        device_info.device_mac=device_mac;
        device_info.sn=device_sn;
        device_info.device_kind=device_kind;        
        mesh_sql_v.mysql_get_device_id(device_info, device_info.device_id);
        cJSON* ans;
        cJSON* Device_ans;
        ans = cJSON_CreateObject();
        Device_ans = cJSON_CreateObject();
        cJSON_AddStringToObject(Device_ans, "device_mac", device_info.device_mac);
        cJSON_AddStringToObject(Device_ans, "device_name", device_info.device_name);
        cJSON_AddStringToObject(Device_ans, "device_kind", device_info.device_kind);
        cJSON_AddStringToObject(Device_ans, "device_id", device_info.device_id);
        cJSON_AddStringToObject(Device_ans, "device_data", device_info.device_data);
        cJSON_AddStringToObject(Device_ans, "device_status", device_info.device_status);
        cJSON_AddStringToObject(Device_ans, "device_status", device_info.device_status);
        cJSON_AddStringToObject(Device_ans, "sn", device_info.sn);
        cJSON_AddStringToObject(ans, "method", "get_device_id");
        cJSON_AddItemToObject(ans, "device", Device_ans);

        char *ans_str = cJSON_Print(ans);
        memset(reply_data, 0, sizeof(reply_data));
        strcpy(reply_data, ans_str);
        if(ans_str != NULL){
            free(ans_str);
        }
        cJSON_Delete(Device_ans);
        cJSON_Delete(Device);
        cJSON_Delete(device_data);
        cJSON_Delete(ans);  
    }else if(!strcmp(method, "add_device")){
        char* opid=cJSON_GetStringValue(cJSON_GetObjectItem(root, "opid"));

        cJSON* SN = cJSON_GetObjectItem(root, "sn");   //gataway
        char* position=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"position"));
        char* sn=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"sn"));
        char* name=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"name"));
        char* floor=cJSON_GetStringValue(cJSON_GetObjectItem(SN,"floor"));

        cJSON* Device = cJSON_GetObjectItem(root, "device");   //device
        char* device_sn=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "sn"));
        char* device_mac=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_mac"));
        char* device_kind=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_kind"));
        char* device_id=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_id"));
        char* device_name=cJSON_GetStringValue(cJSON_GetObjectItem(Device, "device_name"));
        cJSON* device_data=cJSON_GetObjectItem(Device, "device_data");
        char* data_method=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_method"));
        char* data_num=cJSON_GetStringValue(cJSON_GetObjectItem(device_data, "data_num"));

        device device_info;
        device_info.device_mac=device_mac;
        device_info.sn=device_sn;
        device_info.device_id=device_id;
        device_info.device_kind=device_kind;
        device_info.device_name=device_name;
        mesh_sql_v.mysql_adddev(device_info);
        cJSON_Delete(Device);
        cJSON_Delete(device_data);
        strcpy(reply_data, ws_data);
    }
    
    cJSON_Delete(root);
    printf("ws data parse ok.\n");
    return 0;
}