#include <stdio.h>
#include "zs_tools/zs_tool.h"
#include "unistd.h"


ptask_list_t ptask_list ={0};

ptask_root_t * root0;
ptask_root_t * root1;

ptask_t * task0;
ptask_t * task1;

ptask_t * task2;
ptask_t * task3;

void event_callback(ptask_t *task, ptask_event_t *e)
{
    static uint8_t i = 0;
    if (++i > 10)
    {
        i = 0;
        ptask_root_select(&ptask_list, root1);
        return;
    }

    if (task == task0)
    {
        printf("rt0 task0 hello word : %d\n", ptask_get_code(e));
    }
    else if (task == task1)
    {
        printf("rt0 task1 hello word : %d\n", ptask_get_code(e));
    }
}

void event2_callback(ptask_t *task, ptask_event_t *e)
{
    static uint8_t i = 0;
    if (++i > 10)
    {
        i = 0;
        ptask_root_select(&ptask_list, root0);
        ptask_pause(task2);
        printf("stop task2..\n");
        return;
    }

    if (task == task2)
    {
        printf("rt1 task2 hello word : %d\n", ptask_get_code(e));
    }
    else if (task == task3)
    {
        printf("rt1 task3 hello word : %d\n", ptask_get_code(e));
    }
}

int main()
{
    ptask_init(&ptask_list);
    root0 = ptask_root_create(&ptask_list);
    root1 = ptask_root_create(&ptask_list);
    
    task0 = ptask_create(root0, event_callback, NULL);
    task1 = ptask_create(root0, event_callback, NULL);

    task2 = ptask_create(root1, event2_callback, NULL);
    task3 = ptask_create(root1, event2_callback, NULL);

    ptask_root_select(&ptask_list, root0);

    // ptask_list_send_event(&ptask_list, PTASK_EVENT_EXIT, NULL);
    // ptask_root_send_event(root, PTASK_EVENT_EXIT, NULL);

    while (1)
    {
        ptask_run(&ptask_list);
        usleep(1000 * 500);
    }
    return 0;
}
