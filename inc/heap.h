#ifndef _HEAP_H_
#define _HEAP_H_

#include <types.h>

#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE   (1024u*2)
#endif

#define malloc os_malloc
#define free   os_free

void *os_malloc(size_t size);
void os_free(void *pv);

#endif
