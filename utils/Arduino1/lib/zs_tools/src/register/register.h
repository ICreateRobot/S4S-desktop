#ifndef __REGISTER_H__
#define __REGISTER_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"
#if (ZST_USE_REGISTER == 1)
#include "../component/cc/cc_inc.h"
#include "../event/zst_event.h"

typedef cc_hash_map_iter_t reg_data_iter_s_t;
typedef cc_hash_map_iter_t reg_data_iter_d_t;
typedef cc_array_iter_t    reg_data_element_iter_t;

typedef enum
{
    DATA_PACK_TYPE_DOUBLE_ADDR, // 双地址
    DATA_PACK_TYPE_SIGNAL_ADDR, // 单个地址
    DATA_PACK_TYPE_SIGNAL_LAST,
} DATA_PACK_TYPE_T;

typedef enum
{
    _DATA_PACK_ENENT_ = ZST_EVENT_LAST,
    DATA_PACK_ENENT_RECEIVE_FINSH,      // 接收完成
    DATA_PACK_ENENT_TRANSMIT_FINSH,     // 发送完成（主机读取完成）

    /***
     * 数据存在差异 ( 需要订阅 )
     */
    DATA_PACK_ENENT_ELEMENT_DIFF_CHECK,     // 处理之前
    DATA_PACK_ENENT_ELEMENT_DIFF_AFTERMATH, // 处理之后

} DATA_PACK_EVENT_T;

struct _reg_data_pack_;
struct _reg_data_;

typedef struct 
{
    uint8_t receive_finsh_flag  : 1; // 接收完成标志
    uint8_t transmit_finsh_flag : 1; // 发送完成标志
    uint8_t camp_diff_flag      : 1; // 比较差异标志
} reg_data_flag_t;

typedef struct _reg_data_element_
{
    _zst_obj_spec_attr_t      spec_attr;
    struct _reg_data_pack_    * owner;      // 所属的reg_data_pack
    void                      * user_data;
    uint8_t                   * data;
    struct _reg_data_element_ * ent_rev_next;
    uint16_t                  data_size;
    uint8_t                   subscribe  : 1; // 订阅; 订阅后，查找是否与内存数据不同
    reg_data_flag_t           flag;
} reg_data_element_t;

typedef struct _reg_data_pack_
{
    _zst_obj_spec_attr_t      spec_attr;
    struct _reg_data_         * owner;               // 所属的reg_data
    void                      * user_data;
    uint8_t                   * comparison_buffer;   // 用于比较
    struct _reg_data_pack_    * ent_rev_next;
    reg_data_element_t        * ent_rev_elements; 
    cc_array_t                element_array;
    reg_data_flag_t           flag;
    uint8_t                   addr;
} reg_data_pack_t;

typedef struct _reg_data_
{
    _zst_obj_spec_attr_t      spec_attr;
    cc_hash_map_t             * data_pack;
    reg_data_pack_t           * ent_rev_packs;
    const DATA_PACK_TYPE_T    data_pack_type;
    const uint8_t             mini_flag : 1; // mini
    reg_data_flag_t           flag;
} reg_data_t;

typedef struct 
{
    reg_data_pack_t     * reg_data_pack;
    reg_data_element_t  * element_array;
    uint8_t             * comparison_buffer;// 用于比较
    size_t              element_array_size;
    uint16_t            addr;
} reg_data_pack_init_t;

int reg_data_init(reg_data_t * reg_data, uint16_t hash_capacity);
int reg_data_pack_init(reg_data_t * reg_data, reg_data_pack_init_t * reg_data_pack_init);

int reg_data_get_type(const reg_data_t * reg_data, DATA_PACK_TYPE_T * data_pack_type);
int reg_data_element_set_receive_finsh_flag(reg_data_element_t * reg_data_element, uint8_t flag);
int reg_data_element_set_transmit_finsh_flag(reg_data_element_t * reg_data_element, uint8_t flag);
int reg_data_element_diff_camp(reg_data_element_t * reg_data_element);
int reg_data_elemet_diff_eliminate(reg_data_element_t * reg_data_element);
int reg_data_element_get_addr(reg_data_element_t * reg_data_element);

int reg_data_core_run(reg_data_t * reg_data);

// when data_pack_type == DATA_PACK_TYPE_DOUBLE_ADDR
reg_data_pack_t * reg_data_get_pack_4addr_d(const reg_data_t * reg_data, uint16_t addr);
reg_data_element_t * reg_data_get_element_4addr_d(reg_data_pack_t * data_pack, uint16_t addr2);
// 迭代器
int reg_data_get_pack_iter_init_d(reg_data_t * reg_data, reg_data_iter_d_t * iter);
int reg_data_get_pack_iter_next_d(reg_data_t * reg_data, reg_data_iter_d_t * iter, reg_data_pack_t ** reg_data_pack, uint16_t * addr, size_t * index);
int reg_data_get_element_iter_init_d(reg_data_element_iter_t * iter, reg_data_pack_t * reg_data_pack);
int reg_data_get_element_iter_next_d(reg_data_element_iter_t * iter, reg_data_element_t ** reg_data_element, uint16_t * addr, size_t * index);


// when data_pack_type == DATA_PACK_TYPE_SIGNAL_ADDR
reg_data_element_t * reg_data_get_element_4addr_s(const reg_data_t * reg_data, uint16_t addr);
// 迭代器
int reg_data_get_pack_iter_init_s(reg_data_t * reg_data, reg_data_iter_s_t * iter);
int reg_data_get_pack_iter_next_s(reg_data_t * reg_data, reg_data_iter_s_t * iter, reg_data_element_t ** reg_data_element, uint16_t * addr, size_t * index);


#endif /* ZST_USE_REGISTER  */
#ifdef __cplusplus
}
#endif
#endif /* __REGISTER_H__ */

