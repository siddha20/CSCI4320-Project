#include "ctrmode.h"
#include <cstring>
#include <cassert>
#include <thread>
#include <iostream>

namespace crypto
{
CtrMode::Counter::Counter(const buf_t &iv, u32 value)
{
    // Use first 12 bytes as iv
    buf.resize(16);
    memcpy(&buf[0], iv.data(), std::min(iv.size(), (size_t)12));
    *(u32*)&buf[buf.size()-4] = value;
}

CtrMode::CtrMode(const buf_t &iv, const buf_t &key)
    : m_iv(iv)
{
    assert(iv.size() >= 12);
    assert(SetKey(key));
}

bool CtrMode::Crypt(const u8 *src, size_t srcLength, size_t offset, buf_t &dst)
{
    if (!srcLength)
        return false;

    u32 ctrBase = (u32)(offset / 16);
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    size_t blockSize = BlockSize();
    size_t blocks = (srcLength / blockSize) + (srcLength % blockSize != 0);
    size_t blocksPerThread = blocks / threads.size();
    size_t bytesPerThread = blockSize * blocksPerThread;
    size_t bytes = blockSize * (srcLength / blockSize) + (srcLength % blockSize);
    assert(bytes == srcLength);
    dst.clear();
    dst.resize(srcLength);

    for (size_t i = 0; i < threads.size(); ++i)
    {
        size_t thisOffset = bytesPerThread * i;
        size_t remaining = srcLength - thisOffset;
        size_t thisLength = (i != threads.size() - 1) ? std::min(remaining, bytesPerThread) : remaining;
        u32 ctrValue = ctrBase + i * blocksPerThread;
        threads[i] = std::thread(&CtrMode::CryptThr, this, src + thisOffset, thisLength, &dst[thisOffset], ctrValue);
    }

    for (size_t i = 0; i < threads.size(); ++i)
        threads[i].join();
    return true;
}

void CtrMode::CryptThr(const u8 *src, size_t srcLength, u8 *dst, u32 ctrValue)
{
    Counter ctr(m_iv, ctrValue);

    // Encrypt the blocks
    size_t blockSize = BlockSize();
    for (size_t i = 0; i < srcLength; i += blockSize)
    {
        const buf_t &ctrblock = ctr.GetBlock();
        buf_t ctrblock_out;
        Rijndael::Encrypt(ctrblock, ctrblock_out);
        for (size_t j = 0; j < std::min(srcLength - i, blockSize); ++j)
            dst[i + j] = src[i + j] ^ ctrblock_out[j];
        ctr.Incr();
    }
}
}