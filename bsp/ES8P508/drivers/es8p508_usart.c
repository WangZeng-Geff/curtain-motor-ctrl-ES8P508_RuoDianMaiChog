#include <utils.h>
#include <config.h>
#include <printk.h>
#include <init.h>
#include "ES8P508x.h"
#include "lib_config.h"
#include "device.h"
#include "es8p508_usart.h"

#ifdef configUSING_SERIAL

/* USART0 */
#define UART0_GPIO_TX           GPIO_Pin_1
#define UART0_GPIO_RX           GPIO_Pin_0
#define UART0_GPIO_AF           GPIO_Func_2
#define UART0_GPIO              GPIOB

/* USART1 */
#define UART1_GPIO_TX           GPIO_Pin_3
#define UART1_GPIO_RX           GPIO_Pin_2
#define UART1_GPIO_AF           GPIO_Func_2
#define UART1_GPIO              GPIOB

/* USART2 */
#define UART2_GPIO_TX           GPIO_Pin_15
#define UART2_GPIO_RX           GPIO_Pin_16
#define UART2_GPIO_AF           GPIO_Func_2
#define UART2_GPIO              GPIOA

/* USART3 */
#define UART3_GPIO_TX           GPIO_Pin_13
#define UART3_GPIO_RX           GPIO_Pin_14
#define UART3_GPIO_AF           GPIO_Func_2
#define UART3_GPIO              GPIOA

/* USART4 */
#define UART4_GPIO_TX           GPIO_Pin_11
#define UART4_GPIO_RX           GPIO_Pin_10
#define UART4_GPIO_AF           GPIO_Func_1
#define UART4_GPIO              GPIOB

/* USART5 */
#define UART5_GPIO_TX           GPIO_Pin_13
#define UART5_GPIO_RX           GPIO_Pin_12
#define UART5_GPIO_AF           GPIO_Func_1
#define UART5_GPIO              GPIOB
/* hr8p296 uart driver */
struct es8p508_uart
{
    UART_TypeDef *uart_device;
    NVIC_IRQChannel irq;
};

static void uart_interrupt_handler(struct serial_device *serial);

#if defined(configUSING_UART0)
struct es8p508_uart uart0 =
{
    UART0,
    NVIC_UART0_IRQn,
};
static struct serial_device serial0;

void UART0_IRQHandler(void)
{
    uart_interrupt_handler(&serial0);
}
#endif

#if defined(configUSING_UART1)
struct es8p508_uart uart1 =
{
    UART1,
    NVIC_UART1_IRQn,
};
static struct serial_device serial1;

void UART1_IRQHandler(void)
{
    uart_interrupt_handler(&serial1);
}
#endif

#if defined(configUSING_UART2)
struct es8p508_uart uart2 =
{
    UART2,
    NVIC_UART2_IRQn,
};
static struct serial_device serial2;

void UART2_IRQHandler(void)
{
    uart_interrupt_handler(&serial2);
}
#endif

#if defined(configUSING_UART3)
struct es8p508_uart uart3 =
{
    UART3,
    NVIC_UART3_IRQn,
};
static struct serial_device serial3;

void UART3_IRQHandler(void)
{
    uart_interrupt_handler(&serial3);
}
#endif

static void GPIO_Configuration(void)
{
    GPIO_InitStruType y;

#ifdef configUSING_UART0
	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = UART0_GPIO_AF;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Normal;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
    y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	GPIO_Init(UART0_GPIO, UART0_GPIO_TX, &y); //TXD - PB1
	
	y.GPIO_Direction = GPIO_Dir_In;
	GPIO_Init(UART0_GPIO, UART0_GPIO_RX, &y); //RXD - PB0
#endif

#ifdef configUSING_UART1
	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = UART1_GPIO_AF;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Normal;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
    y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	GPIO_Init(UART1_GPIO, UART1_GPIO_TX, &y); //TXD - PB3
	
	y.GPIO_Direction = GPIO_Dir_In;
	GPIO_Init(UART1_GPIO, UART1_GPIO_RX, &y); //RXD - PB2
#endif

#ifdef configUSING_UART2
	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = UART2_GPIO_AF;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Strong;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
    y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	GPIO_Init(UART2_GPIO, UART2_GPIO_TX, &y); 
	
	y.GPIO_Direction = GPIO_Dir_In;
	GPIO_Init(UART2_GPIO, UART2_GPIO_RX, &y); 
#endif

#ifdef configUSING_UART3
	y.GPIO_Signal = GPIO_Pin_Signal_Digital;
	y.GPIO_Func = UART3_GPIO_AF;
	y.GPIO_Direction = GPIO_Dir_Out;
	y.GPIO_OD = GPIO_ODE_Output_Disable;
	y.GPIO_DS = GPIO_DS_Output_Normal;
	y.GPIO_PUEN = GPIO_PUE_Input_Disable;
    y.GPIO_PDEN = GPIO_PDE_Input_Disable;
	GPIO_Init(UART3_GPIO, UART3_GPIO_TX, &y); 
	
	y.GPIO_Direction = GPIO_Dir_In;
	GPIO_Init(UART3_GPIO, UART3_GPIO_RX, &y); 
#endif
}
static void NVIC_Configuration(struct es8p508_uart *uart)
{
    NVIC_Init(uart->irq, NVIC_Priority_1, ENABLE);
}
static err_t es8p508_configure(struct serial_device *serial, struct serial_configure *cfg)
{
    UART_InitStruType USART_InitStructure;
    struct es8p508_uart *uart = (struct es8p508_uart *)serial->parent.user_data;

    UART_TYPE_DATAMOD datamode;
    switch (cfg->data_bits)
    {
    case DATA_BITS_8:
    default:
        switch (cfg->parity)
        {
        case PARITY_ODD :
            datamode = UART_DataMode_8Odd;
            break;
        case PARITY_EVEN:
            datamode = UART_DataMode_8Even;
            break;
        case PARITY_NONE:
        default:
            datamode = UART_DataMode_8;
            break;
        }
        break;
    }
    USART_InitStructure.UART_TxMode = datamode;
    USART_InitStructure.UART_RxMode = datamode;

    USART_InitStructure.UART_StopBits = UART_StopBits_1;
    USART_InitStructure.UART_TxPolar  = UART_Polar_Normal;
    USART_InitStructure.UART_RxPolar  = UART_Polar_Normal;
    USART_InitStructure.UART_BaudRate = cfg->baud_rate;
    USART_InitStructure.UART_ClockSet = UART_Clock_1;
    UART_Init(uart->uart_device, &USART_InitStructure);

    /* enable interrupt */
    UART_TBIMConfig(uart->uart_device, UART_TRBIM_Byte);
    UART_RBIMConfig(uart->uart_device, UART_TRBIM_Byte);
    UART_ITConfig(uart->uart_device, UART_IT_RB, ENABLE);
	
    uart->uart_device->CON.TXEN = 1;
    uart->uart_device->CON.RXEN = 1;
    return 0;
}

static err_t es8p508_control(struct serial_device *serial, int cmd, void *arg)
{
    struct es8p508_uart *uart = (struct es8p508_uart *)serial->parent.user_data;

    switch (cmd)
    {
    case DEVICE_CTRL_CLR_INT:
        UART_ITConfig(uart->uart_device, UART_IT_RB, DISABLE);
        break;
    case DEVICE_CTRL_SET_INT:
        UART_ITConfig(uart->uart_device, UART_IT_RB, ENABLE);
        break;
    case DEVICE_CTRL_CLR_TX_INT:
        UART_ITConfig(uart->uart_device, UART_IT_TB, DISABLE);
        break;
    case DEVICE_CTRL_SET_TX_INT:
        UART_ITConfig(uart->uart_device, UART_IT_TB, ENABLE);
        break;
    }

    return 0;
}

static int es8p508_putc(struct serial_device *serial, char c)
{
    struct es8p508_uart *uart = (struct es8p508_uart *)serial->parent.user_data;

    while (!(uart->uart_device->IF.Word & UART_FLAG_TB));
    uart->uart_device->TBW.Byte[0] = c;

    return 1;
}

static int es8p508_getc(struct serial_device *serial)
{
    int ch = -1;
    struct es8p508_uart *uart = (struct es8p508_uart *)serial->parent.user_data;

    if (uart->uart_device->IF.Word & UART_FLAG_RB)
    {
        ch = uart->uart_device->RBR.Byte[0] & 0xff;
    }

    return ch;
}
static const struct uart_ops es8p508_uart_ops =
{
    es8p508_configure,
    es8p508_control,
    es8p508_putc,
    es8p508_getc,
};

static void uart_interrupt_handler(struct serial_device *serial)
{
    UART_TypeDef *uart_dev = ((struct es8p508_uart *)serial->parent.user_data)->uart_device;

    if (UART_GetFlagStatus(uart_dev, UART_FLAG_TB))
    {
        serial_device_isr(serial, SERIAL_EVENT_TX_RDY);
    }

    if ((UART_GetFlagStatus(uart_dev, UART_FLAG_RB)) && (UART_GetITStatus(uart_dev, UART_IT_RB)))
    {
        serial_device_isr(serial, SERIAL_EVENT_RX_IND);
    }
}

int board_usart_init(void)
{
    struct es8p508_uart *uart;
    struct serial_configure config = SERIAL_CONFIG_DEFAULT;

    GPIO_Configuration();

#ifdef configUSING_UART0
    uart = &uart0;
    config.bufsz   = 0x100;
    serial0.ops    = &es8p508_uart_ops;
    serial0.config = config;

    NVIC_Configuration(&uart0);
    serial_device_register(&serial0, "uart0", 0, uart);
#endif

#ifdef configUSING_UART1
    uart = &uart1;

    config.bufsz   = 0x100;
    serial1.ops    = &es8p508_uart_ops;
    serial1.config = config;

    NVIC_Configuration(&uart1);
    serial_device_register(&serial1, "uart1", 0, uart);
#endif

#ifdef configUSING_UART2
    uart = &uart2;

    config.bufsz   = 0x100;
    serial2.ops    = &es8p508_uart_ops;
    serial2.config = config;

    NVIC_Configuration(&uart2);
    serial_device_register(&serial2, "uart2", 0, uart);
#endif

#ifdef configUSING_UART3
    uart = &uart3;

    config.bufsz   = 0x100;
    serial3.ops    = &es8p508_uart_ops;
    serial3.config = config;

    NVIC_Configuration(&uart3);
    serial_device_register(&serial3, "uart3", 0, uart);
#endif

    return 0;
}
device_initcall(board_usart_init);

#endif

