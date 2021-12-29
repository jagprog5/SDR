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
    std::vector<MemScore<SDR_key_t, value_t>> scores;
};
}

template <typename SDR_key_t = unsigned int, typename value_t = SDR<SDR_key_t>>
class SDRMemory {
    public:
        SDRMemory() : SDRMemory(value_t()) {}
        SDRMemory(value_t written_value);

        void write(const SDR<SDR_key_t>& address, const value_t& value);
        value_t read(const SDR<SDR_key_t>& address, unsigned int depth = -1, unsigned int breadth = -1);
    private:
        std::map<SDR<SDR_key_t>, SDRMemory<SDR_key_t, value_t>> memory;
        value_t average;

        MemScores<SDR_key_t, value_t> getApplicableMemory();

        static constexpr float SCORE_SUM_THRESHOLD = 1;
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
MemScores<SDR_key_t, value_t> SDRMemory<SDR_key_t, value_t>::getApplicableMemory() {
    MemScores<SDR_key_t, value_t> scores;
    // TODO approximate k-nn instead of exhaustive search
    for (auto& mem : this->memory) {
        MemScore<SDR_key_t, value_t> score;
        score.score = SCORE(mem, mem);
        scores.score_sum += score.score;
        score.mem = mem;
        scores.scores.insert(std::upper_bound(scores.begin(), scores.end(), score), score);
    }
}

template <typename SDR_key_t, typename value_t>
void SDRMemory<SDR_key_t, value_t>::write(const SDR<SDR_key_t>& address, const value_t& value) {
    AVG_UPDATE(this->average, value);
    float score_sum = 0;
    // score each memory element, in descending order of score
    // std::vector<MemScore<SDR_key_t, value_t>> scores(this->memory.size());
    // for (auto& mem : this->memory) {
        
    //     MemScore<SDR_key_t, value_t> score;
    //     score.score = SCORE(address, mem);
    //     score_sum += score.score;
    //     score.mem = mem;
    //     scores.insert(std::upper_bound(scores.begin(), scores.end(), score), score);
    // }
    // if (score_sum < SCORE_SUM_THRESHOLD) {

    // }
}

template <typename SDR_key_t, typename value_t>
value_t SDRMemory<SDR_key_t, value_t>::read(const SDR<SDR_key_t>& address, unsigned int depth, unsigned int breadth) {
    if (depth == 0) return this->average;
    value_t output = value_t();
    return output;
}

// template <typename SDR_key_t, typename value_t>
// void SDRMemory<SDR_key_t, value_t>::compress(float amount) {
    
// }