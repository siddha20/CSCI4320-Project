#pragma once
#include <cstddef>
#include "distribution.h"


namespace probability {

/* A child class to distribution. Full Bias is a custom distribution such that
one candiate is uncondiationally preferred to all others. */
class FullBias : public Distribution {
    public:
        FullBias();
        FullBias(size_t size, size_t index); 
};
}