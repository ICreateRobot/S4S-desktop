#ifndef __CC_RING_H
#define __CC_RING_H

#include "cc_array.h"
#include "cc_common.h"
#include "cc_queue.h"
#include <stddef.h>

typedef struct cc_ring 
{
	cc_queue_i_t *interface;
	cc_array_t *data;
	size_t read_index;
	size_t write_index;
} cc_ring_t;

int cc_ring_new(cc_ring_t **self, size_t elem_nums, size_t elem_size);
int cc_ring_delete(cc_ring_t *self);

int cc_ring_init(cc_ring_t *self, cc_array_t *data);

int cc_ring_enqueue(cc_ring_t *self, void *item);
int cc_ring_dequeue(cc_ring_t *self, void *item);
int cc_ring_peek(cc_ring_t *self, void *item);

size_t cc_ring_elem_nums(cc_ring_t *self);
size_t cc_ring_space(cc_ring_t *self);

#endif
