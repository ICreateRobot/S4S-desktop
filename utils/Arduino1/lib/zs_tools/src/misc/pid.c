#include "./pid.h"

#if (ZST_USE_PID == 1)

#include "./zst_mem.h"

zst_pid_t *pid_init(zst_pid_t *p_pid, uint32_t maxVal, float P, float I, float D)
{
    zst_memset_00(p_pid, sizeof(zst_pid_t)); // 内存清空

    p_pid->P = P;
    p_pid->I = I;
    p_pid->D = D;
    p_pid->maxVal = maxVal;

    return p_pid;
}

/*
    功能：输出pid最终值

    wantval：欲到达值
    nowval：目前值
*/
ZST_PID_CALC_TYPE pid_calculation(zst_pid_t *p_pid, ZST_PID_CALC_TYPE nowVal, ZST_PID_CALC_TYPE wantVal)
{
    ZST_PID_CALC_TYPE retval = 0;
    ZST_PID_CALC_TYPE diffval = wantVal - nowVal; // 差值
    ZST_PID_CALC_TYPE d_val = 0;

    retval = diffval * p_pid->P; // p值
    retval = retval > p_pid->maxVal ? p_pid->maxVal : 
                retval < -p_pid->maxVal ? -p_pid->maxVal : 
                retval;

    p_pid->i_val += diffval; // 积分
    p_pid->i_val = p_pid->i_val > p_pid->maxVal ? p_pid->maxVal : 
                    p_pid->i_val < -p_pid->maxVal ? -p_pid->maxVal : 
                    p_pid->i_val;

    d_val = diffval - p_pid->last_diff;
    p_pid->last_diff = diffval;
    d_val = d_val > p_pid->maxVal ? p_pid->maxVal :
                d_val < -p_pid->maxVal ? -p_pid->maxVal : 
                d_val;

    retval = retval + p_pid->i_val * p_pid->I + d_val * p_pid->D;
    retval = retval > p_pid->maxVal ? p_pid->maxVal : 
                retval < -p_pid->maxVal ? -p_pid->maxVal : 
                retval;

    return nowVal + retval;
}



#endif /* ZST_USE_PID */
