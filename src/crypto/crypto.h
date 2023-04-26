#ifndef CRYPTOMPI_H
#define CRYPTOMPI_H

#include "ctrmode.h"
#include "sha1.h"

namespace crypto
{
class Crypto final
{
public:
    Crypto(const buf_t &iv, const buf_t &key, size_t blocksPerHash);

    bool Encrypt(const u8 *data, size_t length, size_t offset, buf_t &result);
    bool Decrypt(const u8 *data, size_t length, size_t offset, buf_t &result);

private:
    size_t ComputeHashes(const u8 *data, size_t length, buf_t &dst, size_t numHashes = 0);
    void HashThr(const u8 *src, size_t srcLength, u8 *dst, size_t blockSize);

    CtrMode m_ctr;
    size_t m_blocksPerHash;
};
}

#endif