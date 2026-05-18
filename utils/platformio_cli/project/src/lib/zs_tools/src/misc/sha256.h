#ifndef __SHA256_H__
#define __SHA256_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "zst_conf.h"
#if (ZST_SHA256==1)
typedef struct
{
    uint32_t state[8]; // 哈希状态
    uint64_t bitlen;   // 总比特长度
    uint8_t data[64];  // 数据块
    size_t datalen;    // 当前块使用长度
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);
void sha256_final(SHA256_CTX *ctx, uint8_t hash[32]);

#endif
#ifdef __cplusplus
}
#endif

#endif /* __SHA256_H__ */
