#ifndef	__DS1340_H__
#define	__DS1340_H__
#include "rtdevice.h"

typedef struct ds1340Device
{
	struct rt_device device;
	struct rt_i2c_bus_device *i2cBus;
}ds1340Device_t;

extern void DS1340Init(const char *name,const char * bus);
#endif


