#include "../inc/cc_ring.h"
#include <stdlib.h>

cc_queue_i_t cc_ring_interface = {
	.enqueue = (cc_queue_enqueue_fn_t)cc_ring_enqueue,
	.dequeue = (cc_queue_dequeue_fn_t)cc_ring_dequeue,
	.peek = (cc_queue_peek_fn_t)cc_ring_peek,
};

static inline size_t next_index(cc_ring_t *self, size_t index)
{
	size_t next_index = index + 1;
	if (cc_array_is_valid_index(self->data, next_index))
		return next_index;
	else
		return 0;
}

size_t cc_ring_elem_nums(cc_ring_t *self)
{
	if (self->write_index >= self->read_index)
		return self->write_index - self->read_index;
	else
		return self->data->elem_nums + self->write_index
				- self->read_index;
}

size_t cc_ring_space(cc_ring_t *self)
{
	return self->data->elem_nums - cc_ring_elem_nums(self) - 1;
}

int cc_ring_enqueue(cc_ring_t *self, void *item)
{
	size_t write_index_next;

	write_index_next = next_index(self, self->write_index);
	if (write_index_next == self->read_index)
		return CC_QUEUE_FULL;

	cc_array_set_unsafe(self->data, self->write_index, item);
	self->write_index = write_index_next;
	return 0;
}

int cc_ring_peek(cc_ring_t *self, void *item)
{
	if (self->read_index == self->write_index)
		return CC_QUEUE_EMPTY;

	cc_array_get_unsafe(self->data, self->read_index, item);
	return 0;
}

int cc_ring_dequeue(cc_ring_t *self, void *item)
{
	if (cc_ring_peek(self, item) == CC_QUEUE_EMPTY)
		return CC_QUEUE_EMPTY;

	self->read_index = next_index(self, self->read_index);
	return 0;
}

int cc_ring_init(cc_ring_t *self, cc_array_t *data)
{
	self->interface = &cc_ring_interface;
	self->data = data;
	self->read_index = 0;
	self->write_index = 0;
	return 0;
}

int cc_ring_new(cc_ring_t **self, size_t elem_nums, size_t elem_size)
{
	cc_ring_t *tmp;
	cc_array_t *data;

	tmp = cc_alloc(sizeof(*tmp));
	if (tmp == NULL)
		goto fail1;

	/*
	 * There will be 1 element got wasted,
	 * so pass `elem_nums + 1` to `cc_array_new`.
	 */
	if (cc_array_new(&data, elem_nums + 1, elem_size))
		goto fail2;
	if (cc_ring_init(tmp, data))
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

int cc_ring_delete(cc_ring_t *self)
{
	if (cc_array_delete(self->data))
		return 1;

	cc_free(self);
	return 0;
}
