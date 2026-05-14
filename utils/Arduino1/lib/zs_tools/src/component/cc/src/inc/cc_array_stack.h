#ifndef __CC_ARRAY_STACK_H
#define __CC_ARRAY_STACK_H

#include "cc_array.h"
#include "cc_common.h"
#include "cc_stack.h"
#include <stddef.h>

typedef struct cc_array_stack 
{
	cc_stack_i_t *interface;
	cc_array_t *data;
	size_t top;
} cc_array_stack_t;

int cc_array_stack_new(cc_array_stack_t **self, size_t elem_nums,
		size_t elem_size);

int cc_array_stack_delete(cc_array_stack_t *self);

int cc_array_stack_init(cc_array_stack_t *self, cc_array_t *data);

int cc_array_stack_push(cc_array_stack_t *self, void *item);
int cc_array_stack_pop(cc_array_stack_t *self, void *item);
int cc_array_stack_peek(cc_array_stack_t *self, void *item);

size_t cc_array_stack_elem_nums(cc_array_stack_t *self);
size_t cc_array_stack_space(cc_array_stack_t *self);

#endif
