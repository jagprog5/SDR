#pragma once

#include <map>
#include "SparseDistributedRepresentation.hpp"

template <typename SDR_key_t, typename value_t>
class SDRMemory;
namespace {
template <typename SDR_key_t, typename value_t>
struct MemScore {
    float score;
    SDRMemory<SDR_key_t, value_t>& mem;
    bool operator<(const MemScore& other) const { return score < other.score; }
};

template <typename SDR_key_t, typename value_t>
struct MemScores {
    float score_sum = 0;
    // sorted in descending score order
    // scores are normalized so the sum is 1
    std::vector<MemScore<SDR_key_t, value_t>> scores;
};
}

template <typename SDR_key_t = unsigned int, typename value_t = SDR<SDR_key_t>>
class SDRMemory {
    public:
        SDRMemory() : SDRMemory(value_t()) {}
        SDRMemory(value_t written_value);

        void write(const SDR<SDR_key_t>& address, const value_t& value);
        value_t read(const SDR<SDR_key_t>& address, unsigned int depth = -1);
    private:
        std::map<SDR<SDR_key_t>, SDRMemory<SDR_key_t, value_t>> memory;
        value_t average;

        MemScores<SDR_key_t, value_t> getApplicableMemory(const SDR<SDR_key_t> query);

        static constexpr float SCORE_SUM_THRESHOLD = 0.5;
        static float SCORE(const SDR<SDR_key_t>& addr, const SDR<SDR_key_t>& mem) {
            return addr.ands(mem) / std::max(addr.size(), mem.size());
        }
        static void AVG_UPDATE(value_t& average, const value_t& towards) {
            average += (towards - average) * 0.1;
        }
};

template <typename SDR_key_t, typename value_t>
SDRMemory<SDR_key_t, value_t>::SDRMemory(value_t written_value) {
    this->average = written_value;
}

template <typename SDR_key_t, typename value_t>
MemScores<SDR_key_t, value_t> SDRMemory<SDR_key_t, value_t>::getApplicableMemory(const SDR<SDR_key_t> query) {
    MemScores<SDR_key_t, value_t> mem_scores;
    // TODO approximate k-nn instead of exhaustive search
    auto mem_pos = this->memory.begin();
    auto mem_end = this->memory.end();
    while (mem_pos != mem_end) {
        SDRMemory<SDR_key_t, value_t>& mem = (*mem_pos).second;
        float score = SCORE(query, mem);
        MemScore<SDR_key_t, value_t> mem_score = {score, mem};
        mem_scores.score_sum += score;
        mem_scores.scores.insert(std::upper_bound(mem_scores.scores.begin(), mem_scores.scores.end(), mem_score), mem_score);
        ++mem_pos;
    }
    for (MemScore<SDR_key_t, value_t>& mem_score : mem_scores.scores) {
        mem_score.score /= mem_scores.score_sum;
    }
    return mem_scores;
}

template <typename SDR_key_t, typename value_t>
void SDRMemory<SDR_key_t, value_t>::write(const SDR<SDR_key_t>& address, const value_t& value) {
    AVG_UPDATE(this->average, value);
    auto scores = getApplicableMemory(address);
    if (scores.score_sum < SCORE_SUM_THRESHOLD) {
        this->memory[address] = SDRMemory(value);
    } else {
        
    }
}

template <typename SDR_key_t, typename value_t>
value_t SDRMemory<SDR_key_t, value_t>::read(const SDR<SDR_key_t>& address, unsigned int depth) {
    if (depth == 0) return this->average;
    value_t output = value_t();
    return output;
}

// template <typename SDR_key_t, typename value_t>
// void SDRMemory<SDR_key_t, value_t>::compress(float amount) {
    
// }