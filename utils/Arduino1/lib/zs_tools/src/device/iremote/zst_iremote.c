#include "zst_iremote.h"
#if ZST_DEVICE_IRMOTE==1

/**
 * @description: 初始化红外遥控对象
 * @param {iremote_t *} iremote 红外遥控对象
 * @param {float} tick_us 微秒计数器跳动一次需要多少时间
 * @param {uint16_t} overflow_count 计数器溢出需要多少次
 * @return {*}
 */
void iremote_init(iremote_t * iremote, float tick_us, uint16_t overflow_count)
{
    iremote->ir_560us.min = 300 / tick_us;
    iremote->ir_560us.max = 800 / tick_us;

    iremote->ir_1680us.min = 1400 / tick_us;
    iremote->ir_1680us.max = 1800 / tick_us;

    iremote->ir_2250us.min = 2000 / tick_us;
    iremote->ir_2250us.max = 3000 / tick_us;

    iremote->ir_4500us.min = 4200 / tick_us;
    iremote->ir_4500us.max = 4700 / tick_us;

    iremote->overflow_count = overflow_count;

    iremote->init_finsh = 1;
}

void iremote_timOverFlow_callback(iremote_t * iremote)
{
    if (0 == iremote->init_finsh) return;
    if (++iremote->g_remote.state.overflowCounter > 5)
    {
        iremote->g_remote.state.get_guidanceCode = 0;
        iremote->g_remote.count = 0;
    }
}

void iremote_gpio_interrupt_callback(iremote_t *iremote)
{
    if (0 == iremote->init_finsh) return;
    uint16_t dval; /* 下降沿时计数器的值 */

    if (iremote->get_gpio_pin_level(iremote)) /* 上升沿捕获 */
    {
        if (iremote->set_falling) iremote->set_falling(iremote); /* 下降沿捕获 */
        iremote->last_val = iremote->get_countVal(iremote);
        iremote->g_remote.state.overflowCounter = 0; /* 清空计时器 */
        iremote->g_remote.state.get_risingEdge = 1;  /* 标记上升沿已经被捕获 */
    }
    else /* 下降沿捕获 */
    {
        dval = iremote->overflow_count * iremote->g_remote.state.overflowCounter + iremote->get_countVal(iremote) - iremote->last_val;
        if (iremote->set_rising) iremote->set_rising(iremote);

        if (1 == iremote->g_remote.state.get_risingEdge) /* 完成一次高电平捕获 */
        {
            if (1 == iremote->g_remote.state.get_guidanceCode) /* 接收到了引导码 */
            {
                if (dval > iremote->ir_560us.min && dval < iremote->ir_560us.max) /* 560为标准值,560us */
                {
                    iremote->g_remote.data >>= 1;            /* 右移一位 */
                    iremote->g_remote.data &= ~(0x80000000); /* 接收到0 */
                }
                else if (dval > iremote->ir_1680us.min && dval < iremote->ir_1680us.max) /* 1680为标准值,1680us */
                {
                    iremote->g_remote.data >>= 1;         /* 右移一位 */
                    iremote->g_remote.data |= 0x80000000; /* 接收到1 */
                }
                else if (dval > iremote->ir_2250us.min && dval < iremote->ir_2250us.max) /* 得到按键键值增加的信息 2250为标准值2.25ms */
                {
                    iremote->g_remote.state.get_keyAllInfo = 1;
                    iremote->g_remote.count++;                   /* 按键次数增加1次 */
                    iremote->g_remote.state.overflowCounter = 0; /* 清空计时器 */
                }
            }
            else if (dval > iremote->ir_4500us.min && dval < iremote->ir_4500us.max) /* 4500为标准值4.5ms */
            {
                iremote->g_remote.state.get_guidanceCode = 1; /* 标记成功接收到了引导码 */
                iremote->g_remote.count = 0;                  /* 清除按键次数计数器 */
            }
        }
        iremote->g_remote.state.get_risingEdge = 0;
    }
}

/**
 * @brief       处理红外按键(类似按键扫描)
 * @param       无
 * @retval      0   , 没有任何按键按下
 *              其他, 按下的按键键值
 */
uint8_t remote_scan(iremote_t * iremote)
{
    uint8_t sta = 0;
    uint8_t t1, t2;

    if (1 == iremote->g_remote.state.get_keyAllInfo) /* 得到一个按键的所有信息了 */
    {
        iremote->g_remote.state.get_keyAllInfo = 0;
        t1 = iremote->g_remote.data;               /* 得到地址码 */
        t2 = (iremote->g_remote.data >> 8) & 0xff; /* 得到地址反码 */

        if ((t1 == (uint8_t)~t2) && t1 == iremote->id) /* 检验遥控识别码(ID)及地址 */
        {
            t1 = (iremote->g_remote.data >> 16) & 0xff;
            t2 = (iremote->g_remote.data >> 24) & 0xff;

            if (t1 == (uint8_t)~t2)
            {
                sta = t1; /* 键值正确 */
            }
        }
        if ((sta == 0) || (iremote->g_remote.state.get_guidanceCode == 0)) /* 按键数据错误/遥控已经没有按下了 */
        {
            iremote->g_remote.state.get_keyAllInfo = 0; /* 清除接收到有效按键标识 */
            iremote->g_remote.count = 0;                /* 清除按键次数计数器 */
        }
    }
    return sta;
}

uint8_t remote_get_key_count(iremote_t * iremote)
{
    return iremote->g_remote.count;
}

#endif


