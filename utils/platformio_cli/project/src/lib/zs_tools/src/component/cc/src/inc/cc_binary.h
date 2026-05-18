#ifndef __CC_BINARY_H
#define __CC_BINARY_H

#include "cc_common.h"
#include "cc_iter.h"
#include <stddef.h>
#include "cc_list.h"

typedef struct cc_binary 
{
	struct cc_binary *parent;
	struct cc_binary *left;
	struct cc_binary *right;
	void *data;
} cc_binary_t;

int cc_binary_new(cc_binary_t **self, cc_binary_t *parent,
		void *data);

int cc_binary_delete(cc_binary_t *self);

int cc_binary_insert_left(cc_binary_t *self, void *data);
int cc_binary_insert_right(cc_binary_t *self, void *data);

int cc_binary_rotate_left(cc_binary_t **start_slot);
int cc_binary_rotate_right(cc_binary_t **start_slot);

int cc_binary_print(cc_binary_t *root, int depth,
		cc_simple_fn_1_t print_fn);

typedef struct cc_binary_iter 
{
	cc_iter_i_t *iterator;
	cc_binary_t *tree;
	cc_list_t *queue;
	size_t index;
	enum cc_traverse_direction direction;
} cc_binary_iter_t;

/*
 * This iterator do not provide `_init` method since it need a `_delete` to
 * cc_free some resources.
 */
int cc_binary_iter_new(cc_binary_iter_t **self, cc_binary_t *root,
		enum cc_traverse_direction direction);

int cc_binary_iter_delete(cc_binary_iter_t *self);

int cc_binary_iter_next(cc_binary_iter_t *self, void **item,
		size_t *index);

#endif
