#pragma once
#include <vector>
#include <cstddef>
#include "distribution.h"


namespace probability {

/* TODO: Implement a customm probability distribution. */
/* This is for a custom probability distribution. The density and cumulative
distribution must be set each. */

class Custom : public Distribution {
    public:
        Custom();
        Custom(const std::vector<double> &dist);
};
}