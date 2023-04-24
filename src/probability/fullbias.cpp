#include <vector>
#include <numeric>
#include "fullbias.h"


namespace probability {

FullBias::FullBias() = default;

FullBias::FullBias(size_t size, size_t index) {
    this->size = size;
    dist = std::vector<double>(size);
    c_dist = std::vector<double>(size);
    dist[index] = 1.0;
    std::partial_sum(dist.begin(), dist.end(), c_dist.begin());
}
}