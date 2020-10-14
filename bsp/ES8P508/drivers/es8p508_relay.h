#ifndef _ES8P508_RELAY_H_
#define _ES8P508_RELAY_H_


#define RELAY1_PORT       GPIOA
#define RELAY1_PIN        GPIO_Pin_12
#define RELAY2_PORT      GPIOA
#define RELAY2_PIN       GPIO_Pin_13

#define RELAY3_PORT       GPIOA
#define RELAY3_PIN        GPIO_Pin_14
#define RELAY4_PORT      GPIOA
#define RELAY4_PIN       GPIO_Pin_11

#define SET_PIN_H(port, pin)          GPIO_SetBit(port, pin)
#define SET_PIN_L(port, pin)          GPIO_ResetBit(port, pin)


#endif
