#include "ds1340.h"
#include "rtdevice.h"
#include <time.h>

#define	  I2C_ADDR	0xD0
#define	  BCD_HEX(x) ((x>>4) * 10 + (x %16))
#define	  HEX_BCD(x) ((x/10 << 4) + (x%10))


static int readDS1340(struct rt_i2c_bus_device * i2c_device,unsigned char addr,unsigned char *data,int len)
{
	unsigned char r_addr;
	struct rt_i2c_msg msg[2];
	int err;
	r_addr = addr;
	msg[0].addr = I2C_ADDR>>1;
	msg[0].flags = RT_I2C_WR;
	msg[0].len = 1;
	msg[0].buf = &r_addr;
	
	msg[1].addr = I2C_ADDR>>1;
	msg[1].flags = RT_I2C_RD;
	msg[1].len = len;
	msg[1].buf = data;
	
	err = rt_i2c_transfer(i2c_device, msg, 2);
	if(err != 2)
		return -1;
	else
		return 0;
}

static int writeDS1340(struct rt_i2c_bus_device * i2c_device,unsigned char addr , unsigned char *data,int len)
{
	
	unsigned char w_addr;
	struct rt_i2c_msg msg[2];
	int err;
	w_addr = addr;
	msg[0].addr = I2C_ADDR>>1;
	msg[0].flags = RT_I2C_WR;
	msg[0].len = 1;
	msg[0].buf = &w_addr;
	
	msg[1].addr = I2C_ADDR>>1;
	msg[1].flags = RT_I2C_WR|RT_I2C_NO_START;
	msg[1].len = len;
	msg[1].buf = data;
	
	err =  rt_i2c_transfer(i2c_device, msg, 2);
	if(err != 2)
		return -1;
	else
		return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	time_t *time;
    struct tm time_temp;
	unsigned char buf[8];
	int err;
	ds1340Device_t *ds1340Device;
    RT_ASSERT(dev != RT_NULL);
	ds1340Device = (ds1340Device_t *)dev;
    rt_memset(&time_temp, 0, sizeof(struct tm));
	
    switch (cmd)
    {
		case RT_DEVICE_CTRL_RTC_GET_TIME:
		{
			time = (time_t *)args;
			rt_memset(buf,0,sizeof(buf));
			err = readDS1340(ds1340Device->i2cBus ,0x00,buf,7);
			if(err != 0)
			{
				return RT_ERROR;
			}
			time_temp.tm_year = BCD_HEX(buf[6]) + 2000 -1900;
			time_temp.tm_mon = BCD_HEX(buf[5]) - 1;
			time_temp.tm_mday = BCD_HEX(buf[4]) ;
			
			time_temp.tm_hour = BCD_HEX(buf[2]);
			time_temp.tm_min = BCD_HEX(buf[1]) ;
			time_temp.tm_sec = BCD_HEX(buf[0]) ;
			*time = mktime(&time_temp);
			return RT_EOK;
		}
       
		case RT_DEVICE_CTRL_RTC_SET_TIME:
		{
			const struct tm* time_new;
			time = (time_t *)args;
			time_new = localtime(time);
			rt_memset(buf,0,sizeof(buf));
			buf[6] = HEX_BCD((time_new->tm_year+1900-2000));
			buf[5] = HEX_BCD((time_new->tm_mon + 1));
			buf[4] = HEX_BCD(time_new->tm_mday);
			buf[3] = HEX_BCD((time_new->tm_wday + 1));
			buf[2] = HEX_BCD(time_new->tm_hour) ;
			buf[1] = HEX_BCD(time_new->tm_min);
			buf[0] = HEX_BCD(time_new->tm_sec);
			err = writeDS1340(ds1340Device->i2cBus,0x00,buf,7);
			if(err != 0)
			{
				return RT_ERROR;
			}
			return RT_EOK;
		}
	}
    return RT_EOK;
}
static int rtcConfiguration(struct rt_i2c_bus_device * i2c_device)
{
	unsigned char data = 0;
	int err;
	err = readDS1340(i2c_device,0x09,&data,1);
	if(err != 0)
	{
		rt_kprintf("rtc read ds1340 error\n");
		return RT_ERROR;
	}
	rt_kprintf("rtc read reg[0x09] value[%d]\n",data);
	if(data & 1<< 7)
	{
		unsigned char buf[7];
		rt_kprintf("config rtc[%d]...\n",data);
		data =((1<<6) | (1<<5) | (6<<0));
	
		err = writeDS1340(i2c_device,0x07,&data,1);
		if(err != 0)
		{
			return RT_ERROR;
		}

		rt_memset(buf,0,sizeof(buf));
		buf[6] = HEX_BCD(16);
		buf[5] = HEX_BCD(1);
		buf[4] = HEX_BCD(1);
		buf[3] = HEX_BCD(0);
		buf[2] = HEX_BCD(0) ;
		buf[1] = HEX_BCD(0);
		buf[0] = HEX_BCD(0);
		err = writeDS1340(i2c_device,0x00,buf,7);
		if(err != 0)
		{
			return RT_ERROR;
		}
		
		
		data = 0;
		err = writeDS1340(i2c_device,0x09,&data,1);
		if(err != 0)
		{
			return RT_ERROR;
		}
	}
	return RT_EOK;
}


void DS1340Init(const char *name,const char * bus)
{
	ds1340Device_t *ds1340Device;
	
	ds1340Device = rt_malloc(sizeof(ds1340Device_t));
	if(ds1340Device == NULL)
		return;
	rt_memset(ds1340Device,0,sizeof(ds1340Device_t));
    ds1340Device->device.type	= RT_Device_Class_RTC;

    /* register rtc device */
    ds1340Device->device.init 	= RT_NULL;
    ds1340Device->device.open 	= RT_NULL;
    ds1340Device->device.close	= RT_NULL;
    ds1340Device->device.read 	= RT_NULL;
    ds1340Device->device.write	= RT_NULL;
    ds1340Device->device.control = rt_rtc_control;
    /* no private */
    ds1340Device->device.user_data = RT_NULL;
	ds1340Device->i2cBus = rt_i2c_bus_device_find(bus);
	if(ds1340Device->i2cBus == NULL)
	{
		rt_kprintf("not find i2c device\n");
		return;
	}
	rtcConfiguration(ds1340Device->i2cBus);
    rt_device_register(&ds1340Device->device, name, RT_DEVICE_FLAG_RDWR);
    return;
}
