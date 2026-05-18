#include "../inc/cc_array_stack.h"
#include <stdlib.h>

cc_stack_i_t cc_array_stack_interface = {
	.push = (cc_stack_push_fn_t)cc_array_stack_push,
	.pop = (cc_stack_pop_fn_t)cc_array_stack_pop,
	.peek = (cc_stack_peek_fn_t)cc_array_stack_peek,
};

int cc_array_stack_push(cc_array_stack_t *self, void *item)
{
	if (cc_array_set(self->data, self->top, item) == CC_ARRAY_OUT_OF_RANGE)
		return CC_STACK_FULL;

	self->top++;
	return 0;
}

int cc_array_stack_pop(cc_array_stack_t *self, void *item)
{
	if (self->top == 0)
		return CC_STACK_EMPTY;

	self->top--;
	cc_array_get_unsafe(self->data, self->top, item);
	return 0;
}

int cc_array_stack_peek(cc_array_stack_t *self, void *item)
{
	if (self->top == 0)
		return CC_STACK_EMPTY;

	cc_array_get_unsafe(self->data, self->top - 1, item);
	return 0;
}

size_t cc_array_stack_elem_nums(cc_array_stack_t *self)
{
	return self->top;
}

size_t cc_array_stack_space(cc_array_stack_t *self)
{
	return self->data->elem_nums - self->top;
}

int cc_array_stack_init(cc_array_stack_t *self, cc_array_t *data)
{
	self->interface = &cc_array_stack_interface;
	self->data = data;
	self->top = 0;
	return 0;
}

int cc_array_stack_new(cc_array_stack_t **self, size_t elem_nums,
		size_t elem_size)
{
	cc_array_stack_t *tmp;
	cc_array_t *data;

	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;
	if (cc_array_new(&data, elem_nums, elem_size))
		goto fail2;
	if (cc_array_stack_init(tmp, data))
		goto fail3;

	*self = tmp;
	return 0;

fail3:
	cc_array_delete(data);
fail2:
	cc_free(tmp);
fail1:
	return 1;
}

int cc_array_stack_delete(cc_array_stack_t *self)
{
	if (cc_array_delete(self->data))
		return 1;

	cc_free(self);
	return 0;
}
