#pragma once

#include <map>
#include "SparseDistributedRepresentation.hpp"

#include <iostream>

namespace SDRMem {
        // when reading from SDRMemory, the memory addresses are matched against the query address.
        // this similarity result is the overlap (ands) between each memory address and the query address.

        // based on the overlap score, how must of the memory address's corresponding value be used in the return value.
        // This is reflected in the return value of this lambda, from 0 to 1 inclusively,
        // where 1 means the entirety of the memory address's value should be used in the result.
        template<typename SDR_t>
        auto read_relevance = [](typename SDR<SDR_t>::size_type overlap_score,
                                    typename SDR<SDR_t>::size_type size_mem,
                                    typename SDR<SDR_t>::size_type size_query) -> float {
            float overlap_portion = overlap_score / std::max(size_mem, size_query);
            return overlap_portion < 0.5 ? 0 : overlap_score;
        };
}

template <typename SDR_key_t = unsigned int, typename value_t = SDR<SDR_key_t>, typename read_relevance = decltype(SDRMem::read_relevance<SDR_key_t>)>
class SDRMemory {
    public:
        void write(SDR<SDR_key_t> address, value_t value);
        value_t read(SDR<SDR_key_t> address);
        // amount is from 0 to 1, inclusively, where a higher value leads to more data loss
        void compress(float amount);
    
    private:
        std::vector<SDR<SDR_key_t>> memory;
};

template <typename SDR_key_t, typename value_t, typename read_relevance>
void SDRMemory<SDR_key_t, value_t, read_relevance>::write(SDR<SDR_key_t> address, value_t value) {
    
}

template <typename SDR_key_t, typename value_t, typename read_relevance>
value_t SDRMemory<SDR_key_t, value_t, read_relevance>::read(const SDR<SDR_key_t>& address) {
    SDR<SDR_key_t> output;
    for (const SDR<SDR_key_t>& elem : this->memory) {
        auto score = address && elem;

    }
}

template <typename SDR_key_t, typename value_t, typename read_relevance>
void SDRMemory<SDR_key_t, value_t, read_relevance>::compress(float amount) {
    
}