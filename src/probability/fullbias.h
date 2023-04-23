#pragma once
#include <cstddef>
#include "distribution.h"


namespace probability {

class FullBias : public Distribution {
    public:
        FullBias();
        FullBias(size_t size, size_t index); 
};
}