#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "types.h"

void object_attach(struct object *o, const char *name);
void object_detach(struct object *o);
object_t *object_find(const char *name);

#endif
