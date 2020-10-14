#ifndef	_ENCODE_H_
#define	_ENCODE_H_

#include "proto_smart.h"


#define ENCODE_LEN                  (SN_LEN+DKEY_LEN+AID_LEN+PW_LEN)


#define ENCODE_PORT                 GPIOA   	/* change depends on your mcu */
#define ENCODE_PIN                  GPIO_Pin_8  /* change depends on your mcu */
// #define ENCODE_PORT                 GPIOB  
// #define ENCODE_PIN                  GPIO_Pin_11

#define ENCODE_UART_BAUD            9600    	/* no need to change */
#define ENCODE_PERIOD               (configSYS_CLOCK / ENCODE_UART_BAUD - 1)

/* I/O's configuration depends on your mcu, but in general case, no need to change */
#define RX_SDA_VALUE()              GPIO_ReadBit(ENCODE_PORT, ENCODE_PIN)
#define TX_SDA_SET()                GPIO_SetBit(ENCODE_PORT, ENCODE_PIN)
#define TX_SDA_CLR()                GPIO_ResetBit(ENCODE_PORT, ENCODE_PIN)

#define ENCODE_SDA_IN() 			do{\
										GPIO_InitStruType gpio_init_struct;\
										gpio_init_struct.GPIO_Signal = GPIO_Pin_Signal_Digital;\
										gpio_init_struct.GPIO_Func 	= GPIO_Func_0;\
										gpio_init_struct.GPIO_Direction 	= GPIO_Dir_In;\
										gpio_init_struct.GPIO_PUEN 	= GPIO_PUE_Input_Disable;\
										gpio_init_struct.GPIO_PDEN 	= GPIO_PDE_Input_Disable;\
										gpio_init_struct.GPIO_OD	= GPIO_ODE_Output_Disable;\
										gpio_init_struct.GPIO_DS   	= GPIO_DS_Output_Normal;\
										GPIO_Init(ENCODE_PORT, ENCODE_PIN, &gpio_init_struct);\
									 }while(0);
#define ENCODE_SDA_OUT() 			do{\
										GPIO_InitStruType gpio_init_struct;\
										gpio_init_struct.GPIO_Signal = GPIO_Pin_Signal_Digital;\
										gpio_init_struct.GPIO_Func 	= GPIO_Func_0;\
										gpio_init_struct.GPIO_Direction 	= GPIO_Dir_Out;\
										gpio_init_struct.GPIO_PUEN 	= GPIO_PUE_Input_Enable;\
										gpio_init_struct.GPIO_PDEN 	= GPIO_PDE_Input_Disable;\
										gpio_init_struct.GPIO_OD   	= GPIO_ODE_Output_Disable;\
										gpio_init_struct.GPIO_DS   	= GPIO_DS_Output_Normal;\
										GPIO_Init(ENCODE_PORT, ENCODE_PIN, &gpio_init_struct);\
									 }while(0);
struct CODE_FRAME
{
    uint8_t start;
    uint8_t len;
    uint8_t data[1];
};

struct ENCODE_PARAM
{
    uint8_t sn[SN_LEN];
    uint8_t dkey[DKEY_LEN];
    uint8_t id[AID_LEN];
    uint8_t pwd[PW_LEN];
};

void device_encode_opt(void);

#endif
