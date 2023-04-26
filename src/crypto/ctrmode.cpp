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
    // IV must contain at least 96 bits for the counter block
    // We use the last 32 bits for the counter value
    assert(iv.size() >= 12);
    assert(SetKey(key));
}

bool CtrMode::Crypt(const u8 *src, size_t srcLength, size_t offset, buf_t &dst)
{
    if (!srcLength)
        return false;

    // Calculate counter value to start at based on our offset in the file
    u32 ctrBase = (u32)(offset / 16);

    // Compute values to evenly distrubute chunks of the file to different threads
    // for AES encryption using CTR mode
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    size_t blockSize = BlockSize();
    size_t blocks = (srcLength / blockSize) + (srcLength % blockSize != 0);
    size_t blocksPerThread = blocks / threads.size();
    size_t bytesPerThread = blockSize * blocksPerThread;
    size_t bytes = blockSize * (srcLength / blockSize) + (srcLength % blockSize);
    assert(bytes == srcLength);
 
    // Prepare result output vector
    dst.clear();
    dst.resize(srcLength);

    // Spawn AES encryption threads
    // Note that this process is the same for both encryption and decryption
    for (size_t i = 0; i < threads.size(); ++i)
    {
        // Calculate the offset and length of the chunk that will be encrypted by this thread
        // This chunk can and will likely contain multiple 16 byte/128-bit blocks for AES.
        size_t thisOffset = bytesPerThread * i;
        size_t remaining = srcLength - thisOffset;
        size_t thisLength = (i != threads.size() - 1) ? std::min(remaining, bytesPerThread) : remaining;
        u32 ctrValue = ctrBase + i * blocksPerThread;
        threads[i] = std::thread(&CtrMode::CryptThr, this, src + thisOffset, thisLength, &dst[thisOffset], ctrValue);
    }

    // Wait for threads to complete the work
    for (size_t i = 0; i < threads.size(); ++i)
        threads[i].join();
    return true;
}

void CtrMode::CryptThr(const u8 *src, size_t srcLength, u8 *dst, u32 ctrValue)
{
    // Create counter starting at ctrValue
    Counter ctr(m_iv, ctrValue);

    // Encrypt the blocks
    size_t blockSize = BlockSize();
    for (size_t i = 0; i < srcLength; i += blockSize)
    {
        // Encrypt the counter block to get the XOR key
        const buf_t &ctrblock = ctr.GetBlock();
        buf_t ctrblock_out;
        Rijndael::Encrypt(ctrblock, ctrblock_out);

        // Encrypt the plaintext block with the XOR key we just got from AES
        for (size_t j = 0; j < std::min(srcLength - i, blockSize); ++j)
            dst[i + j] = src[i + j] ^ ctrblock_out[j];

        // Increment counter value for the next block
        ctr.Incr();
    }
}
}