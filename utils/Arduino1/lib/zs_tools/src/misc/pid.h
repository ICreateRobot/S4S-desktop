#ifndef __PID_H__
#define __PID_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"

#if (ZST_USE_PID == 1)
typedef struct
{
	float P;
	float I;
	float D;
	int32_t maxVal; //绝对值最值
	ZST_PID_CALC_TYPE last_diff;
	ZST_PID_CALC_TYPE i_val;
}zst_pid_t;

zst_pid_t *pid_init(zst_pid_t *p_pid, uint32_t maxVal, float P, float I, float D);
ZST_PID_CALC_TYPE pid_calculation(zst_pid_t *p_pid, ZST_PID_CALC_TYPE nowVal, ZST_PID_CALC_TYPE wantVal);


#endif /* ZST_USE_PID */

#ifdef __cplusplus
}
#endif
#endif /* __PID_H__ */

