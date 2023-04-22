#ifndef RIJNDAEL_H
#define RIJNDAEL_H

#include "cryptocommon.h"

#include <array>
#include <algorithm>

namespace crypto
{
class Rijndael
{
    struct Configuration
    {
        i32 Nk;
        i32 Nb;
        i32 Nr;
    }; 

public:
    virtual ~Rijndael() = default;

    bool SetKey(const buf_t &k);

    // Encrypts plaintext to get the ciphertext
    // Returns if the encrypt succeeds
    bool Encrypt(const buf_t &pt, buf_t &ct) { return Encrypt(pt, 0, ct); }
    
    size_t BlockSize() const { return m_cfg ? (size_t)m_cfg->Nb * 4 : 0; }

protected:
    bool Encrypt(const buf_t &pt, size_t off, buf_t &ct);

private:
    void Cipher(const matrix_t &in, matrix_t &out);

    void AddRoundKey(matrix_t &state, i32 round);
    void SubBytes(matrix_t &state, const u8 sbox[16][16]);
    void ShiftRows(matrix_t &state);
    void MixColumns(matrix_t &state);
    
    static buf_t &RotWord(buf_t &buf);
    static buf_t &SubWord(buf_t &buf);
    static buf_t &Xor(buf_t &a, const buf_t &b);
    static u8 LookupSbox(u8 x, const u8 sbox[16][16]);
    static u8 GFMul(u8 a, u8 b);

    static const Configuration s_cfgs[3];
    static const u8 s_sbox[16][16];
    static const u8 s_inv_sbox[16][16];
    static const matrix_t s_rcon;

    const Configuration *m_cfg;
    matrix_t m_w;
};
}

#endif