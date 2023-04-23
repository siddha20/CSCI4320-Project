#pragma once
#include <vector>
#include <cstddef>
#include "distribution.h"


namespace probability {

class Custom : public Distribution {
    public:
        Custom();
        Custom(const std::vector<double> &dist);
};
}