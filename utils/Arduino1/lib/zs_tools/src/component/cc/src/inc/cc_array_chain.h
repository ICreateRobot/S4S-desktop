#ifndef __CC_ARRAY_CHAIN_H
#define __CC_ARRAY_CHAIN_H

#include "cc_array.h"
#include "cc_list.h"

typedef struct cc_array_chain
{
	cc_list_t *node_chain;
	cc_array_t *cursor;
	size_t cursor_index;
	size_t total_nums;
	size_t node_elem_nums;
	size_t node_elem_size;
} cc_array_chain_t;

int cc_array_chain_new(cc_array_chain_t **self, int node_elem_nums,
		int node_elem_size);

int cc_array_chain_delete(cc_array_chain_t *self);

int cc_array_chain_add_elem(cc_array_chain_t *self, void *elem);
int cc_array_chain_append(cc_array_chain_t *self, void *data, size_t nums);

int cc_array_chain_to_array(cc_array_chain_t *self,
		cc_array_t **result, int nums_to_reserve);

#endif
