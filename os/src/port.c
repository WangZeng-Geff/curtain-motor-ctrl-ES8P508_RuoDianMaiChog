#include <config.h>
#include <types.h>
#include <init.h>
#include "lib_config.h"
#include "SEGGER_RTT.h"
#include "port.h"


#define port_SYSTICK_CLK_BIT    (1UL << 2UL)
#define port_SYSTICK_INT_BIT    (1UL << 1UL)
#define port_SYSTICK_ENABLE_BIT (1UL << 0UL)

int setup_tick_service(void)
{
	SysTick->LOAD = (configSYS_CLOCK / configHZ) - 1UL;
	SysTick->VAL = 0;
    SysTick->CTRL = (port_SYSTICK_CLK_BIT | port_SYSTICK_INT_BIT | port_SYSTICK_ENABLE_BIT);

    return 0;
}
core_initcall(setup_tick_service);

void console_write_bytes(const void *in, size_t n)
{
#ifdef configUSING_SERIAL_DBG
	const u8 *_in = (const u8 *)in;
    while (n--)
	{
		while (UART_GetFlagStatus(UART0, UART_FLAG_TXIDLE) == RESET);
		UART_SendByte(UART0, *_in++);
	}
#endif
#ifdef configUSING_RTT_DBG
	RTT_write(in, n);
#endif
}

void setup_console(void)
{
#ifdef configUSING_SERIAL_DBG
	/* USART0 */
	#define CONSOLE_GPIO_TX           GPIO_Pin_1
	#define CONSOLE_GPIO_RX           GPIO_Pin_0
	#define CONSOLE_GPIO_AF           GPIO_Func_2
	#define CONSOLE_GPIO              GPIOC

    GPIO_InitStruType y;
    UART_InitStruType uart0;

    GPIO_RegUnLock();
    y.GPIO_Func      = CONSOLE_GPIO_AF;
    y.GPIO_Direction = GPIO_Dir_Out;
    y.GPIO_PUEN      = DISABLE;
    y.GPIO_PDEN      = DISABLE;
    y.GPIO_OD        = DISABLE;
    GPIO_Init(CONSOLE_GPIO, CONSOLE_GPIO_RX, &y);
    GPIO_Init(CONSOLE_GPIO, CONSOLE_GPIO_TX, &y);
    GPIO_RegLock();

    uart0.UART_StopBits = UART_StopBits_1;
    uart0.UART_TxMode   = UART_DataMode_8;
    uart0.UART_TxPolar  = UART_Polar_Normal;
    uart0.UART_RxMode   = UART_DataMode_8;
    uart0.UART_RxPolar  = UART_Polar_Normal;
    uart0.UART_BaudRate = 115200;
    uart0.UART_ClockSet = UART_Clock_1;
    UART_Init(UART0, &uart0);

    UART_TBIMConfig(UART0, UART_TBIM_Byte);
    UART_RBIMConfig(UART0, UART_TBIM_Byte);
    UART_ITConfig(UART0, UART_IT_RB, ENABLE);
    NVIC_Init(NVIC_UART0_IRQn, NVIC_Priority_1, ENABLE);
    UART0_TxEnable();
    UART0_RxEnable();
#endif
#ifdef configUSING_RTT_DBG
	RTT_init();
#endif
}

#ifdef configUSING_SERIAL_DBG
void UART0_IRQHandler(void)
{
    extern void shell_get_chr(char ch);

    if (UART_GetITStatus(UART0, UART_IT_RB))
    {

        while (UART0->IF.Word & UART_FLAG_RB)
        {
            shell_get_chr(UART0->RBR.Byte[0] & 0xff);
        }
    }
}
#endif
