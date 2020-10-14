/*********************************************************************
*              SEGGER MICROCONTROLLER SYSTEME GmbH                   *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996-2014 SEGGER Microcontroller Systeme GmbH           *
*                                                                    *
* Internet: www.segger.com Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT.h
Purpose : Implementation of SEGGER real-time terminal which allows
          real-time terminal communication on targets which support
          debugger memory accesses while the CPU is running.
---------------------------END-OF-HEADER------------------------------
*/
#include <stddef.h>
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SEGGER_RTT_MODE_MASK                  (3 << 0)

#define SEGGER_RTT_MODE_NO_BLOCK_SKIP         (0)
#define SEGGER_RTT_MODE_NO_BLOCK_TRIM         (1 << 0)
#define SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL    (1 << 1)

/*********************************************************************
*
*       RTT API functions
*
**********************************************************************
*/

int RTT_read(void *out, size_t maxlen);
int RTT_write(const char *in, size_t len);
int RTT_write_string(const char *str);

int RTT_get_key(void);
int RTT_wait_key(void);
int RTT_has_key(void);

void RTT_init(void);


/*************************** End of file ****************************/
