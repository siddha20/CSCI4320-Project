#ifndef CRYPTOCOMMON_H
#define CRYPTOCOMMON_H

#include <cstdint>
#include <cassert>
#include <vector>
#include <string>
#include <array>

namespace crypto
{
using i32 = std::int32_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;
using buf_t = std::vector<u8>;
using buf4_t = std::array<u8, 4>;
using matrix_t = std::vector<buf_t>;
using matrix44_t = std::array<buf4_t, 4>;

inline matrix_t BufToMatrix(const u8 *buf, size_t size, size_t rows, size_t rowSize=0)
{
    assert(size > 0 && rows > 0);
    matrix_t m;
    m.resize(std::max(rows, rowSize));
    for (size_t i = 0; i < m.size(); ++i)
        m[i].resize(4);
    for (size_t i = 0; i < size; ++i)
        m[i / rows][i % rows] = buf[i];
    return m;
}

inline matrix44_t BufToMatrix44(const u8 *buf)
{
    matrix44_t m;
    m[0][0] = buf[0];
    m[0][1] = buf[1];
    m[0][2] = buf[2];
    m[0][3] = buf[3];
    m[1][0] = buf[4];
    m[1][1] = buf[5];
    m[1][2] = buf[6];
    m[1][3] = buf[7];
    m[2][0] = buf[8];
    m[2][1] = buf[9];
    m[2][2] = buf[10];
    m[2][3] = buf[11];
    m[3][0] = buf[12];
    m[3][1] = buf[13];
    m[3][2] = buf[14];
    m[3][3] = buf[15];
    return m;
}

inline buf_t hexstring_to_buf(const char *str)
{
    buf_t result;
    i32 b = 0;
    for (size_t i = 0; i < std::char_traits<char>::length(str); ++i)
    {
        i32 num = (str[i] >= '0' && str[i] <= '9') ? (str[i] - '0') : (10 + (str[i] - 'a'));
        assert(num >= 0 && num <= 15);
        if ((i & 1) == 0)
            b = num << 4;
        else
        {
            b |= num;
            result.push_back((u8)b);
            b = 0;
        }
    }
    return result;
}
}

#endif