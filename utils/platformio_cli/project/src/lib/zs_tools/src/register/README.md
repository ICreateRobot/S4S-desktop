# 简介
寄存器模型。一般与i2c相结合，实现如eeprom类似的读写方式。



# 使用

- 使用 `reg_data_pack_init_t` 进行初始化
- 使用 `reg_data_t` 注册寄存器全局句柄

## reg_data_t

### data_pack_type

DATA_PACK_TYPE_DOUBLE_ADDR : 双地址

DATA_PACK_TYPE_SIGNAL_ADDR : 单地址

**双地址**

寻址需要双地址才能得到最后的element

**单地址**

寻址单个地址就能得到最后的element

### mini_flag

主要与内部的实现有关

为0时，内部会为第一个等级的地址创建一个 hash 表。

> 单地址：为每一个元素（element）都在 hash表 中申请一个空间。
>
> 双地址：为一个等级的地址创建一个 hash 表，为第一等级的每一个地址都在 hash 表中都申请一个空间。第二地址以 element      array的形式挂载在第一等级的地址元素上

为1时，内部 hash 大小强制为1，即退化为链表

> 单地址：不再为每一个元素都在hash表中申请一个空间，而是将 element array 的 reg_data_pack_t 添加到 hash 中，即链表中。每次寻址都需要遍历 hash 中的 reg_data_pack_t 来获取 element。适合 功能设备数量少 的场景，否则遍历会浪费很多时间。
>
> 双地址：同单地址。

# 注意事项
- 单个地址不一定对应单个字节，而是可以对应多个字节。
- 一般所有寄存器可读可写，不可写寄存器需要手动实现。