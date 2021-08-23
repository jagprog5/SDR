#pragma once

#include <assert.h>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <iostream>
#include <math.h>
#include <stdbool.h>
using namespace std;

template<typename SDR_t = unsigned int>
class SDR {
    static_assert(std::is_integral<SDR_t>::value, "SDR_t must be integral");

    public:
        SDR() {}
        SDR(const SDR<SDR_t>& sdr) : v{sdr.v} {}
        void set(SDR_t index, bool value);
        // void set(SDR<SDR_t> query, bool value); // TODO
        bool get(SDR_t index);
        SDR<SDR_t> get(SDR_t start_inclusive, SDR_t stop_exclusive);
        SDR<SDR_t> get(const SDR<SDR_t>& query);
        // turn off all bits not in query
        void focus(const SDR<SDR_t>& query);
        // turn off all bits in query
        // void rm(const SDR<SDR_t>& query); // TODO
        // returns the length of the result if get(query) was performed.
        unsigned int score(const SDR<SDR_t>& query);

        auto cbegin() const { return v.cbegin(); }
        auto cend() const { return v.cend(); }
        auto crbegin() const { return v.crbegin(); }
        auto crend() const { return v.crend(); }

        friend ostream& operator<<(ostream& os, const SDR<SDR_t>& sdr) {
            static constexpr FormatText beginning;
            os << beginning.arr; 
            for (auto it = sdr.cbegin(), end = sdr.cend(); it != end; ++it) { 
                const auto i = *it;
                cout << i;
                if (it + 1 != end) cout << ", ";
            }
            cout << ']';
            return os;
        }
    
    private:
        vector<SDR_t> v;

        void turn_off(SDR_t);
        void turn_on(SDR_t);

        union SDROPResult {
            SDR<SDR_t>* sdr; // ptr to initialized sdr, to be overwritten
            unsigned int* length; // ptr to uninitialized int
        };
        static void overlap(SDROPResult r, const SDR<SDR_t>* a, const SDR<SDR_t>* b, bool length_only);

        struct FormatText {
            char arr[3 + (int)ceil(log10(sizeof(SDR_t) * 8 + 1))] = {0};
            constexpr FormatText() {
                int i = sizeof(arr) / sizeof(arr[0]) - 1;
                int s = sizeof(SDR_t) * 8;
                arr[i--] = '\0';
                arr[i--] = '[';
                do {
                    arr[i--] = '0' + s % 10;
                    s /= 10;
                } while (s > 0);
                arr[i--] = ((SDR_t)-1) >= 0 ? 'u' : 'i';
            }
        };
};

template<typename SDR_t>
void SDR<SDR_t>::set(SDR_t index, bool value) {
    if (value) turn_on(index);
    else turn_off(index);
}

template<typename SDR_t>
void SDR<SDR_t>::turn_on(SDR_t index) {
    auto b =  _bound(cbegin(), cend(), index);
    if (v.empty() || *b != index) { // skip duplicate
        v.insert(b, index);
    }
}

template<typename SDR_t>
void SDR<SDR_t>::turn_off(SDR_t index) {
    auto b = lower_bound(cbegin(), cend(), index);
    if (!v.empty() && *b == index) {
        v.erase(b);
    }
}

// template<typename SDR_t> // TODO
// void set(SDR<SDR_t> query, bool value) {
//     if (value) {
//         focus(query);
//     } else {
        
//     }
// }

template<typename SDR_t>
void SDR<SDR_t>::focus(const SDR<SDR_t>& query) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    overlap(rop, this, &query, false);
    this->v.swap(r.v);
}

template<typename SDR_t>
// void SDR<SDR_t>::rm(const SDR<SDR_t>& query) { // TODO
//     auto query_pos = query.crbegin();
//     auto query_end = query.crend();
//     SDR_t query_val;
//     auto this_pos = crbegin();
//     auto this_end = crend();
//     SDR_t this_val;
//     while (query_pos != query_end) {
//         query_val = *query_pos++;
//         if (this_pos != this_end) {
//             this_val = *this_pos++;
//         } else {
//             break;
//         }
//     }

//     v.shrink_to_fit();
// }

template<typename SDR_t>
bool SDR<SDR_t>::get(SDR_t index) {
    return !v.empty() && *lower_bound(cbegin(), cend(), index) == index;
}

template<typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::get(SDR_t start_inclusive, SDR_t stop_exclusive) {
    SDR<SDR_t> sdr;
    auto start_it = lower_bound(cbegin(), cend(), start_inclusive);
    auto end_it = lower_bound(start_it, cend(), stop_exclusive);
    sdr.v.reserve(distance(start_it, end_it));
    copy(start_it, end_it, back_inserter(sdr.v));
    return sdr; // nrvo
}

template<typename SDR_t>
void SDR<SDR_t>::overlap(SDROPResult r, const SDR<SDR_t>* a, const SDR<SDR_t>* b, bool length_only) {
    assert(length_only || (a != r.sdr && b != r.sdr));
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    SDR_t a_val;
    bool a_valid = true;
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    SDR_t b_val;
    bool b_valid = true;
    if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // get from a, or update a_valid if no more elements
    if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
    while (a_valid && b_valid) {
        if (a_val < b_val) {
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
        } else if (a_val > b_val) {
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        } else {
            if (!length_only) {
                auto &v = r.sdr->v;
                if (true) { // is v a random access container?
                    if (v.capacity() == v.size()) {
                        // allocate for half the max possible remaining elements
                        unsigned int a_left = distance(a_pos, a_end);
                        unsigned int b_left = distance(b_pos, b_end);
                        unsigned int max_remaining = a_left < b_left ? a_left : b_left;
                        unsigned int cap_increase = (max_remaining + 1) / 2 + 1;
                        v.reserve(v.capacity() + cap_increase);
                    }
                }
                v.push_back(a_val);
            } else {
                ++*r.length;
            }
            
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        }
    }
    if (length_only) {
        r.sdr->v.shrink_to_fit();
    }
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::get(const SDR<SDR_t>& query) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    overlap(rop, this, &query, false);
    return r; // nrvo 
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::score(const SDR<SDR_t>& query) {
    unsigned int r;
    SDROPResult rop;
    rop.length = &r;
    overlap(rop, this, &query, true);
    return r;
}
