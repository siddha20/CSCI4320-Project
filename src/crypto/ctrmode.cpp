#include "ctrmode.h"
#include <cstring>
#include <cassert>

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
    : m_encCtr(iv, 1)
    , m_decCtr(iv, 1)
{
    assert(iv.size() >= 12);
    assert(SetKey(key));
}

bool CtrMode::Crypt(const buf_t &src, buf_t &target, Counter &ctr)
{
    if (src.empty())
        return false;

    // We will fill the resulting ciphertext
    target.clear();
    target.resize(src.size());

    // Encrypt the blocks
    size_t block_size = BlockSize();
    for (size_t i = 0; i < src.size(); i += block_size)
    {
        const buf_t &ctrblock = ctr.GetBlock();
        buf_t ctrblock_out;
        if (!Rijndael::Encrypt(ctrblock, ctrblock_out))
            return false;
        for (size_t j = 0; j < std::min(src.size() - i, block_size); ++j)
            target[i + j] = src[i + j] ^ ctrblock_out[j];
        ctr.Incr();
    }
    return true;
}
}