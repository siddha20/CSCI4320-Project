#ifndef SHA1
#define SHA1

#include "cryptocommon.h"

namespace crypto
{
class Sha1
{
public:
    static void ComputeDigest(const u8 *b, size_t size, u8 (&digest)[20]);
    static bool CompareDigests(const u8 *b, size_t size, const u8 *d);

private:
    static u32 RotLeft(u32 x, u32 b) { return (x << b) | (x >> (32 - b)); }
    static void Round(u32 s1[5], u32 t, u32 k, u32 w);
};
}

#endif