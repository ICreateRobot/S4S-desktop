#ifndef __CC_LIST_H
#define __CC_LIST_H

#include "cc_array.h"
#include "cc_common.h"
#include "cc_iter.h"

/******************************************************************************
	The List Node
 *****************************************************************************/
typedef struct cc_list_node 
{
	struct cc_list_node *prev;
	struct cc_list_node *next;
	/*
	 * For data node, we use `data` to hold a value or a pointer to value.
	 * For root node, we use `size` to keep the number of elements.
	 */
	union 
	{
		void *data;
		size_t size;
	};
} cc_list_node_t;

int cc_list_node_insert_before(cc_list_node_t *self, void *data);
int cc_list_node_insert_after(cc_list_node_t *self, void *data);
int cc_list_node_remove_before(cc_list_node_t *self, void **result);
int cc_list_node_remove_after(cc_list_node_t *self, void **result);

int cc_list_node_delete_and_next(cc_list_node_t **current,
		cc_delete_fn_t remove_fn);

/******************************************************************************
	The List Container
 *****************************************************************************/
enum cc_list_error 
{
	CC_LIST_EMPTY = 0xFE10,
};

typedef struct cc_list 
{
	cc_list_node_t root;
} cc_list_t;

int cc_list_new(cc_list_t **self);
int cc_list_delete(cc_list_t *self);

int cc_list_concat(cc_list_t *left, cc_list_t *right);

int cc_list_insert_head(cc_list_t *self, void *data);
int cc_list_insert_tail(cc_list_t *self, void *data);
int cc_list_remove_head(cc_list_t *self, void **data);
int cc_list_remove_tail(cc_list_t *self, void **data);
int cc_list_get_head(cc_list_t *self, void **data);
int cc_list_get_tail(cc_list_t *self, void **data);

int cc_list_is_empty(cc_list_t *self);
size_t cc_list_size(cc_list_t *self);

int cc_list_to_cc_array(cc_list_t *self, cc_array_t **result);

/* For debugging */
int cc_list_print(cc_list_t *self, int direction);

/******************************************************************************
	The List Cursor
 *****************************************************************************/
enum cc_list_cursor_error 
{
	CC_LIST_CURSOR_MOVE_OUT_OF_RANGE = 0xFF10,
	CC_LIST_CURSOR_GET_OUT_OF_RANGE = 0xFF11,
	CC_LIST_CURSOR_REMOVING_CURRENT = 0xFF19,
	CC_LIST_CURSOR_AT_END = 0xFF90,
};

typedef struct cc_list_cursor 
{
	cc_list_t *list;
	cc_list_node_t *current;
	cc_delete_fn_t remove_fn;
} cc_list_cursor_t;

int cc_list_cursor_init(cc_list_cursor_t *tmp, cc_list_t *list,
		cc_delete_fn_t remove_fn);

int cc_list_cursor_new(cc_list_cursor_t **self, cc_list_t *list,
		cc_delete_fn_t remove_fn);

int cc_list_cursor_delete(cc_list_cursor_t *self);

int cc_list_cursor_relative_pos(cc_list_cursor_t *self, int offset,
		cc_list_node_t **result);

int cc_list_cursor_get(cc_list_cursor_t *self, int offset, int count,
		void **result);

int cc_list_cursor_move(cc_list_cursor_t *self, int offset);

int cc_list_cursor_insert_before(cc_list_cursor_t *self, int offset,
		void *data);

int cc_list_cursor_remove(cc_list_cursor_t *self, int offset, int count);

int cc_list_cursor_at_end(cc_list_cursor_t *self);
void cc_list_cursor_reset(cc_list_cursor_t *self);

/******************************************************************************
	The List Iterator
 *****************************************************************************/
typedef struct cc_list_iter 
{
	cc_iter_i_t *iterator;
	cc_list_t *list;
	cc_list_node_t *cursor;
	size_t index;
	int direction;
} cc_list_iter_t;

int cc_list_iter_init(cc_list_iter_t *self, cc_list_t *list,
		int direction);

int cc_list_iter_next(cc_list_iter_t *self, void **item, size_t *index);

#endif
