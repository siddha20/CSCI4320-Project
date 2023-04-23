#pragma once
#include <cstddef>
#include "distribution.h"


namespace probability {

class Uniform : public Distribution {
    public:
        Uniform();
        Uniform(size_t size);
};
}