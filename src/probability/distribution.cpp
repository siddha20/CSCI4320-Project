#include <cstddef>
#include <vector>
#include "distribution.h"


namespace probability {
Distribution::Distribution() = default;

/* Gets the index of using cumulative distribution. */
size_t Distribution::get_index(double f1) const {
    for (int i = 0; i < c_dist.size(); i++) {
        if (f1 <= c_dist[i]) return i;
    }
    return c_dist.size() - 1;
}

size_t Distribution::get_size() const { return size; }

}