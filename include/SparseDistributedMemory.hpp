#pragma once

#include <map>
#include "SparseDistributedRepresentation.hpp"

namespace {
template<typename SDR_t>
auto normalized_overlap = [](const SDR<SDR_t>& a, const SDR<SDR_t>& b) -> float {
    typename SDR<SDR_t>::size_type denominator = std::max(a.size(), b.size());
    if (denominator == 0) return 1;
    return (float)a.ands(b) / denominator;
};
}

/*
 * Inspired by:
 * https://en.wikipedia.org/wiki/Sparse_distributed_memory
 */
template<typename SDR_t = unsigned int, typename distance_metric = decltype(normalized_overlap<SDR_t>)>
class SDM {
    public:
        void write(const SDR<SDR_t>& address, const SDR<SDR_t>& value, float distance_threshold = 0.8f);
        SDR<SDR_t> read(const SDR<SDR_t>& address, float distance_threshold = 0.8f);
    
    private:
        std::map<SDR<SDR_t>, std::vector<SDR<SDR_t>>> memory;
};

template<typename SDR_t, typename distance_metric>
void SDM<SDR_t, distance_metric>::write(const SDR<SDR_t>& address, const SDR<SDR_t>& value, float distance_threshold) {
    auto mem_pos = this->memory.cbegin();
    auto mem_end = this->memory.cend();
    bool relevant_address_found = false;
    while (mem_pos != mem_end) {
        auto mem_pair = *mem_pos;
        const SDR<SDR_t>& mem_addr = mem_pair.first;
        if (distance_metric(mem_addr, address) < distance_threshold) {
            relevant_address_found = true;
            std::vector<SDR<SDR_t>>& mem_data = mem_pair.second;
            mem_data.push_back(value);
        }
        mem_pos++;
    }
    if (!relevant_address_found) {
        this->memory[address] = value;
    }
}

template<typename SDR_t, typename distance_metric>
SDR<SDR_t> SDM<SDR_t, distance_metric>::read(const SDR<SDR_t>& address, float distance_threshold) {
    SDR<SDR_t> output;
    auto mem_pos = this->memory.cbegin();
    auto mem_end = this->memory.cend();
    while (mem_pos != mem_end) {
        auto mem_pair = *mem_pos;
        const SDR<SDR_t>& mem_addr = mem_pair.first;
        if (distance_metric(mem_addr, address) < distance_threshold) {
            std::vector<SDR<SDR_t>>& mem_data = mem_pair.second;
            output &= mem_data;
        }
        mem_pos++;
    }
    return output;
}
