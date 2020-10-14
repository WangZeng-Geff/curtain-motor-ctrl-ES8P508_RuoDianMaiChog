#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <bitops.h>
#include <device.h>
#include <printk.h>
#include <syserr.h>
#include <settings.h>
#include "lib_config.h"
#include "board.h"
#include "os.h"
#include "dev.h"
#include "init.h"
#include "proto_smart.h"
#include "curtain_ctl_gpio.h"

uint8_t relay_time;
struct MOTOR Motor1 = 
{
    MOTOR_F_PORT,MOTOR_F_PIN,MOTOR_R_PORT,MOTOR_R_PIN,MOTOR_OFF_PORT,MOTOR_OFF_PIN,MOTOR_ROFF,MOTOR_ROFF, 0, 0, 0, 0, 0
};

static int curtain_ctl_gpio_init(void)
{
	GPIO_InitStruType y;
	
	y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_Out;
	y.GPIO_PUEN			= GPIO_PUE_Input_Disable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
  	GPIO_Init(MOTOR_F_PORT, MOTOR_F_PIN, &y);
	GPIO_Init(MOTOR_R_PORT, MOTOR_R_PIN, &y);
  	GPIO_Init(MOTOR_OFF_PORT, MOTOR_OFF_PIN, &y);

    y.GPIO_Signal		= GPIO_Pin_Signal_Digital;
	y.GPIO_Func			= GPIO_Func_0;
	y.GPIO_Direction	= GPIO_Dir_In;
	y.GPIO_PUEN			= GPIO_PUE_Input_Enable;
	y.GPIO_PDEN			= GPIO_PDE_Input_Disable;
	y.GPIO_OD			= GPIO_ODE_Output_Disable;
	y.GPIO_DS			= GPIO_DS_Output_Normal;
	
  	GPIO_Init(CHK_MOTOR_F_PORT, CHK_MOTOR_F_PIN, &y);
	GPIO_Init(CHK_MOTOR_R_PORT, CHK_MOTOR_R_PIN, &y);
  	GPIO_Init(CHK_MOTOR_OFF_PORT, CHK_MOTOR_OFF_PIN, &y);

    return 0;
}

device_initcall(curtain_ctl_gpio_init);

int set_motor_ctrl_inf(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
    uint8_t cmd;
    int cnt = 960000;

    if(len != 0x04) return(-LEN_ERR);

    cmd = in[1] & 0x0F;

   if (cmd == MOTOR_FORWARD)
    {
        SET_PIN_H(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_time = setting.dev_infor.actuation_time;

        while (--cnt)
            if (cnt == 0) 
                break;

        if (RESET != (GPIO_ReadBit(CHK_MOTOR_F_PORT, CHK_MOTOR_F_PIN)))
            return (-DATA_ERR);
    }
    else if (cmd == MOTOR_REVERSE)
    {
        SET_PIN_H(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_time = setting.dev_infor.actuation_time;

        while (--cnt)
            if (cnt == 0) 
                break;

        if (RESET != (GPIO_ReadBit(CHK_MOTOR_R_PORT, CHK_MOTOR_R_PIN)))
            return (-DATA_ERR);
    }
    else if (cmd == MOTOR_STOP) 
    {
        SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        relay_time = setting.dev_infor.actuation_time;

        while (--cnt)
            if (cnt == 0) 
                break;

        if (RESET != (GPIO_ReadBit(CHK_MOTOR_OFF_PORT, CHK_MOTOR_OFF_PIN)))
            return (-DATA_ERR);
    }
    else
    {
        return (-DATA_ERR);
    }

    return(NO_ERR);
}

int set_motor_ctrl(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen)
{
    uint8_t cmd;
    int cnt = 960000;

    if(len != 0x01) return(-LEN_ERR);

    cmd = in[0];

   if (cmd == MOTOR_FORWARD)
    {
        SET_PIN_H(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_time = setting.dev_infor.actuation_time;

        while (--cnt)
            if (cnt == 0) 
                break;

        if (RESET != (GPIO_ReadBit(CHK_MOTOR_F_PORT, CHK_MOTOR_F_PIN)))
            return (-DATA_ERR);
    }
    else if (cmd == MOTOR_REVERSE)
    {
        SET_PIN_H(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_time = setting.dev_infor.actuation_time;

        while (--cnt)
            if (cnt == 0) 
                break;

        if (RESET != (GPIO_ReadBit(CHK_MOTOR_R_PORT, CHK_MOTOR_R_PIN)))
            return (-DATA_ERR);
    }
    else if (cmd == MOTOR_STOP) 
    {
        SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        relay_time = setting.dev_infor.actuation_time;

        while (--cnt)
            if (cnt == 0) 
                break;

        if (RESET != (GPIO_ReadBit(CHK_MOTOR_OFF_PORT, CHK_MOTOR_OFF_PIN)))
            return (-DATA_ERR);
    }
    else
    {
       return (-DATA_ERR);
    }

    return(NO_ERR);
}


