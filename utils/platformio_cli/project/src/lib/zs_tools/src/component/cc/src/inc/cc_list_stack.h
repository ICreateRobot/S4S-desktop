#ifndef __CC_LIST_STACK_H
#define __CC_LIST_STACK_H

#include "cc_list.h"
#include "cc_stack.h"

typedef struct cc_list_stack 
{
	cc_stack_i_t *interface;
	cc_list_t *list;
} cc_list_stack_t;

int cc_list_stack_new(cc_list_stack_t **self);
int cc_list_stack_delete(cc_list_stack_t *self);

int cc_list_stack_push(cc_list_stack_t *self, void *data);
int cc_list_stack_pop(cc_list_stack_t *self, void **result);
int cc_list_stack_peek(cc_list_stack_t *self, void **result);

#endif
