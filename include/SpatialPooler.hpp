#pragma once

#include <SparseDistributedRepresentation.hpp>

template<typename SDR_t = unsigned int>
class SpatialPooler {
    public:
        SpatialPooler(SDR<SDR_t> attention, unsigned int num_outputs = 0);
        const SDR<SDR_t>& operator()(const SDR<SDR_t>& input, bool do_learning = true);

    private:
        static constexpr float CONNECTION_CHANCE = 0.25f;
        static constexpr float PERMANENCE_STD_DEV = 1;
        static constexpr float PERMANENCE_THRESHOLD = 0.5f;
        static constexpr float PERMANENCE_MIN = PERMANENCE_THRESHOLD - 1.0f;
        static constexpr float PERMANENCE_MAX = PERMANENCE_THRESHOLD + 1.0f;
        static constexpr float PERMANENCE_INCREMENT = 0.05f;
        static constexpr float PERMANENCE_DECREMENT = PERMANENCE_INCREMENT;
        static constexpr float OUTPUT_DENSITY = 0.05f;

        class Column {
            private:
                static float get_random_permanence() {
                    static std::normal_distribution<float> d{SpatialPooler<SDR_t>::POTENTIAL_POOL_CONNECTION_CHANCE, SpatialPooler<SDR_t>::PERMANENCE_STD_DEV};
                    return d(SDR<SDR_t>::get_twister());
                }

                static bool should_connect() {
                static SDR_t check_val = CONNECTION_CHANCE * (SDR<SDR_t>::get_twister().max() / 2);
                return SDR<SDR_t>::get_twister()() / 2 < check_val;
            }

                const SDR_t output;
                std::vector<float> permanences;
                SDR<SDR_t> connections;
            public:
                Column(const SDR<SDR_t>& inputs, SDR_t output): output(output) {
                    this->permanences.reserve(inputs.size() * CONNECTION_CHANCE);
                    this->connections.reserve(inputs.size() * CONNECTION_CHANCE);
                    for (SDR_t in : inputs) {
                        if (should_connect()) {
                            permanences.push_back(get_random_permanence());
                            connections.push_back(in);
                        }
                    }
                    this->permanences.shrink_to_fit();
                    this->connections.shrink_to_fit();
                }

                std::vector<typename SDR<SDR_t>::size_type> overlap_indices(const SDR<SDR_t>& input) {
                    return this->connections.andb(input);
                }

                void adjust_permanences(const std::vector<typename SDR<SDR_t>::size_type>& overlap_indices) {
                    auto indices_pos = overlap_indices.cbegin();
                    auto indices_end = overlap_indices.cend();
                    for (unsigned int i = 0; i < this->permanences.size(); ++i) {
                        bool end = indices_pos == indices_end;
                        bool matched;
                        if (!end) {
                            matched = *indices_pos == i;
                            if (matched) {
                                ++indices_pos;
                            }
                        }
                        if (end || !matched) {
                            float perm = this->permanences[i] - PERMANENCE_DECREMENT;
                            if (perm < PERMANENCE_MIN) perm = PERMANENCE_MIN;
                            this->permanences[i] = perm;
                        } else {
                            float perm = this->permanences[i] + PERMANENCE_INCREMENT;
                            if (perm > PERMANENCE_MAX) perm = PERMANENCE_MAX;
                            this->permanences[i] = perm;
                        }
                    }
                    auto perm_pos = this->permanences.cbegin();
                    auto perm_end = this->permanences.cend();
                    for (auto& p : this->permanences) {
                        // if (overlap_pos)
                    }
                }
        };

        std::vector<Column> columns;
};

template<typename SDR_t>
SpatialPooler<SDR_t>::SpatialPooler(SDR<SDR_t> attention, unsigned int num_outputs) {
    columns.reserve(num_outputs);

    SDR<SDR_t> outputs;
    outputs.reserve(num_outputs);
    while (outputs.size() < num_outputs) {
        outputs.set(SDR<SDR_t>::get_random_number(), true);
    }

    for (SDR_t out : outputs) {
        columns.emplace_back(attention, out);
    }
}

template<typename SDR_t>
const SDR<SDR_t>& SpatialPooler<SDR_t>::operator()(const SDR<SDR_t>& input, bool do_learning) {
    static SDR<SDR_t> ret;
    // ret.reserve(OUTPUT_DENSITY * this->)

    return ret;
}