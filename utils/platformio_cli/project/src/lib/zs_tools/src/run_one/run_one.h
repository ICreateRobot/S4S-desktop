#ifndef __RUN_ONE_H__
#define __RUN_ONE_H__

#include "zst_conf.h"

#if ZST_RUN_ONE==1

typedef struct 
{
    uint8_t * data;
    uint8_t * compare_data;
    size_t size;
    uint64_t hash;
    uint8_t equals_flag;
} run_one_t;

typedef struct
{
    run_one_t * run_one_handle;
    void * data;
    void * compare_data;
    size_t size;
} run_one_init_t;

int run_one_init(run_one_init_t * run_one_init);

// 同一个run_one,下面函数只能使用一个
int run_one_data_changed(run_one_t * run_one_d);
int run_one_data_equals(run_one_t * run_one_handle, void * data);


#endif /* ZST_RUN_ONE==1 */

#endif /* __RUN_ONE_H__ */
