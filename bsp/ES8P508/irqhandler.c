#include "irqhandler.h"
#include "lib_config.h"
#include "jiffies.h"
#include "printk.h"
#include "dev.h"
#include <device.h>


void NMI_IRQHandler(void)
{
	log_d("NMI_IRQHandler!\n");
}

void HardFault_IRQHandler(void)
{
	log_d("HardFault!\n");
    while (1);
}

void SVC_IRQHandler(void)
{
	log_d("SVC_IRQHandler!\n");
}

void PendSV_IRQHandler(void)
{
	log_d("PendSV_IRQHandler!\n");
}

//10ms jiffies
void SysTick_IRQHandler(void)
{
    jiffies++;
// 		static int i = 0;
// 		if(i++ == 10)
// 		{
// 			i = 0;
			GPIO_ToggleBit(GPIOB, GPIO_Pin_11);
// 		}

}

void PINT4_IRQHandler(void)
{
	if(PINT_GetITStatus(PINT_IT_PINT4) != RESET)
    {

    }
}

void T16N1_IRQHandler(void)
{
	if(T16Nx_GetITStatus(T16N1,TIM_IT_MAT0) != RESET)
    {

    }
}
