#ifndef _ES8P508_KEY_H_
#define _ES8P508_KEY_H_

//hardware pin define
#define KEY1_PORT       GPIOB
#define KEY1_PIN        GPIO_Pin_3
#define KEY2_PORT       GPIOB
#define KEY2_PIN        GPIO_Pin_4
#define KEY3_PORT       GPIOB
#define KEY3_PIN        GPIO_Pin_5
#define KEY4_PORT       GPIOB
#define KEY4_PIN        GPIO_Pin_6
#define KEY5_PORT       GPIOB
#define KEY5_PIN        GPIO_Pin_7


int board_key_init(void);

#endif
