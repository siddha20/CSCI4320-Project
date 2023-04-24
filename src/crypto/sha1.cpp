#include "sha1.h"

#include <cstring>

namespace crypto
{
void Sha1::ComputeDigest(const u8 *b, size_t size, u8 (&digest)[20])
{
    assert(size != 0);
    if (!size)
        return;
        
    u64 bits = (u64)(size * 8);
    u32 chnk = (u32)((size + 8) / 64 + 1);
    u32 tl = (u32)(64 * chnk - size);
    u8 t[128] = { 0x80 };
    for (i32 i = 0; i < 8; ++i)
        t[tl - (i + 1)] = ((u8*)&bits)[i];

    u32 s2[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
    for (u32 i = 0, p = 0; i < chnk; ++i)
    {
        u32 words[80] = {};
        for (u32 w = 0; w < 16; ++w)
        {
            // Load words we can fit
            i32 remw = 24;
            for (; p < size && remw >= 0; ++p, remw -= 8)
                words[w] += ((u32)b[p]) << remw;
            
            // Padding
            for (; remw >= 0; ++p, remw -= 8)
                words[w] += ((u32)t[p - size]) << remw;
        }

        // Extend 16 words into 32 bit words
        for (u32 w = 16; w < 32; ++w)
            words[w] = RotLeft(words[w - 3] ^ words[w - 8] ^ words[w - 14] ^ words[w - 16], 1);
        for (u32 w = 32; w < 80; ++w)
            words[w] = RotLeft(words[w - 6] ^ words[w - 16] ^ words[w - 28] ^ words[w - 32], 2);
        
        // Do transformations with 4 different types of rounds with 20 operations each.
        u32 s1[5] = { s2[0], s2[1], s2[2], s2[3], s2[4] };
        for (i32 i = 0; i < 20; ++i)
            Round(s1, (s1[1] & s1[2]) | (~s1[1] & s1[3]), 0x5A827999, words[i]);
        for (i32 i = 20; i < 40; ++i)
            Round(s1, s1[1] ^ s1[2] ^ s1[3], 0x6ED9EBA1, words[i]);
        for (i32 i = 40; i < 60; ++i)
            Round(s1, (s1[1] & s1[2]) | (s1[1] & s1[3]) | (s1[2] & s1[3]), 0x8F1BBCDC, words[i]);
        for (i32 i = 60; i < 80; ++i)
            Round(s1, s1[1] ^ s1[2] ^ s1[3], 0xCA62C1D6, words[i]);
        
        // Update state
        for (i32 i = 0; i < 5; ++i)
            s2[i] += s1[i];
    }

    // Get digest
    for (i32 i = 0; i < 5; ++i)
    {
        for (i32 j = 0; j < 4; ++j)
            digest[i * 4 + j] = ((const u8*)&s2[i])[3 - j];
    }
}

void Sha1::Round(u32 s1[5], u32 t, u32 k, u32 w)
{
    u32 r = RotLeft(s1[0], 5) + t + s1[4] + k + w;
    s1[4] = s1[3];
    s1[3] = s1[2];
    s1[2] = RotLeft(s1[1], 30);
    s1[1] = s1[0];
    s1[0] = r;
}

bool Sha1::CompareDigests(const u8 *b, size_t size, const u8 *d)
{
    u8 digest[20];
    ComputeDigest(b, size, digest);
    return !memcmp(b, digest, 20);
}
}