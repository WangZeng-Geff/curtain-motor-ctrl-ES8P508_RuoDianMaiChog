#ifndef _ES8P508_I2C_H_
#define _ES8P508_I2C_H_

/*
 * GPIO define
 */
/*******************************************/
 #ifdef USING_I2C1
 
#define GPIO_PORT_I2C1_SCL  GPIOA
#define PIN_I2C1_SCL      	GPIO_Pin_2 

#define GPIO_PORT_I2C1_SDA  GPIOA
#define PIN_I2C1_SDA      	GPIO_Pin_3

#endif
/*******************************************/
#ifdef USING_I2C2

#define GPIO_PORT_I2C2_SCL  GPIOA
#define PIN_I2C2_SCL      	GPIO_Pin_4

#define GPIO_PORT_I2C2_SDA  GPIOA
#define PIN_I2C2_SDA      	GPIO_Pin_5

#endif
/*******************************************/

int board_i2c_init(void);

#endif
