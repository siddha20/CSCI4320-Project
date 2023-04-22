#pragma once

namespace probability {

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