#ifndef SHA1
#define SHA1

#include "cryptocommon.h"

namespace crypto
{
class Sha1
{
public:
    static buf_t ComputeDigest(const buf_t &b);
    static bool CompareDigests(const buf_t &b, const buf_t &d);

private:
    static u32 RotLeft(u32 x, u32 b) { return (x << b) | (x >> (32 - b)); }
    static void Round(u32 s1[5], u32 t, u32 k, u32 w);
};
}

#endif