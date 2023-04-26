#include "crypto.h"

using namespace crypto;

#define ENC_KEY_128 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0xd, 0xe, 0xf
#define ENC_KEY_192 KEY_128, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
#define ENC_KEY_256 KEY_192, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
#define ENC_KEY_CUR ENC_KEY_128
#define ENC_IV 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66
#define BLOCKS_PER_HASH 2048
#include <cassert>
#include <cstring>
#include <cstdio>
int main()
{
    Crypto c({ ENC_IV }, { ENC_KEY_128 }, 1000);
    buf_t pt, pt2;
    for (i32 i = 1; i < 10; ++i)
        pt.push_back((u8)i);
    buf_t ct;
    assert(c.Encrypt(pt.data(), pt.size(), 0, ct));
    assert(c.Decrypt(ct.data(), ct.size(), 0, pt2));
    
    for (size_t i = 0; i < pt2.size(); ++i)
        printf("%2x ", pt2[i]);
    printf("\n");
    
    assert(pt2.size() >= pt.size());
    assert(!memcmp(pt.data(), pt2.data(), pt.size()));
    return 0;
}
