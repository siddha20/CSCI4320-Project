#ifndef CRYPTOCOMMON_H
#define CRYPTOCOMMON_H

#include <cstdint>
#include <cassert>
#include <vector>
#include <string>

namespace crypto
{
using i32 = std::int32_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;
using buf_t = std::vector<u8>;
using matrix_t = std::vector<buf_t>;

inline matrix_t MatrixTranspose(const matrix_t &matrix)
{
    size_t size = matrix.size();
    matrix_t result = matrix;
    for (size_t r = 0; r < size; ++r)
    {
        assert(matrix[r].size() == size);
        for (size_t c = 0; c < size; ++c)
            result[c][r] = matrix[r][c];
    }
    return result;
}

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
}

#endif