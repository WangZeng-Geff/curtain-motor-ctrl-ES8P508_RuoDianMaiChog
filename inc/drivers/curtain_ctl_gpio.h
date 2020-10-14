#ifndef _CURTAIN_CTL_H_
#define _CURTAIN_CTL_H_

#include <types.h>
#include "lib_gpio.h"

#define SET_PIN_L(port,pin)            do{GPIO->PADATABCR.Word = (uint32_t)0x1<<pin;}while(0)
#define SET_PIN_H(port,pin)            do{GPIO->PADATABSR.Word = (uint32_t)0x1<<pin;}while(0)

#define   _OFF    0x00
#define   _ON     0x01

enum MOTOR_S
{
    MOTOR_PWR_OFF=0, MOTOR_FOFF, MOTOR_FON, MOTOR_F, MOTOR_ROFF, MOTOR_RON, MOTOR_R, MOTOR_END
};

struct MOTOR
{
   GPIO_TypeDef* GPIOx_f;
   GPIO_TYPE_PIN GPIO_Pin_f;
   GPIO_TypeDef* GPIOx_r;
   GPIO_TYPE_PIN GPIO_Pin_r;
   GPIO_TypeDef* GPIOx_off;
   GPIO_TYPE_PIN GPIO_Pin_off;

    uint8_t cur_state;
    uint8_t next_state;
    uint8_t delay;
    uint8_t motorflag;	
    uint8_t rotateflag;
    uint16_t time;
    uint16_t run_t;
};

enum
{
    MOTOR_STOP = 1,
    MOTOR_FORWARD,
    MOTOR_REVERSE
};


#define CHK_MOTOR_OFF_PORT  GPIOA
#define CHK_MOTOR_OFF_PIN   GPIO_Pin_1
#define CHK_MOTOR_F_PORT    GPIOA
#define CHK_MOTOR_F_PIN     GPIO_Pin_2
#define CHK_MOTOR_R_PORT    GPIOA
#define CHK_MOTOR_R_PIN     GPIO_Pin_3

#define MOTOR_OFF_PORT      GPIOA
#define MOTOR_OFF_PIN       GPIO_Pin_4
#define MOTOR_F_PORT        GPIOA
#define MOTOR_F_PIN         GPIO_Pin_5
#define MOTOR_R_PORT        GPIOA
#define MOTOR_R_PIN         GPIO_Pin_6


extern uint8_t relay_time;
void curtain_state_fun(void);
int set_motor_ctrl_inf(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen);
int set_motor_ctrl(const uint8_t * in, size_t len, uint8_t * out, size_t maxlen);
//void curtain_ctl_gpio_init(void);
#endif
