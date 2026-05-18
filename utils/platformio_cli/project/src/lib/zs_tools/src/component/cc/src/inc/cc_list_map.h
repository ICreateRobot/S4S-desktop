#ifndef __CC_LIST_MAP_H
#define __CC_LIST_MAP_H

#include "cc_common.h"
#include "cc_iter.h"
#include "cc_list.h"
#include "cc_map.h"

typedef struct cc_list_map 
{
	cc_map_i_t *interface;
	cc_list_t *data;
	cc_list_cursor_t cursor;
	cc_cmp_fn_t cmp;
} cc_list_map_t;

int cc_list_map_new(cc_list_map_t **self, cc_cmp_fn_t cmp);
int cc_list_map_delete(cc_list_map_t *self);

int cc_list_map_get(cc_list_map_t *self, void *key, void **result);
int cc_list_map_set(cc_list_map_t *self, void *key, void *value,
		void **old_value);

int cc_list_map_set_new(cc_list_map_t *self, void *key, void *value);

int cc_list_map_del(cc_list_map_t *self, void *key,
		cc_map_item_t **result);

int cc_list_map_print(cc_list_map_t *self, void (*callback)(cc_map_item_t *), char *end_string);

typedef struct cc_list_map_iter 
{
	cc_iter_i_t *iterator;
	cc_list_iter_t inner_iter;
} cc_list_map_iter_t;

int cc_list_map_iter_init(cc_list_map_iter_t *self,
		cc_list_map_t *map);

int cc_list_map_iter_next(cc_list_map_iter_t *self, void **item,
		size_t *index);

#endif
