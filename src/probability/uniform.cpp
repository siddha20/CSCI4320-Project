#include <vector>
#include <numeric>
#include "uniform.h"


namespace probability {

Uniform::Uniform() = default;

/* Unifrom distribution implementation. */
Uniform::Uniform(size_t size) {
        this->size = size;
        dist = std::vector<double>(size, 1.0/size);
        c_dist = std::vector<double>(size);
        std::partial_sum(dist.begin(), dist.end(), c_dist.begin());
    }
}