#ifndef __CC_ARRAY_H
#define __CC_ARRAY_H

#include "cc_common.h"
#include "cc_iter.h"
#include <stddef.h>

enum 
{
	CC_ARRAY_OUT_OF_RANGE = 0xFFD0,
};

/******************************************************************************
	Generic array for data of different length
 *****************************************************************************/
typedef struct cc_array 
{
	unsigned char *data;
	size_t elem_nums;
	size_t elem_size;
} cc_array_t;

int cc_array_new(cc_array_t **self, size_t elem_nums, size_t elem_size);
int cc_array_delete(cc_array_t *self);
int cc_array_delete_keep_data(cc_array_t *self);

int cc_array_init(cc_array_t *self, unsigned char *data, size_t elem_nums,
		size_t elem_size);

/* These `_unsafe` functions do not check the boundaries of arrays. */

void cc_array_get_unsafe(cc_array_t *self, size_t index, void *result);
void cc_array_get_ref_unsafe(cc_array_t *self, size_t index, void **ref);
void cc_array_set_unsafe(cc_array_t *self, size_t index, void *value);

int cc_array_get(cc_array_t *self, size_t index, void *result);
int cc_array_get_ref(cc_array_t *self, size_t index, void **ref);
int cc_array_set(cc_array_t *self, size_t index, void *value);

/* Check whether the index has exceeded the boundary of the array. */
int cc_array_is_valid_index(cc_array_t *self, size_t index);
int cc_array_cmp(cc_array_t *self, cc_cmp_fn_t cmp, size_t i, size_t j);
int cc_array_reverse(cc_array_t *self, size_t start, size_t end);
void cc_array_swap(cc_array_t *self, size_t i, size_t j);

/******************************************************************************
	Iterator for generic array
 *****************************************************************************/
typedef struct cc_array_iter
{
	cc_iter_i_t *iterator;
	cc_array_t *data;
	size_t cursor;
} cc_array_iter_t;

int cc_array_iter_init(cc_array_iter_t*self, cc_array_t *data);
int cc_array_iter_next(cc_array_iter_t*self, void **item, size_t *index);

#endif
