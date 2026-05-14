#include "../inc/cc_list_stack.h"
#include <stdint.h>
#include <stdlib.h>

cc_stack_i_t cc_list_stack_interface = {
	.push = (cc_stack_push_fn_t)cc_list_stack_push,
	.pop = (cc_stack_pop_fn_t)cc_list_stack_pop,
	.peek = (cc_stack_peek_fn_t)cc_list_stack_peek,
};

static inline int translate_code(int code)
{
	return (code == CC_LIST_EMPTY) ? CC_STACK_EMPTY : code;
}

int cc_list_stack_push(cc_list_stack_t *self, void *data)
{
	return cc_list_insert_head(self->list, data);
}

int cc_list_stack_pop(cc_list_stack_t *self, void **result)
{
	return translate_code(cc_list_remove_head(self->list, result));
}

int cc_list_stack_peek(cc_list_stack_t *self, void **result)
{
	return translate_code(cc_list_get_head(self->list, result));
}

int cc_list_stack_new(cc_list_stack_t **self)
{
	cc_list_stack_t *tmp;
	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;

	tmp->interface = &cc_list_stack_interface;
	if (cc_list_new(&tmp->list))
		goto fail2;

	*self = tmp;
	return 0;
//fail3:
//	cc_list_delete(tmp->list);
fail2:
	cc_free(tmp);
fail1:
	return 1;
}

int cc_list_stack_delete(cc_list_stack_t *self)
{
	if (cc_list_delete(self->list))
		return 1;

	cc_free(self);
	return 0;
}
