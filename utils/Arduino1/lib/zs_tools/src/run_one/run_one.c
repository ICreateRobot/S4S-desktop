#include "run_one.h"

#if ZST_RUN_ONE == 1
#include "string.h"
#include "../misc/zst_mem.h"
static uint64_t fnv1a_64(const void *data, size_t len);

int run_one_init(run_one_init_t * run_one_init)
{
    if (NULL == run_one_init || NULL == run_one_init->run_one_handle)
    {
        return -1;
    }
    zst_memset_00(run_one_init->run_one_handle, sizeof(run_one_t));
    run_one_init->run_one_handle->data = run_one_init->data;
    run_one_init->run_one_handle->size = run_one_init->size;
    run_one_init->run_one_handle->compare_data = run_one_init->compare_data;
    return 0;
}

/**
 * 返回0触发一次，否侧不触发
 */
int run_one_data_changed(run_one_t * run_one_handle)
{
    if (NULL == run_one_handle)
        return -1;

    if (NULL != run_one_handle->compare_data)
    {
        int is_equal = 1;
        if (0 != memcmp(run_one_handle->data, run_one_handle->compare_data, run_one_handle->size))
        {
            memcpy(run_one_handle->compare_data, run_one_handle->data, run_one_handle->size);
            return -1;
        }
        return 0;
    }

    uint64_t hash = fnv1a_64((uint8_t *)run_one_handle->data, run_one_handle->size);
    if (hash != run_one_handle->hash)
    {
        run_one_handle->hash = hash;
        return -1;
    }
    return 0;
}


int run_one_data_equals(run_one_t * run_one_handle, void * data)
{
    if (NULL == run_one_handle || NULL == data)
        return -1;

    if (NULL != run_one_handle->compare_data)
    {
        if (0 != memcmp(run_one_handle->data, data, run_one_handle->size))
        {
            memcpy(run_one_handle->compare_data, run_one_handle->data, run_one_handle->size);
            return -1;
        }
        return 0;
    }

    uint64_t hash = fnv1a_64(data, run_one_handle->size);
    if (run_one_handle->hash == hash)
    {
        if (0 == run_one_handle->equals_flag)
        {
            run_one_handle->equals_flag = 1;
            return 0;
        }
    }
    else
    {
        run_one_handle->equals_flag = 0;
    }
    return 1;
}





static uint64_t fnv1a_64(const void *data, size_t len)
{
    if (0 == len)
    {
        return (uintptr_t)data;
    }
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = 0xcbf29ce484222325; // FNV offset basis for 64-bit

    for (size_t i = 0; i < len; i++)
    {
        hash ^= bytes[i];
        hash *= 0x100000001b3; // FNV prime for 64-bit
    }
    return hash;
}


#endif
