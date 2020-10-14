#include <config.h>
#include <board.h>
#include <croutine.h>
#include "lib_config.h"
int main(void)
{
    disable_irq();

    board_setup();

#ifdef configTEST
    extern void test_setup(void);
    test_setup();
#endif
    extern void setup_app(void);
    setup_app();

    enable_irq();
	
		GPIO_InitStruType gpio_init_struct;
		gpio_init_struct.GPIO_Signal = GPIO_Pin_Signal_Digital;
		gpio_init_struct.GPIO_Func 	= GPIO_Func_0;
		gpio_init_struct.GPIO_Direction 	= GPIO_Dir_Out;
		gpio_init_struct.GPIO_PUEN 	= GPIO_PUE_Input_Enable;
		gpio_init_struct.GPIO_PDEN 	= GPIO_PDE_Input_Disable;
		gpio_init_struct.GPIO_OD   	= GPIO_ODE_Output_Disable;
		gpio_init_struct.GPIO_DS   	= GPIO_DS_Output_Normal;
		GPIO_Init(GPIOB, GPIO_Pin_11, &gpio_init_struct);
		
    for (;;)
    {
        task_schedule();
    }
}
