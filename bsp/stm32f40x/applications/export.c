#include <board.h>
#include <rtthread.h>
#include <lwip/inet.h>
#include <lwip/netdb.h> /* Ϊޢ϶׷ܺĻìѨҪѼڬ netdb.h ͷτݾ */
#include <lwip/sockets.h> /* ʹԃ BSD socketìѨҪѼڬ sockets.h ͷτݾ */
#include <lwip/sys.h>
#include <lwip/api.h>
#include "cJSON.h"
#include "common.h"

static void process_json(const char* buf)
{
	cJSON * root = cJSON_Parse(buf);
	cJSON *format = RT_NULL;
	if (root == RT_NULL)
	{
		rt_kprintf("json parse error\n");
		return ;
	}
	format = cJSON_GetObjectItemCaseSensitive(root, "type");
	if (cJSON_IsString(format))
	{
		rt_kprintf("==>%s\n", format->valuestring);
	}
	cJSON_Delete(root);
}

static void rt_export_thread_entry(void* parameter)
{
	int fd,len;
	socklen_t addr_len;
	struct sockaddr_in server_addr, client_addr;
	static char buf[2048];
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
			process_json(buf);
			sendto(fd, buf, len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
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
