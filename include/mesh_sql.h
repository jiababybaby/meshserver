#ifndef _MESH_SQL_H
#define _MESH_SQL_H
#define max_sn 100
#define max_device 200
typedef struct device{//opid sn mac sn_position sn_name sn_status mac_class mac_status
    char *sn;
    char *device_mac;
    char *device_id;
    char *device_data;
    char *device_kind;
    char *device_name;
    char *device_status;
} device;
typedef struct user_gw{//opid sn mac sn_position sn_name sn_status mac_class mac_status
    char* opid;
    char* sn;
    char* sn_position;
    char* sn_floor;
    char* sn_name;
    char* sn_status;
} user_gw;
typedef struct sn_get_from_sql
{
    unsigned int num;
    user_gw user_gw_v[max_sn];
}sn_get_from_sql;
typedef struct device_get_from_sql
{
    unsigned int num;
    device device_v[max_sn];
}device_get_from_sql;
int sn_get_data_deinit(sn_get_from_sql *sn_get_from_sql_v, int num_i);
int device_get_data_deinit(device_get_from_sql *device_get_from_sql_v, int num_i);
static int meshsql_init();
static int add_device_data(device device_info);
static int mysql_adddev(device device_info);
static int mysql_get_device_id(device device_info, char *id);
static int mysql_addsn(user_gw gw_info);
static int mysql_delsn(user_gw gw_info);
static int mysql_delall(user_gw gw_info);
static int mysql_upsn(user_gw gw_info);
static int mysql_getsn(char *opid, sn_get_from_sql *sn_get_from_sql_v);
static int mysql_getdevice(user_gw gw_info, char *device_mac, device_get_from_sql *device_get_from_sql_v);
typedef struct mesh_sql{
    int (*init)();
    int (*add_device_data)(device device_info);
    int (*mysql_adddev)(device device_info);
    int (*mysql_get_device_id)(device device_info, char *id);
    int (*mysql_addsn)(user_gw gw_info);
    int (*mysql_delsn)(user_gw gw_info);
    int (*mysql_delall)(user_gw gw_info);
    int (*mysql_upsn)(user_gw gw_info);
    int (*mysql_getsn)(char *opid, sn_get_from_sql *sn_get_from_sql_v);
    int (*mysql_getdevice)(user_gw gw_info, char *device_mac, device_get_from_sql *device_get_from_sql_v);
}mesh_sql;
extern mesh_sql mesh_sql_v;
int meshsql_init(void);

#endif
