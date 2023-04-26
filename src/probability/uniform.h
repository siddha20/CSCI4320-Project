#pragma once
#include <cstddef>
#include "distribution.h"


namespace probability {

/* Uniform distribution that is a child to Distribution. Uniformly samples a
permutation from a set of all permutations. */
class Uniform : public Distribution {
    public:
        Uniform();
        Uniform(size_t size);
};
}