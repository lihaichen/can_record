#include <board.h>
#include <rtthread.h>
#include <lwip/inet.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include <lwip/sys.h>
#include <lwip/api.h>
#include "cJSON.h"
#include "export.h"
#include "common.h"

#define	BUF_SIZE 	2048

// 支持的命令列表
typedef enum 
{
	PING = 0,
	SET_TIME,
	GET_TIME,
	SET_ID,
	GET_ID,
	RM_FILE,
	ERASE,
	READ_LIST,
	READ_FIME,
	UNKNOW,
} type_e;
static const char *type_s[] = {"ping","set_time","get_time","set_id","get_id","rm_file","erase","read_list","read_file"};

static type_e find_cmd_type(char *s)
{
	int i = 0;
	for (i = 0; i <sizeof(type_s)/sizeof(char *); i++)
	{
		if(rt_strcmp(type_s[i], s) == 0)
			return (type_e)i;
	}
	return UNKNOW;
}

static int process_json(char* buf)
{
	// 接受变量定义
	// 解析json的根节点
	cJSON *recv_root = cJSON_Parse(buf);
	// 类型节点
	cJSON *type_node = RT_NULL;
	type_e type = UNKNOW;
	// 返回的json变量定义
	// 发送的json的根, 发送的body节点
	cJSON *send_root = RT_NULL, *send_body = RT_NULL;
	// 打印的字符串指针
	char *print_p = RT_NULL;
	// 打印的长度
	int print_len = 0;
	send_root = cJSON_CreateObject();
	if (recv_root == RT_NULL)
	{
		cJSON_AddItemToObject(send_root, "type", cJSON_CreateString("error"));
		cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(460));
		cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		rt_kprintf("json parse error\n");
		goto RETURN;
	}
	// 判断类型是否支持
	type_node = cJSON_GetObjectItemCaseSensitive(recv_root, "type");
	if (cJSON_IsString(type_node))
	{
		type = find_cmd_type(type_node->valuestring);
	}
	if(!cJSON_IsString(type_node) || (type == UNKNOW))
	{
		cJSON_AddItemToObject(send_root, "type", cJSON_CreateString("error"));
		cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(470));
		cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		goto RETURN;
	}
	// 加入类型和body
	cJSON_AddItemToObject(send_root, "type", cJSON_CreateString(type_s[type]));
	cJSON_AddItemToObject(send_root, "body", send_body = cJSON_CreateObject());
	switch(type)
	{
		case PING:
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_body, "type", cJSON_CreateString("pong"));
			break;
		default:
			break;
	}
	RETURN:
	if(recv_root != RT_NULL)
	{
		cJSON_Delete(recv_root);
	}
	print_p = cJSON_Print(send_root);
	print_len = rt_strlen(print_p);
	if(print_len >= BUF_SIZE)
	{
		print_len = BUF_SIZE - 1;
		rt_kprintf("print_len >= BUF_SIZE\n");
	}
	rt_memset(buf, 0, BUF_SIZE);
	rt_memcpy(buf, print_p, print_len);
	rt_free(print_p);
	cJSON_Delete(send_root);
	return print_len;
}

static void rt_export_thread_entry(void* parameter)
{
	int fd,len;
	socklen_t addr_len;
	struct sockaddr_in server_addr, client_addr;
	static char buf[BUF_SIZE];
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8989);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	rt_thread_delay(RT_TICK_PER_SECOND);
	addr_len = sizeof(client_addr);
	while(1)
	{
		fd = socket(AF_INET, SOCK_DGRAM, 0);//udp
		if (fd == -1)
		{
			rt_kprintf("create socker err[%d]\n",rt_get_errno());
			rt_thread_delay(RT_TICK_PER_SECOND << 2);
			continue;
		}else
		{
			break;	
		}
	}
		
	if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		rt_kprintf("bind socker err[%d]\n",rt_get_errno());
		return ;
	}
	
	while(1)
	{
		rt_memset(buf,0,sizeof(buf));
		len = recvfrom(fd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&client_addr, &addr_len);
		if(len > 0)
		{
			int send_len = process_json(buf);
			if(len > 0)
			{
				sendto(fd, buf, send_len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
			}
		}
	}
}
int rt_export_init()
{
	 rt_thread_t tid;

    tid = rt_thread_create("export",
        rt_export_thread_entry, RT_NULL,
        1024, 5, 50);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
