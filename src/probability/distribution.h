#pragma once
#include <vector>

namespace probability {

enum DistributionType { UNIFORM, FULLBIAS, CUSTOM };

struct DistributionProperties {
    DistributionType type;
    size_t bias_index;
    std::vector<double> custom_dist;
};

class Distribution {
    public:
        Distribution();
        size_t get_index(double f1) const;
        size_t get_size() const;

    protected:
        size_t size;
        std::vector<double> dist;
        std::vector<double> c_dist;
};
}