#include "i2c.h"
#include "rtdevice.h"
#include "board.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"

/* I2C2 */ 
#define I2C2_GPIO_SCL GPIO_Pin_8
#define I2C2_GPIO_SDA GPIO_Pin_9
#define I2C2_GPIO GPIOB

/* GPIO */
static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
    GPIO_InitStructure.GPIO_Pin = I2C2_GPIO_SCL|I2C2_GPIO_SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(I2C2_GPIO, &GPIO_InitStructure);
    GPIO_SetBits(I2C2_GPIO,I2C2_GPIO_SCL|I2C2_GPIO_SDA);     
}


    
/* rt_hw_i2c_init */

void stm32_set_sda(void *data, rt_int32_t state)
{
    if(state==1)
        GPIO_SetBits(I2C2_GPIO,I2C2_GPIO_SDA); 
    else if(state==0)
        GPIO_ResetBits(I2C2_GPIO,I2C2_GPIO_SDA); 
}

void stm32_set_scl(void *data, rt_int32_t state)
{
    if(state==1)
        GPIO_SetBits(I2C2_GPIO,I2C2_GPIO_SCL); 
    else if(state==0)
        GPIO_ResetBits(I2C2_GPIO,I2C2_GPIO_SCL); 
}

/* ¶Ái2cÏß×´Ì¬ */
rt_int32_t stm32_get_sda(void *data)
{
	 return GPIO_ReadInputDataBit(I2C2_GPIO,I2C2_GPIO_SDA);
}
rt_int32_t stm32_get_scl(void *data)
{
    return GPIO_ReadInputDataBit(I2C2_GPIO,I2C2_GPIO_SCL);
}

void stm32_udelay(rt_uint32_t us)
{
    rt_uint32_t delta;
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
    delta = SysTick->VAL;
    while (delta - SysTick->VAL< us);
}


static const struct  rt_i2c_bit_ops stm32_i2c_bit_ops =
{
//  .data    = 
    (void*)0xaa,
//  .set_sda = 
    stm32_set_sda,
//  .set_scl = 
    stm32_set_scl,
//  .get_sda = 
    stm32_get_sda,
//  .get_scl = 
    stm32_get_scl,
//  .udelay = 
    stm32_udelay,
//  .delay_us = 
    20,
//  .timeout = 
    5
};

void i2c_init(void)
{
    static struct rt_i2c_bus_device stm32_i2c;
    GPIO_Configuration();
    rt_memset((void *)&stm32_i2c, 0, sizeof(struct rt_i2c_bus_device));
    stm32_i2c.ops = (void *)&stm32_i2c_bit_ops;
		stm32_i2c.priv = (void *)&stm32_i2c_bit_ops;
    rt_i2c_bit_add_bus(&stm32_i2c, "i2c");
    
}
