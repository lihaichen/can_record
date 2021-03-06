#include <board.h>
#include <rtthread.h>
#include <lwip/inet.h>
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include <lwip/sys.h>
#include <lwip/api.h>
#include <dfs_posix.h>
#include <time.h>
#include "cJSON.h"
#include "export.h"
#include "common.h"

#define	BUF_SIZE 			 2048
#define PAGE_SIZE 		 10
#define FILE_PAGE_SIZE 40
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
static const char *type_s[] = {"ping","setTime","getTime","setId","getId","rmFile","erase","readList","readFile"};

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
	cJSON *recv_type = RT_NULL, *recv_body = RT_NULL;
	type_e type = UNKNOW;
	// 返回的json变量定义
	// 发送的json的根, 发送的body节点
	cJSON *send_root = RT_NULL, *send_body = RT_NULL;
	// 打印的字符串指针
	char *print_p = RT_NULL;
	// 打印的长度
	int print_len = 0;
	static char tmp[FRAME_SIZE << 1];
	// static char readBuf1[FILE_PAGE_SIZE*FRAME_SIZE];
	// static int readBuf2[FILE_PAGE_SIZE*FRAME_SIZE];
	static char fileReadBuf[FILE_PAGE_SIZE * FRAME_SIZE + FRAME_SIZE];
	send_root = cJSON_CreateObject();
	if (recv_root == RT_NULL)
	{
		cJSON_AddItemToObject(send_root, "type", cJSON_CreateString("error"));
		cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(460));
		cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		goto RETURN;
	}
	// 判断类型是否支持
	recv_type = cJSON_GetObjectItemCaseSensitive(recv_root, "type");
	if (cJSON_IsString(recv_type))
	{
		type = find_cmd_type(recv_type->valuestring);
	}
	if(!cJSON_IsString(recv_type) || (type == UNKNOW))
	{
		cJSON_AddItemToObject(send_root, "type", cJSON_CreateString("error"));
		cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(470));
		cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		goto RETURN;
	}
	// 获取body节点
	recv_body = cJSON_GetObjectItemCaseSensitive(recv_root, "body");
	// 加入类型和body
	cJSON_AddItemToObject(send_root, "type", cJSON_CreateString(type_s[type]));
	switch(type)
	{
		case PING:
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_root, "body", cJSON_CreateString("pong"));
			break;
		case SET_TIME:
		{ 
			rt_device_t device;
			time_t time;
			if(!cJSON_IsNumber(recv_body))
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			 
			device = rt_device_find("rtc");
			if (device == RT_NULL)
			{
        cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(501));
				goto RETURN;
			}
			time = recv_body->valueint;
			time += 8*60*60;
			rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &time);
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		}
			break;
		case GET_TIME:
		{
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_root, "body", cJSON_CreateNumber(time(RT_NULL)));
		}
			break;
		case SET_ID:
		{
			int sum = 0, i = 0, fd = 0;
			if(!cJSON_IsArray(recv_body))
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			sum = cJSON_GetArraySize(recv_body);
			sum = sum > FILTER_ID_SIZE ? FILTER_ID_SIZE : sum;
			// 清空全部的id
			for(i = 0; i < FILTER_ID_SIZE; i++)
			{
				global.filter_id[i] = -1;
			}
			for(i = 0; i < sum; i++)
			{
				global.filter_id[i] = cJSON_GetArrayItem(recv_body, i)->valueint;
			}	
			fd = open(FILTER_ID_FILE,O_WRONLY | O_CREAT,0);
			if(fd < 0)
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(501));
				goto RETURN;
			}
			write(fd,global.filter_id,sizeof(global.filter_id));
			close(fd);
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		}
			break;
		case GET_ID:
		{
			int i = 0;
			cJSON_AddItemToObject(send_root, "body", send_body = cJSON_CreateArray());
			for(i = 0; i < FILTER_ID_SIZE; i++)
			{
				
				if(global.filter_id[i] < 0)
				{
					break;
				}
				cJSON_AddItemToArray(send_body,cJSON_CreateNumber(global.filter_id[i]));
			}
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
		}
			break;
		case RM_FILE:
		{
			int rm_sum = 0, i = 0;
			if(!cJSON_IsArray(recv_body))
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			rm_sum = cJSON_GetArraySize(recv_body);
			for(i = 0; i < rm_sum; i++)
			{
				rt_memset(tmp,0,sizeof(tmp));
				snprintf(tmp,sizeof(tmp)-1,"/%s",cJSON_GetArrayItem(recv_body, i)->valuestring);
				unlink(tmp);
			}
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		}
			break;
		case ERASE:
		{
			dfs_mkfs("elm","sd0");
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
			cJSON_AddItemToObject(send_root, "body", cJSON_CreateObject());
		}
			break;
		case READ_LIST:
		{
			DIR *dir;
			struct dirent *ptr;
			cJSON *recv_page = RT_NULL, *send_list = RT_NULL;
			int page = 0, sum = 0;
			recv_page = cJSON_GetObjectItemCaseSensitive(recv_body, "page");
			if (cJSON_IsNumber(recv_page))
			{
				page = recv_page->valueint;
			}
			if(page < 1)
			{	
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			cJSON_AddItemToObject(send_root, "body", send_body = cJSON_CreateObject());
			dir=opendir("/");
			if(dir == RT_NULL)
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(501));
				goto RETURN;
			} 
			send_list = cJSON_CreateArray();
			while ((ptr=readdir(dir)) != RT_NULL)
			{
				if(ptr->d_type == DFS_DT_REG)
				{
					int ch = -1, time = -1;
					rt_memset(tmp,0,sizeof(tmp));
					sscanf(ptr->d_name,"CH%d_%d.bin",&ch, &time);
					if(ch == -1 || time == -1)
					{
						rt_kprintf("invalid file %s\n", ptr->d_name);
						continue;
					}
					if((sum >= (page -1)* PAGE_SIZE) && (sum < page * PAGE_SIZE))
					{
						struct stat file_info;
						cJSON *item = cJSON_CreateObject();
						rt_memset(tmp,0,sizeof(tmp));
						snprintf(tmp,sizeof(tmp)-1,"/%s",ptr->d_name);
						stat(tmp,&file_info);
						cJSON_AddItemToObject(item, "file", cJSON_CreateString(ptr->d_name));
						cJSON_AddItemToObject(item, "size", cJSON_CreateNumber(file_info.st_size/FRAME_SIZE));
						cJSON_AddItemToArray(send_list,item);
					}
					sum ++;
				}
			}
			closedir(dir);
			cJSON_AddItemToObject(send_body, "page", cJSON_CreateNumber(page));
			cJSON_AddItemToObject(send_body, "pageSize", cJSON_CreateNumber(PAGE_SIZE));
			cJSON_AddItemToObject(send_body, "sum", cJSON_CreateNumber(sum));
			cJSON_AddItemToObject(send_body, "list", send_list);
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
		}
			break;
		case READ_FIME:
		{
#if 0			
			int page = 0, fd = -1, i = 0, readLen =0;
			cJSON *recv_page = RT_NULL, *recv_file = RT_NULL, *send_list = RT_NULL;
			const char *fileName = RT_NULL;
			recv_page = cJSON_GetObjectItemCaseSensitive(recv_body, "page");
			recv_file = cJSON_GetObjectItemCaseSensitive(recv_body, "file");
			if (cJSON_IsNumber(recv_page))
			{
				page = recv_page->valueint;
			}
			if(page < 1)
			{	
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			if (cJSON_IsString(recv_file))
			{
				fileName = recv_file->valuestring;
			}
			if (!cJSON_IsString(recv_file) || rt_strlen(fileName) < 1)
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			// 加入body
			cJSON_AddItemToObject(send_root, "body", send_body = cJSON_CreateObject());
			rt_memset(tmp,0,sizeof(tmp));
			snprintf(tmp,sizeof(tmp)-1,"/%s",fileName);
			fd = open(tmp,O_RDONLY,0);
			if(fd < 0)
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(404));
				goto RETURN;
			}
			lseek(fd, (page-1)*FILE_PAGE_SIZE*FRAME_SIZE,SEEK_SET);
			rt_memset(readBuf1,0,sizeof(readBuf1));
			rt_memset(readBuf2,0,sizeof(readBuf2));
			readLen = read(fd,readBuf1,sizeof(readBuf1));
			for(i =0; i < readLen; i++)
			{
				readBuf2[i] = readBuf1[i];
			}
			send_list = cJSON_CreateIntArray(readBuf2, readLen);
			close(fd);
			cJSON_AddItemToObject(send_body, "page", cJSON_CreateNumber(page));
			cJSON_AddItemToObject(send_body, "pageSize", cJSON_CreateNumber(FILE_PAGE_SIZE));
			cJSON_AddItemToObject(send_body, "list", send_list);
			cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(200));
#else
			int page = 0, fd = -1, readLen =0;
			cJSON *recv_page = RT_NULL, *recv_file = RT_NULL;
			const char *fileName = RT_NULL;
			// 获取页
			recv_page = cJSON_GetObjectItemCaseSensitive(recv_body, "page");
			// 获取文件
			recv_file = cJSON_GetObjectItemCaseSensitive(recv_body, "file");
			if (cJSON_IsNumber(recv_page))
			{
				page = recv_page->valueint;
			}
			if(page < 1)
			{	
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			if (cJSON_IsString(recv_file))
			{
				fileName = recv_file->valuestring;
			}
			if (!cJSON_IsString(recv_file) || rt_strlen(fileName) < 1)
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(403));
				goto RETURN;
			}
			rt_memset(tmp,0,sizeof(tmp));
			snprintf(tmp,sizeof(tmp)-1,"/%s",fileName);
			fd = open(tmp,O_RDONLY,0);
			if(fd < 0)
			{
				cJSON_AddItemToObject(send_root, "status", cJSON_CreateNumber(404));
				goto RETURN;
			}
			lseek(fd, (page-1)*FILE_PAGE_SIZE*FRAME_SIZE,SEEK_SET);
			rt_memset(fileReadBuf,0,sizeof(fileReadBuf));
			strcpy(fileReadBuf,type_s[READ_FIME]);
			readLen = read(fd,fileReadBuf + rt_strlen(type_s[READ_FIME]), FILE_PAGE_SIZE * FRAME_SIZE);
			close(fd);
			print_len = readLen + rt_strlen(type_s[READ_FIME]);
			if(recv_root != RT_NULL)
			{
				cJSON_Delete(recv_root);
			}
			if(send_root != RT_NULL)
			{
				cJSON_Delete(send_root);
			}
			rt_memset(buf, 0, BUF_SIZE);
			if(print_len >= BUF_SIZE)
			{
				print_len = BUF_SIZE - 1;
				rt_kprintf("print_len >= BUF_SIZE\n");
			}
			rt_memcpy(buf, fileReadBuf, print_len);
			return print_len;
#endif			
		}
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
	rt_memset(buf, 0, BUF_SIZE);
	if(print_len >= BUF_SIZE)
	{
		print_len = BUF_SIZE - 1;
		rt_kprintf("print_len >= BUF_SIZE\n");
	}
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
			if(send_len > 0)
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
        2048, 10, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
