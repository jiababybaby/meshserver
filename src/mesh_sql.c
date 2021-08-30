#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <stdarg.h>
#include "../include/mesh_sql.h"
MYSQL           mysql;
mesh_sql mesh_sql_v = {
    .init = meshsql_init,
    .add_device_data = add_device_data,
    .mysql_adddev = mysql_adddev,
    .mysql_get_device_id = mysql_get_device_id,
    .mysql_addsn = mysql_addsn,
    .mysql_delsn = mysql_delsn,
    .mysql_delall = mysql_delall,
    .mysql_upsn = mysql_upsn,
    .mysql_getsn = mysql_getsn,
    .mysql_getdevice = mysql_getdevice,
};
static int meshsql_init(void)
{

    MYSQL_RES *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;
    if (NULL == mysql_init(&mysql)) {
        printf("mysql_init(): %s\n", mysql_error(&mysql));
        return -1;
    }
    if (NULL == mysql_real_connect(&mysql,
                "localhost",
                "user",
                "123123",
                "mesh",
                0,
                NULL,
                0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&mysql));
        return -1;
    }
    printf("Connected MySQL successful! \n");
    
    query_str = "select * from user_gw_table";
    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }
    res = mysql_store_result(&mysql);
    if (NULL == res) {
         printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
         return -1;
    }
    rows = mysql_num_rows(res);
    printf("The total rows is: %d\n", rows);
    fields = mysql_num_fields(res);
    printf("The total fields is: %d\n", fields);
    while ((row = mysql_fetch_row(res))) {
        for (i = 0; i < fields; i++) {
            printf("%s\t", row[i]);
        }
        printf("\n");
    }
    return 0;
}
static int add_quo(char *str, int n, ...){
    if(str == NULL){
        printf("add_quo error\n");
        return -1;
    }
    va_list args;
    char *args1;
    va_start(args,n);
    args1 = va_arg(args,char *);
    while(n--){
        
        if(args1 != NULL)
        {
            // printf("%s\n",args1);
            strcat(str, args1);
        }else{
           
            strcat(str,"''");
        }
        args1 = va_arg(args,char *);
    }
    va_end(args);
    return 0;
}
static int mysql_addsn(user_gw gw_info){
    printf("add sn\n");
    char command[500] = {0};
    printf("%s%s%s%s%s\n",gw_info.opid, gw_info.sn, gw_info.sn_position, gw_info.sn_name
    , gw_info.sn_status);
    add_quo(command, 11, "insert into mesh.user_gw_table set opid='", gw_info.opid,
            "',sn='", gw_info.sn, "',sn_position='", gw_info.sn_position, "',sn_name='", gw_info.sn_name,
            "',sn_status='", gw_info.sn_status,"'");
    printf("add sn ok\n");
    printf("%s\n",command);
    mysql_real_query(&mysql, command, strlen(command));
    return 0;
}
static int mysql_delsn(user_gw gw_info){
    char command[100]={0};
    add_quo(command, 5, "DELETE FROM user_gw_table where opid ='", gw_info.opid, "'and sn='",
            gw_info.sn, "'");
    printf("%s\n", command);
    mysql_real_query(&mysql, command, strlen(command));
    return 0;
}
static int mysql_delall(user_gw gw_info){
    char command[100]={0};
    add_quo(command, 3, "DELETE FROM user_gw_table where opid ='", gw_info.opid, "'");
    printf("%s\n", command);
    mysql_real_query(&mysql, command, strlen(command));
    return 0;
}
static int mysql_upsn(user_gw gw_info){
    char command[100]={0};
    add_quo(command, 19, "update user_gw_table Set opid='", gw_info.opid, "',sn='", gw_info.sn,
        "',sn_position='", gw_info.sn_position, "',sn_name='", gw_info.sn_name,
         "',sn_status='", gw_info.sn_status, "',sn_name='", gw_info.sn_name, "',sn_status='",
        gw_info.sn_status, "'WHERE opid ='", gw_info.opid, "' and sn ='", gw_info.sn, "'limit 1");
    printf("%s\n",command);
    mysql_real_query(&mysql, command, strlen(command));
    return 0;
}
static int sn_get_data_init(sn_get_from_sql *sn_get_from_sql_v, int num_i){
    char **p = NULL;
    int size = sizeof(sn_get_from_sql_v->user_gw_v[num_i]) / sizeof(sn_get_from_sql_v->user_gw_v[num_i].opid);
    printf("%d\n", size);
    for (int i = 0; i < size; i++)
    {
        p= (void *)(&( sn_get_from_sql_v->user_gw_v[num_i].opid) + i);
        *p= malloc(100);
        memset(*p, 0, 100);
        if (*p != NULL)
        {
             printf("malloc ok\n");
        }
    }
    if(sn_get_from_sql_v->user_gw_v[num_i].opid == NULL
    ||sn_get_from_sql_v->user_gw_v[num_i].sn == NULL||
    sn_get_from_sql_v->user_gw_v[num_i].sn_floor == NULL||
    sn_get_from_sql_v->user_gw_v[num_i].sn_name == NULL ||
    sn_get_from_sql_v->user_gw_v[num_i].sn_position == NULL||
    sn_get_from_sql_v->user_gw_v[num_i].sn_status == NULL
    ){
        printf("malloc error\n");
    }
}
int sn_get_data_deinit(sn_get_from_sql *sn_get_from_sql_v, int num_i){
    if(sn_get_from_sql_v ==NULL)
        return -1;
   if(sn_get_from_sql_v->user_gw_v[num_i].opid == NULL
    ||sn_get_from_sql_v->user_gw_v[num_i].sn == NULL||
    sn_get_from_sql_v->user_gw_v[num_i].sn_floor == NULL||
    sn_get_from_sql_v->user_gw_v[num_i].sn_name == NULL ||
    sn_get_from_sql_v->user_gw_v[num_i].sn_position == NULL||
    sn_get_from_sql_v->user_gw_v[num_i].sn_status == NULL
    ){
        printf("free sn null\n");
        return -1;
   }
    char **p = NULL;
    int size = sizeof(sn_get_from_sql_v->user_gw_v[num_i]) / sizeof(sn_get_from_sql_v->user_gw_v[num_i].opid);
    printf("sn size: %d\n",size);
    for (int i = 0; i < size; i++)
    {
        p= (void *)(&( sn_get_from_sql_v->user_gw_v[num_i].opid) + i); 
        if(p != NULL && *p != NULL){
            free(*p);
            *p = NULL;
        }
    }
}
static int device_get_data_init(device_get_from_sql *device_get_from_sql_v, int num_i){
    char **p = NULL;
    int size = sizeof(device_get_from_sql_v->device_v[num_i]) / sizeof(device_get_from_sql_v->device_v[num_i].sn);
    printf("%d\n", size);
    for (int i = 0; i < size; i++)
    {
        p= (void *)(&( device_get_from_sql_v->device_v[num_i].sn) + i);
        *p= malloc(100);
        memset(*p, 0, 100);
        if (*p != NULL)
        {
             printf("malloc ok\n");
        }
    }
    // if(device_get_from_sql_v->user_gw_v[num_i].opid == NULL
    // ||device_get_from_sql_v->user_gw_v[num_i].sn == NULL||
    // device_get_from_sql_v->user_gw_v[num_i].sn_floor == NULL||
    // device_get_from_sql_v->user_gw_v[num_i].sn_name == NULL ||
    // device_get_from_sql_v->user_gw_v[num_i].sn_position == NULL||
    // device_get_from_sql_v->user_gw_v[num_i].sn_status == NULL
    // ){
    //     printf("malloc error\n");
    // }
}
int device_get_data_deinit(device_get_from_sql *device_get_from_sql_v, int num_i){
    char **p = NULL;
    int size = sizeof(device_get_from_sql_v->device_v[num_i]) / sizeof(device_get_from_sql_v->device_v[num_i].sn);
    for (int i = 0; i < size; i++){
        p= (void *)(&( device_get_from_sql_v->device_v[num_i].sn) + i); 
        if(*p != NULL){
            free(*p);
        }
    }
}
static int mysql_getsn(char *opid, sn_get_from_sql *sn_get_from_sql_v){
    if(opid == NULL || sn_get_from_sql_v == NULL){
        return -1;
    }
    memset(sn_get_from_sql_v, 0, sizeof(sn_get_from_sql));
    char command[100] = {0};
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;
    char *utf8_str = "SET NAMES 'utf8'";
    mysql_real_query(&mysql, utf8_str, strlen(utf8_str));
    add_quo(command, 3, "select * from (select * from user_gw_table order by time desc LIMIT 10000 )as a\
      where opid ='", opid, "' group by sn");
    printf("%s\n", command);
    rc = mysql_real_query(&mysql, command, strlen(command));

    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }
    res = mysql_store_result(&mysql);
    if (NULL == res) {
         printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
         return -1;
    }
    rows = mysql_num_rows(res);
    printf("The total rows is: %d\n", rows);
    fields = mysql_num_fields(res);
    printf("The total fields is: %d\n", fields);
    char column[20][100];
    memset(column, 0, 20*100);
    for (int i = 0; i < fields; ++i)
    {
		strcpy(column[i], mysql_fetch_field(res)->name);
		 printf("%s\t", column[i]);
    }
    printf("\n");
    while ((row = mysql_fetch_row(res)))
    {
        sn_get_data_init(sn_get_from_sql_v, sn_get_from_sql_v->num);
        for (i = 0; i < fields; i++)
        {
            printf("%s\n", row[i]);
            if(!strcmp(column[i],"opid")){
                add_quo(sn_get_from_sql_v->user_gw_v[sn_get_from_sql_v->num].opid, 1, row[i]);
            }
            if(!strcmp(column[i],"sn")){
                add_quo(sn_get_from_sql_v->user_gw_v[sn_get_from_sql_v->num].sn, 1, row[i]);
            }
            if(!strcmp(column[i],"sn_name")){
                add_quo(sn_get_from_sql_v->user_gw_v[sn_get_from_sql_v->num].sn_name, 1, row[i]);
                
            }
            if(!strcmp(column[i],"sn_position")){
                add_quo(sn_get_from_sql_v->user_gw_v[sn_get_from_sql_v->num].sn_position, 1, row[i]);
               
            }
            if(!strcmp(column[i],"sn_floor")){
                add_quo(sn_get_from_sql_v->user_gw_v[sn_get_from_sql_v->num].sn_floor, 1, row[i]);
               
            }
            if(!strcmp(column[i],"sn_status")){
                add_quo(sn_get_from_sql_v->user_gw_v[sn_get_from_sql_v->num].sn_status, 1, row[i]);
                
            }
            
            
        }
        sn_get_from_sql_v->num++;
        printf("\n");
    }
    return 0;
}
static int mysql_getdevice(user_gw gw_info, char *device_mac, device_get_from_sql *device_get_from_sql_v){
    printf("mac %s\n", device_mac);
    if (device_mac==NULL || strlen(device_mac) == 0)
    {
        printf("mac 0\n");
        sn_get_from_sql sn_get_from_sql_d;
        memset(&sn_get_from_sql_d, 0, sizeof(sn_get_from_sql));
        memset(device_get_from_sql_v, 0, sizeof(device_get_from_sql));
        mysql_getsn(gw_info.opid, &sn_get_from_sql_d);
        if(sn_get_from_sql_d.num>0){
            for(int i = 0; i < sn_get_from_sql_d.num; i++){
                char command[500]={0};
                MYSQL_RES       *res = NULL;
                MYSQL_ROW       row = NULL;
                char            *query_str = NULL;
                int             rc=0, j=0, fields=0;
                int             rows =0;
                printf("sn: %x\n",sn_get_from_sql_d.user_gw_v[i].sn);
                add_quo(command, 3, "select * from (select * from (\
                                       select * from mesh.device_table order by time desc limit 100000\
                                   ) as t\
                                   group by t.device_mac) as d where sn = '",
                                   sn_get_from_sql_d.user_gw_v[i].sn, "'");
                printf("%s\n", command);
                rc = mysql_real_query(&mysql, command, strlen(command));

                if (0 != rc) {
                    printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                    return -1;
                }
                res = mysql_store_result(&mysql);
                if (NULL == res) {
                    printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
                    return -1;
                }
                rows = mysql_num_rows(res);
                printf("The total rows is: %d\n", rows);
                fields = mysql_num_fields(res);
                printf("The total fields is: %d\n", fields);
                char column[20][100];
                memset(column, 0, 20*100);
                for (j = 0; j < fields; ++j)
                {
                    strcpy(column[j], mysql_fetch_field(res)->name);
                }
                while ((row = mysql_fetch_row(res))) {

                    device_get_data_init(device_get_from_sql_v,device_get_from_sql_v->num);

                    for (j = 0; j < fields; j++)
                    {
                        printf("%s\t", row[j]);
                        if(!strcmp(column[j],"sn")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].sn, 1, row[j]);
                            continue;
                        }
                        if(!strcmp(column[j],"device_kind")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_kind, 1, row[j]);
                            continue;
                        }
                        if(!strcmp(column[j],"device_name")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_name, 1, row[j]);
                            continue;
                        }
                        if(!strcmp(column[j],"device_mac")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_mac, 1, row[j]);
                            continue;
                        }
                        if(!strcmp(column[j],"device_id")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_id, 1, row[j]);
                            continue;
                        }
                        if(!strcmp(column[j],"device_status")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_status, 1, row[j]);
                            continue;
                        }
                        if(!strcmp(column[j],"device_data")){
                            add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_data, 1, row[j]);
                            continue;
                        }                        
                    }
                    device_get_from_sql_v->num++;
                    printf("\n");
                }
            }
            printf("get device ok\n");
            for (int i = 0; i < sn_get_from_sql_d.num; i++)
            {
                sn_get_data_deinit(&sn_get_from_sql_d, i);
            }
            
        }
        printf("sn deint ok\n");
        return 0;
    }
    else
    {

        printf("mac%s\n", device_mac);
        memset(device_get_from_sql_v, 0, sizeof(device_get_from_sql));
        char command[500] = {0};
        MYSQL_RES       *res = NULL;
        MYSQL_ROW       row;
        char            *query_str = NULL;
        int             rc, i, fields;
        int             rows;
        add_quo(command, 3, "select * from (select * from (\
                                       \
                                       select * from mesh.device_table order by time desc limit 100000)\
                                    as d where device_mac='", device_mac, 
                                    "') as t group by t.device_id");
        printf("%s\n", command);
        rc = mysql_real_query(&mysql, command, strlen(command));

        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            return -1;
        }
        res = mysql_store_result(&mysql);
        if (NULL == res) {
            printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
            return -1;
        }
        rows = mysql_num_rows(res);
        printf("The total rows is: %d\n", rows);
        fields = mysql_num_fields(res);
        printf("The total fields is: %d\n", fields);
        char column[20][100];
        memset(column, 0, 20*100);
        for (int i = 0; i < fields; ++i)
        {
            strcpy(column[i], mysql_fetch_field(res)->name);
            // printf("%s\n", column[i]);
        }
        while ((row = mysql_fetch_row(res))) {
            device_get_data_init(device_get_from_sql_v, device_get_from_sql_v->num);
            for (i = 0; i < fields; i++)
            {
                printf("%s\t", row[i]);
                if(!strcmp(column[i],"sn")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].sn, 1, row[i]);
                    continue;
                }
                if(!strcmp(column[i],"device_kind")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_kind, 1, row[i]);
                    continue;
                }
                if(!strcmp(column[i],"device_name")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_name, 1, row[i]);
                    continue;
                }
                if(!strcmp(column[i],"device_mac")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_mac, 1, row[i]);
                    continue;
                }
                if(!strcmp(column[i],"device_id")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_id, 1, row[i]);
                    continue;
                }
                if(!strcmp(column[i],"device_status")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_status, 1, row[i]);
                    continue;
                }
                if(!strcmp(column[i],"device_data")){
                    add_quo(device_get_from_sql_v->device_v[device_get_from_sql_v->num].device_data, 1, row[i]);
                    continue;
                }                
            }
            device_get_from_sql_v->num++;
            printf("\n");
        }
        return 0;
    }
}
static int table_init(){
    char command[100] = {0};
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;
    add_quo(command, 1, "CREATE TABLE `mesh`.`device_table`( \
                    `sn` VARCHAR(45) NULL, \
                    `device_mac` VARCHAR(45) NULL,\
                    `device_kind` VARCHAR(45) NULL,\
                    `device_id` VARCHAR(45) NULL,\
                    `device_data` VARCHAR(45) NULL,\
                    `device_name` VARCHAR(45) NULL,\
                    `device_status` VARCHAR(45) NULL,\
                    `time` datetime NULL DEFAULT CURRENT_TIMESTAMP)");
    printf("%s\n", command);
    rc = mysql_real_query(&mysql, command, strlen(command));

    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    }
    memset(command, 0, sizeof(command));
    add_quo(command, 1, "CREATE TABLE `mesh`.`user_gw_table` (\
            `opid` VARCHAR(45) NULL,\
            `sn` VARCHAR(45) NULL,\
            `sn_position` VARCHAR(45) NULL,\
            `sn_name` VARCHAR(45) NULL,\
             `sn_status` VARCHAR(45) NULL,\
              `time` datetime NULL DEFAULT CURRENT_TIMESTAMP\
             );");
    printf(command);
    rc = mysql_real_query(&mysql, command, strlen(command));

    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    } 
}
static int add_device_data(device device_info){
    printf("add_device_data 123\n");
    char command[500] = {0};
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;
    add_quo(command, 11, "insert into mesh.device_table set sn='",
    device_info.sn, "',device_mac='", device_info.device_mac, "',device_kind='", 
    device_info.device_kind, "',device_id='", device_info.device_id, "',device_data='",
    device_info.device_data, "'");
    printf("%s\n", command);
    rc = mysql_real_query(&mysql, command, strlen(command));

    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    } 

}
static int mysql_get_device_id(device device_info, char * id){
    if(id == NULL){
        return -1;
    }
    char command[100] = {0};
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;
    for(i=1;i<255;i++){
        char num_i[10] = {0};
        sprintf(num_i, "%d", i);
        add_quo(command, 7, "select * from mesh.device_table where sn='",
                device_info.sn, "'and device_mac='", device_info.device_mac,
                "'and device_id='", num_i, "'");
        printf("%s\n", command);
        rc = mysql_real_query(&mysql, command, strlen(command));

        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            return -1;
        } 
        res = mysql_store_result(&mysql);
        if (NULL == res) {
            printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
            return -1;
        }
        int rows = mysql_num_rows(res);
        if (rows)
        {
            printf("find id  %d : %d个\r\n",i, rows);
        }
        else
        {
            printf("get id %d\r\n",i);
            strcpy(id,num_i);
        }
    }
    return 0;

}
static int mysql_adddev(device device_info){
    char command[100] = {0};
    MYSQL_RES       *res = NULL;
    MYSQL_ROW       row;
    char            *query_str = NULL;
    int             rc, i, fields;
    int             rows;
    add_quo(command, 13, "insert into mesh.device_table set sn='", device_info.sn,
    "',device_mac='", device_info.device_mac, "',device_kind='", device_info.device_kind,
    "',device_name='", device_info.device_name, "',device_id='" ,device_info.device_id, 
    "',device_data='", device_info.device_data, "'");
    printf("%s\n", command);
    rc = mysql_real_query(&mysql, command, strlen(command));
    if (0 != rc) {
        printf("mysql_real_query(): %s\n", mysql_error(&mysql));
        return -1;
    } 
}
