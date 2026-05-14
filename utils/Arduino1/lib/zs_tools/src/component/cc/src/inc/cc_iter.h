#ifndef __CC_ITER_H
#define __CC_ITER_H

#include <stddef.h>

typedef int (*cc_iter_next_fn_t)(void *self, void **item, size_t *index);

typedef struct cc_iter_i 
{
	cc_iter_next_fn_t next;
} cc_iter_i_t;

static inline int cc_iter_next(void *self, void *item, size_t *index)
{
	return (*(cc_iter_i_t **)self)->next(self, (void **)item, index);
}

#endif
