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
    // Encrypt the data, then compute hashes over the encrypted data.
    // Append the resulting hashes to the end.
    // Prepend the number of hashes to the beginning of the data so we know
    // how large the actual encrypted data is.
    m_ctr.Crypt(data, length, offset, result);
    buf_t hashes;
    size_t numHashes = ComputeHashes(result.data(), length, hashes);
    result.insert(result.end(), hashes.begin(), hashes.end());
    result.insert(result.begin(), (u8*)&numHashes, (u8*)(&numHashes+1));
    return true;
}

bool Crypto::Decrypt(const u8 *data, size_t length, size_t offset, buf_t &result)
{
    // Must have at least number of hashes and some data
    if (length <= sizeof(size_t))
        return false;
    
    // Read number of hashes and remove from data
    size_t numHashes;
    memcpy(&numHashes, data, sizeof(size_t));
    size_t totalHashSize = 20 * numHashes;
    if (!numHashes || length - sizeof(size_t) <= totalHashSize)
        return false;
    data += sizeof(numHashes);
    length -= sizeof(numHashes);

    // Compute actual length of data (minus hashes)
    size_t actualLength = length - totalHashSize;

    // Compute hashes over our data, make sure the hashes match    
    buf_t hashes;
    ComputeHashes(data, actualLength, hashes, numHashes);
    if (hashes.size() != totalHashSize || memcmp(hashes.data(), data + actualLength, totalHashSize))
        return false;
    
    // Decrypt the same way that we encrypted
    m_ctr.Crypt(data, actualLength, offset, result);
    return true;
}

size_t Crypto::ComputeHashes(const u8 *data, size_t length, buf_t &dst, size_t numHashes)
{
    // Compute number of bytes to hash 
    size_t bytesPerHash = m_ctr.BlockSize() * m_blocksPerHash;
    if (numHashes == 0)
        numHashes = (length / bytesPerHash) + ((length % bytesPerHash) != 0);
    size_t hashStart = dst.size();
    dst.resize(dst.size() + numHashes * 20);
    
    // std vector of threads (use as many threads as we can)
    std::vector<std::thread> threads(std::thread::hardware_concurrency());
    size_t hashesPerThread = numHashes / threads.size();
    size_t bytesPerThread = hashesPerThread * bytesPerHash;

    // Spawn hash threads
    for (size_t i = 0; i < threads.size(); ++i)
    {
        size_t thisIndex = hashStart + hashesPerThread * 20 * i;
        size_t thisOffset = bytesPerThread * i;
        size_t remaining = length - thisOffset;
        size_t thisLength = (i != threads.size() - 1) ? std::min(remaining, bytesPerThread) : remaining;
        threads[i] = std::thread(&Crypto::HashThr, this, data + thisOffset, thisLength, &dst[thisIndex], bytesPerHash);
    }

    // Wait for hashing threads to complete
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