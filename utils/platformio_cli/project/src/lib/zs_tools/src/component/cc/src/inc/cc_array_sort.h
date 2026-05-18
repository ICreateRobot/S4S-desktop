#ifndef __CC_ARRAY_SORT_H
#define __CC_ARRAY_SORT_H

#include "cc_array.h"
#include "cc_common.h"

int cc_array_sort_bubble(cc_array_t *self, cc_cmp_fn_t);
int cc_array_sort_quick(cc_array_t *self, cc_cmp_fn_t);

#endif
