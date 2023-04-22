#include "rijndael.h"

namespace crypto
{
const Rijndael::Configuration Rijndael::s_cfgs[3] = {
    { 4, 4, 10 },
    { 6, 4, 12 },
    { 8, 4, 14 }
};

const u8 Rijndael::s_sbox[16][16] = {
    { 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76 },
    { 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0 },
    { 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15 },
    { 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75 },
    { 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84 },
    { 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf },
    { 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8 },
    { 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2 },
    { 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73 },
    { 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb },
    { 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79 },
    { 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08 },
    { 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a },
    { 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e },
    { 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf },
    { 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }
};

const matrix_t Rijndael::s_rcon = {
    { 0x00, 0x00, 0x00, 0x00 }, { 0x01, 0x00, 0x00, 0x00 }, { 0x02, 0x00, 0x00, 0x00 }, { 0x04, 0x00, 0x00, 0x00 }, 
    { 0x08, 0x00, 0x00, 0x00 }, { 0x10, 0x00, 0x00, 0x00 }, { 0x20, 0x00, 0x00, 0x00 }, { 0x40, 0x00, 0x00, 0x00 }, 
    { 0x80, 0x00, 0x00, 0x00 }, { 0x1b, 0x00, 0x00, 0x00 }, { 0x36, 0x00, 0x00, 0x00 }, { 0x6c, 0x00, 0x00, 0x00 },
    { 0xd8, 0x00, 0x00, 0x00 }, { 0xab, 0x00, 0x00, 0x00 }, { 0x4d, 0x00, 0x00, 0x00 }, { 0x9a, 0x00, 0x00, 0x00 }
};

bool Rijndael::SetKey(const buf_t &k)
{
    assert(k.size() == 16 || k.size() == 24 || k.size() == 32);
    if (k.size() != 16 && k.size() != 24 && k.size() != 32)
        return false;
    m_cfg = &s_cfgs[k.size() / 8 - 2];

    m_w = BufToMatrix(k.data(), k.size(), m_cfg->Nb, m_cfg->Nb * (m_cfg->Nr + 1));
    for (i32 i = m_cfg->Nk; i < m_cfg->Nb * (m_cfg->Nr + 1); ++i)
    {
        buf_t temp = m_w[i - 1];
        if (i % m_cfg->Nk == 0)
            Xor(SubWord(RotWord(temp)), s_rcon[i / m_cfg->Nk]);
        else if (m_cfg->Nk > 6 && (i % m_cfg->Nk) == 4)
            SubWord(temp);
        m_w[i] = Xor(temp, m_w[i - m_cfg->Nk]);
    }
    return true;
}

bool Rijndael::Encrypt(const buf_t &pt, size_t off, buf_t &ct)
{
    // Must have non-empty plaintext and key
    size_t l = pt.size() - off;
    size_t block_size = BlockSize();
    if (pt.empty() || l < block_size || m_w.empty())
        return false;

    // We don't clear ct since this a block cipher and we
    // may be doing a lot of appending in various modes.
    // Same applies to Decrypt; we don't clear pt either.
    //ct.clear();

    // Encrypt the block
    matrix_t in = BufToMatrix(&pt[off], block_size, m_cfg->Nb), out;
    Cipher(in, out);
    for (size_t j = 0; j < out.size(); ++j)
        ct.insert(ct.end(), out[j].begin(), out[j].end());
    return true;
}

void Rijndael::Cipher(const matrix_t &in, matrix_t &out)
{
    matrix_t state = in;
    
    AddRoundKey(state, 0);
    for (i32 round = 1; round < m_cfg->Nr; ++round)
    {
        SubBytes(state, s_sbox);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, round);
    }

    SubBytes(state, s_sbox);
    ShiftRows(state);
    AddRoundKey(state, m_cfg->Nr);

    out = state;
}

void Rijndael::AddRoundKey(matrix_t &state, i32 round)
{
    for (size_t i = 0; i < state.size(); ++i)
        Xor(state[i], m_w[m_cfg->Nb * round + i]);
}

void Rijndael::SubBytes(matrix_t &state, const u8 sbox[16][16])
{
    for (size_t r = 0; r < state.size(); ++r)
    {
        for (size_t c = 0; c < state[r].size(); ++c)
            state[r][c] = LookupSbox(state[r][c], sbox);
    }
}

void Rijndael::ShiftRows(matrix_t &state)
{
    // TODO: Make more efficient w/o transpose lol
    state = MatrixTranspose(state);
    for (size_t i = 1; i < state.size(); ++i)
    {
        for (size_t j = 0; j < i; ++j)
            RotWord(state[i]);
    }
    state = MatrixTranspose(state);
}

void Rijndael::MixColumns(matrix_t &state)
{
    for (size_t i = 0; i < state.size(); ++i)
    {
        assert(m_cfg->Nb == 4 && state[i].size() == (size_t)m_cfg->Nb);
        u8 s0 = GFMul(2, state[i][0]) ^ GFMul(3, state[i][1]) ^ state[i][2] ^ state[i][3];
        u8 s1 = state[i][0] ^ GFMul(2, state[i][1]) ^ GFMul(3, state[i][2]) ^ state[i][3];
        u8 s2 = state[i][0] ^ state[i][1] ^ GFMul(2, state[i][2]) ^ GFMul(3, state[i][3]);
        u8 s3 = GFMul(3, state[i][0]) ^ state[i][1] ^ state[i][2] ^ GFMul(2, state[i][3]);
        state[i][0] = s0;
        state[i][1] = s1;
        state[i][2] = s2;
        state[i][3] = s3;
    }
}

buf_t &Rijndael::RotWord(buf_t &buf)
{
    if (buf.size() < 2)
        return buf;
    for (size_t i = 0; i < buf.size() - 1; ++i)
        std::swap(buf[i], buf[i + 1]);
    return buf;
}

buf_t &Rijndael::SubWord(buf_t &buf)
{
    for (size_t i = 0; i < buf.size(); ++i)
        buf[i] = LookupSbox(buf[i], s_sbox);
    return buf;
}

buf_t &Rijndael::Xor(buf_t &a, const buf_t &b)
{
    assert(a.size() == b.size());
    for (size_t i = 0; i < std::min(a.size(), b.size()); ++i)
        a[i] ^= b[i];
    return a;
}

u8 Rijndael::LookupSbox(u8 x, const u8 sbox[16][16])
{
    u8 lo = x & 0xf;
    u8 hi = (x >> 4) & 0xf;
    return sbox[hi][lo];
}

u8 Rijndael::GFMul(u8 a, u8 b)
{
    u8 poly = 0;
    for (i32 i = 0; i < 8; ++i)
    {
        if (b & 1)
            poly ^= a;
        bool hi = (a & 0x80) != 0;
        a <<= 1;
        if (hi)
            a ^= 0x1b; // x^8 + x^4 + x^3 + x + 1
        b >>= 1;
    }
    return poly;
}
}