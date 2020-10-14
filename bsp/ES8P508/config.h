#ifndef _CONFIG_H_
#define _CONFIG_H_

#define configSYS_CLOCK   (48000000U)
#define configHZ           100

//#define configUSE_PLL
//#define configUSE_HOSE
//#define configTOTAL_HEAP_SIZE   (2*1024)

#define configUSING_IFLASH

//#define configUSING_DEBUG
#ifdef configUSING_DEBUG
//# define configUSING_SERIAL_DBG
# define configUSING_RTT_DBG
# define configUSING_SHELL
#endif


#define configUSING_SERIAL
#ifdef configUSING_SERIAL
//# define configUSING_UART0
//# define configUSING_UART1
# define configUSING_UART2
//# define configUSING_UART3
//# define configUSING_VCOM
# define configSERIAL_DATA_READ_TIMEOUT		(2*configHZ)
# define configUSING_FRAME_TIMEOUT_SOFT
# ifdef configUSING_FRAME_TIMEOUT_SOFT
#  define configSERIAL_RX_TO				(configHZ/50)
# endif
#endif

#define configUSING_KEY

/*  set update way (put the program in the plc)  */
#define configUSING_UPWAY

//#define configUSING_SPI
//#define configUSING_SPI0
//#define configUSING_SPI_BITBANG
//#define configUSING_W25Qxx

//#define configUSING_W5500

#define configUSING_I2C
#ifdef configUSING_I2C
	# define USING_I2C1
	# define USING_I2C2
#endif
#define configUSING_SI7020
#define configUSING_LCD

#define configUSING_LOCALTIME
#ifdef configUSING_LOCALTIME	
	#define configUSINGTIME_RTC
//	#define configUSINGTIME_EXIT
#endif

//#define configTEST
//#define configTEST_LED
//#define configTEST_SOFTTIMER
//#define configTEST_W25Qxx
//#define configTEST_W5500
//#define configTEST_KEY


#endif


