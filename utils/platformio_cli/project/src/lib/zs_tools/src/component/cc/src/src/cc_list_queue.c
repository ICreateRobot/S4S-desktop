#include "../inc/cc_list_queue.h"
#include <stdlib.h>

cc_queue_i_t cc_list_queue_interface = {
	.enqueue = (cc_queue_enqueue_fn_t)cc_list_queue_enqueue,
	.dequeue = (cc_queue_dequeue_fn_t)cc_list_queue_dequeue,
	.peek = (cc_queue_peek_fn_t)cc_list_queue_peek,
};

static inline int translate_code(int code)
{
	return (code == CC_LIST_EMPTY) ? CC_QUEUE_EMPTY : code;
}

int cc_list_queue_enqueue(cc_list_queue_t *self, void *data)
{
	return cc_list_insert_tail(self->list, data);
}

int cc_list_queue_dequeue(cc_list_queue_t *self, void **result)
{
	return translate_code(cc_list_remove_head(self->list, result));
}

int cc_list_queue_peek(cc_list_queue_t *self, void **result)
{
	return translate_code(cc_list_get_head(self->list, result));
}

int cc_list_queue_new(cc_list_queue_t **self)
{
	cc_list_queue_t *tmp;
	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;

	tmp->interface = &cc_list_queue_interface;
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

int cc_list_queue_delete(cc_list_queue_t *self)
{
	if (cc_list_delete(self->list))
		return 1;

	cc_free(self);
	return 0;
}
