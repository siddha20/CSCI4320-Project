#ifndef CTRMODE_H
#define CTRMODE_H

#include "cryptocommon.h"
#include "rijndael.h"

namespace crypto
{
class CtrMode : private Rijndael
{
public:
    class Counter
    {
    public:
        Counter(const buf_t &iv, u32 value);

        const buf_t &GetBlock() const { return buf; }
        void Incr() { ++*(u32*)&buf[buf.size()-4]; }

    private:
        buf_t buf;
    };

    CtrMode(const buf_t &iv, const buf_t &key);

    bool Crypt(const u8 *src, size_t srcLength, size_t offset, buf_t &dst);

    size_t BlockSize() const { return Rijndael::BlockSize(); }

private:
    void CryptThr(const u8 *src, size_t srcLength, u8 *dst, u32 ctrValue);

    buf_t m_iv;
};
}

#endif