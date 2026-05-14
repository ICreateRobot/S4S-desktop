#include "zst_core.h"

#if (ZST_USE_ALLOC==1 && ZST_MEM_CUSTOM==0)
    #include "./misc/tlsf/mm_tlsf.h"
    #define TLSF_MEM_SIZE (1024 * ZST_MEM_SIZE)
    static uint32_t tlsf_mem[TLSF_MEM_SIZE/4];
#endif

#if (ZST_USE_TIMER == 1)
    ztimer_t zst_ztimer;
#endif

#if (ZST_USE_PTASK == 1)
    ptask_list_t zst_ptask_list;
#endif

void zst_init(void)
{
    #if (ZST_USE_ALLOC==1 && ZST_MEM_CUSTOM==0)
        mm_init(tlsf_mem, TLSF_MEM_SIZE);
    #endif
    #if (ZST_USE_TIMER == 1)
        zst_timer_core_init(&zst_ztimer);
    #endif
    #if (ZST_USE_PTASK == 1)
        ptask_init(&zst_ptask_list);
    #endif
}



void zst_task_handler(void)
{
    #if (ZST_USE_PTASK == 1)
        ptask_run(&zst_ptask_list);
    #endif
    #if (ZST_USE_TIMER == 1)
        zst_timer_handler(&zst_ztimer);
    #endif
}



