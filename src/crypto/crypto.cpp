#include "crypto.h"

#include <thread>
#include <vector>
#include <cstring>

namespace crypto
{

Crypto::Crypto(const buf_t &iv, const buf_t &key, size_t blocksPerHash)
    : m_ctr(iv, key)
    , m_blocksPerHash(blocksPerHash)
{}

bool Crypto::Encrypt(const u8 *data, size_t length, size_t offset, buf_t &result)
{
    m_ctr.Crypt(data, length, offset, result);
    buf_t hashes;
    size_t numHashes = ComputeHashes(result.data(), length, hashes);
    result.insert(result.end(), hashes.begin(), hashes.end());
    result.insert(result.begin(), (u8*)&numHashes, (u8*)(&numHashes+1));
    return true;
}

bool Crypto::Decrypt(const u8 *data, size_t length, size_t offset, buf_t &result)
{
    if (length <= sizeof(size_t))
        return false;
    
    size_t numHashes;
    memcpy(&numHashes, data, sizeof(size_t));
    size_t totalHashSize = 20 * numHashes;
    if (!numHashes || length - sizeof(size_t) <= totalHashSize)
        return false;
    data += sizeof(numHashes);
    length -= sizeof(numHashes);
    size_t actualLength = length - totalHashSize;
    

    buf_t hashes;
    ComputeHashes(data, actualLength, hashes, numHashes);
    if (hashes.size() != totalHashSize || memcmp(hashes.data(), data + actualLength, totalHashSize))
        return false;
    
    m_ctr.Crypt(data, length, offset, result);
    return true;
}

size_t Crypto::ComputeHashes(const u8 *data, size_t length, buf_t &dst, size_t numHashes)
{
    size_t bytesPerHash = m_ctr.BlockSize() * m_blocksPerHash;
    if (numHashes == 0)
        numHashes = (length / bytesPerHash) + ((length % bytesPerHash) != 0);
    size_t hashStart = dst.size();
    dst.resize(dst.size() + numHashes * 20);
    
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    size_t hashesPerThread = numHashes / threads.size();
    size_t bytesPerThread = hashesPerThread * bytesPerHash;

    for (size_t i = 0; i < threads.size(); ++i)
    {
        size_t thisIndex = hashStart + hashesPerThread * 20 * i;
        size_t thisOffset = bytesPerThread * i;
        size_t remaining = length - thisOffset;
        size_t thisLength = (i != threads.size() - 1) ? std::min(remaining, bytesPerThread) : remaining;
        threads[i] = std::thread(&Crypto::HashThr, this, data + thisOffset, thisLength, &dst[thisIndex], bytesPerHash);
    }

    for (size_t i = 0; i < threads.size(); ++i)
        threads[i].join();
    return numHashes;
}

void Crypto::HashThr(const u8 *src, size_t srcLength, u8 *dst, size_t blockSize)
{
    u8 digest[20];
    for (size_t i = 0; i < srcLength; i += blockSize)
    {
        Sha1::ComputeDigest(src + i, std::min(srcLength - i, blockSize), digest);
        memcpy(dst, digest, sizeof(digest));
        dst += sizeof(digest);
    }
}

}