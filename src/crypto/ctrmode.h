#ifndef CTRMODE_H
#define CTRMODE_H

#include "cryptocommon.h"
#include "rijndael.h"

namespace crypto
{
class CtrMode : private Rijndael
{
    class Counter
    {
    public:
        Counter(const buf_t &iv, u32 value);

        const buf_t &GetBlock() const { return buf; }
        void Incr() { ++*(u32*)&buf[buf.size()-4]; }

    private:
        buf_t buf;
    };

public:
    CtrMode(const buf_t &iv, const buf_t &key);

    // Encrypts plaintext to get the ciphertext
    // Returns if the encrypt succeeds
    bool Encrypt(const buf_t &pt, buf_t &ct) { return Crypt(pt, ct, m_encCtr); }
    
    // Decrypts ciphertext to get the plaintext
    // Returns if the decrypt succeeds
    bool Decrypt(const buf_t &ct, buf_t &pt) { return Crypt(ct, pt, m_decCtr); }

private:
    bool Crypt(const buf_t &src, buf_t &target, Counter &ctr);

    Counter m_encCtr;
    Counter m_decCtr;
};
}

#endif