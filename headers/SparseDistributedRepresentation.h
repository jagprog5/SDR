#pragma once

#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <math.h>
#include <stdbool.h>
using namespace std;

template<typename SDR_t = unsigned int>
class SDR {
    public:
        SDR() {}
        SDR(initializer_list<SDR_t> l): v(l) {}
        SDR(const SDR<SDR_t>& sdr) : v{sdr.v} {}
        SDR(float input, unsigned int size, unsigned int underlying_array_length);
        SDR(float input, float period, unsigned int size, unsigned int underlying_array_length);

        void sample(unsigned int amount);
        void sample(float amount);

        // and bit. returns the state of a bit.
        bool andb(SDR_t val) const;
        // and bits. returns the state of many bits.
        SDR<SDR_t> andb(const SDR<SDR_t>& query) const;
        // and bits. returns the state of many bits from start to stop.
        SDR<SDR_t> andb(SDR_t start_inclusive, SDR_t stop_exclusive) const;
        // and inplace. turn off all bits not in query (compute query AND self, and place the result in self).
        void andi(const SDR<SDR_t>& query);
        // and size. returns 0 if the bit is not contained in this, else 1.
        unsigned int ands(SDR_t val) const;
        // and size. returns the number of bits in both this and query.
        unsigned int ands(const SDR<SDR_t>& query) const;
        // and size. returns the number of bits from start to stop.
        unsigned int ands(SDR_t start_inclusive, SDR_t stop_exclusive) const;

        // or bits. 
        SDR<SDR_t> orb(const SDR<SDR_t>& query) const;
        // or inplace. turn on all bits in query
        void ori(const SDR<SDR_t>& query);
        // or size. returns the number of bits in this or query.
        unsigned int ors(const SDR<SDR_t>& query) const;
        // xor bits. 
        SDR<SDR_t> xorb(const SDR<SDR_t>& query) const;
        // xor inplace. computes self xor query, and places the result in self.
        void xori(const SDR<SDR_t>& query);
        // xor size, aka hamming distance. returns the number of bits in this xor query.
        unsigned int xors(const SDR<SDR_t>& query) const;
        
        void set(SDR_t index, bool value);
        void set(SDR<SDR_t> query, bool value);

        auto cbegin() const { return v.cbegin(); }
        auto cend() const { return v.cend(); }
        auto crbegin() const { return v.crbegin(); }
        auto crend() const { return v.crend(); }
        auto size() const { return v.size(); }

        friend ostream& operator<<(ostream& os, const SDR<SDR_t>& sdr) {
            static constexpr FormatText beginning;
            os << beginning.arr; 
            for (auto it = sdr.cbegin(), end = sdr.cend(); it != end; ++it) { 
                const auto i = *it;
                cout << i;
                if (next(it) != end) cout << ", ";
            }
            cout << ']';
            return os;
        }
    
    private:
        vector<SDR_t> v;

        void turn_off(SDR_t);
        void turn_on(SDR_t);

        // turn off all bits in query
        void rm(const SDR<SDR_t>& query);

        union SDROPResult {
            SDR<SDR_t>* sdr;
            unsigned int* length;
        };

        template <typename IT>
        static void sdrop_add_to_output(SDROPResult r, bool size_only, IT a_pos, IT a_end, IT b_pos, IT b_end, SDR_t elem);
        // and operation. computes A & B, and places the result in r.
        static void andop(SDROPResult r, const SDR<SDR_t>* a, const SDR<SDR_t>* b, bool size_only);
        // or operation. places the result in r.
        static void orop(SDROPResult r, const SDR<SDR_t>* a, const SDR<SDR_t>* b, bool size_only, bool exclusive);

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

        template<typename>
        struct is_vector : false_type {};
        template<typename T, typename A>
        struct is_vector<vector<T,A>> : true_type {};
        template<typename>
        struct is_list : false_type {};
        template<typename T, typename A>
        struct is_list<list<T,A>> : true_type {};
    
    static_assert(is_integral<SDR_t>::value, "SDR_t must be integral");
    static_assert(is_vector<decltype(v)>::value || is_list<decltype(v)>::value, "SDR's underlying container must be a vector or list.");
};

template<typename SDR_t>
SDR<SDR_t>::SDR(float input, unsigned int size, unsigned int underlying_array_length) {
    assert(size <= underlying_array_length);
    v.reserve(size);
    SDR_t start_index = roundf((underlying_array_length - size) * input);
    for (SDR_t i = 0; i < size; ++i) {
        v.push_back(start_index + i);
    }
}

template<typename SDR_t>
SDR<SDR_t>::SDR(float input, float period, unsigned int size, unsigned int underlying_array_length) {
    assert(size <= underlying_array_length && period != 0);
    v.reserve(size);
    float progress = input / period;
    progress -= (int)progress;
    assert(progress >= 0);
    SDR_t start_index = roundf(progress * underlying_array_length);
    if (start_index + size > underlying_array_length) {
        SDR_t leading_indices = start_index + size - underlying_array_length;
        SDR_t non_leading_indice = underlying_array_length - leading_indices - 1;
        while (leading_indices > 0) {
            v.push_back(--leading_indices);
        }
        while (non_leading_indice < underlying_array_length) {
            v.push_back(non_leading_indice++);
        }
    } else {
        for (SDR_t i = 0; i < size; ++i) {
            v.push_back(start_index + i);
        }
    }
}

template<typename SDR_t>
void SDR<SDR_t>::sample(unsigned int amount) {
    if (amount > v.size()) return;
    shuffle(v.begin(), v.end());
    v.resize(amount);
    sort(v.begin(), v.end());
}

template<typename SDR_t>
bool SDR<SDR_t>::andb(SDR_t val) const {
    return !v.empty() && *lower_bound(cbegin(), cend(), val) == val;
}

template<typename SDR_t>
void SDR<SDR_t>::turn_on(SDR_t index) {
    auto b = lower_bound(cbegin(), cend(), index);
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

template<typename SDR_t>
void SDR<SDR_t>::set(SDR_t index, bool value) {
    if (value) turn_on(index);
    else turn_off(index);
}

template<typename SDR_t>
template<typename IT>
void SDR<SDR_t>::sdrop_add_to_output(SDROPResult r, bool size_only, IT a_pos, IT a_end, IT b_pos, IT b_end, SDR_t elem) {
    if (size_only) {
        ++r.length;
    } else {
        auto &v = r.sdr->v;
        if constexpr(is_vector<decltype(r.sdr->v)>::value) {
            // allocate for half the max possible remaining elements
            if (v.capacity() == v.size()) {
                unsigned int a_left = distance(a_pos, a_end);
                unsigned int b_left = distance(b_pos, b_end);
                unsigned int max_remaining = a_left < b_left ? a_left : b_left;
                unsigned int cap_increase = (max_remaining + 1) / 2 + 1;
                v.reserve(v.capacity() + cap_increase);
            }
        }
        v.push_back(elem);
    }
}

template<typename SDR_t>
void SDR<SDR_t>::andop(SDROPResult r, const SDR<SDR_t>* a, const SDR<SDR_t>* b, bool size_only) {
    assert(size_only || (a != r.sdr && b != r.sdr));
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    bool a_focused = true;
    while (a_pos != a_end && b_pos != b_end) {
        auto& focus_pos = a_focused ? a_pos : b_pos;
        SDR_t focus_elem = *focus_pos++;
        auto& other_pos = a_focused ? b_pos : a_pos;
        auto other_end = a_focused ? b_end : a_end;
        other_pos = lower_bound(other_pos, other_end, focus_elem);
        SDR_t other_elem = *other_pos;
        if (focus_elem == other_elem) {
            ++other_pos;
            sdrop_add_to_output(r, size_only, a_pos, a_end, b_pos, b_end, focus_elem);
        }
        a_focused = !a_focused;
    }
    if constexpr(is_vector<decltype(a->v)>::value) {
        if (!size_only) {
            r.sdr->v.shrink_to_fit();
        }
    }
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::andb(const SDR<SDR_t>& query) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    andop(rop, this, &query, false);
    return r; // nrvo 
}

template<typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::andb(SDR_t start_inclusive, SDR_t stop_exclusive) const {
    SDR<SDR_t> sdr;
    auto start_it = lower_bound(cbegin(), cend(), start_inclusive);
    auto end_it = lower_bound(start_it, cend(), stop_exclusive);
    if constexpr(is_vector<decltype(v)>::value) sdr.v.reserve(distance(start_it, end_it));
    copy(start_it, end_it, back_inserter(sdr.v));
    return sdr; // nrvo
}

template<typename SDR_t>
void SDR<SDR_t>::andi(const SDR<SDR_t>& a) {
    // Ideally this function should be encompassed by andop (as was the case 2 commits ago in the c implementation)
    // but for simplicity I decided to make it it's own function, even with a bit of copy-paste over.
    auto a_pos = a.cbegin();
    auto a_end = a.cend();
    auto this_pos = cbegin();
    unsigned int num_elements = 0;
    auto this_insert_pos = v.begin();
    auto this_end = cend();
    bool a_focused = true;
    while (a_pos != a_end && this_pos != this_end) {
        auto& focus_pos = a_focused ? a_pos : this_pos;
        SDR_t focus_elem = *focus_pos++;
        auto& other_pos = a_focused ? this_pos : a_pos;
        auto other_end = a_focused ? this_end : a_end;
        other_pos = lower_bound(other_pos, other_end, focus_elem);
        SDR_t other_elem = *other_pos;
        if (focus_elem == other_elem) {
            ++other_pos;
            ++num_elements;
            *this_insert_pos++ = focus_elem;
        }
        a_focused = !a_focused;
    }
    v.resize(num_elements);
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::ands(SDR_t val) const {
    return andb(val) ? 1 : 0;
};

template <typename SDR_t>
unsigned int SDR<SDR_t>::ands(const SDR<SDR_t>& query) const {
    unsigned int r = 0;
    SDROPResult rop;
    rop.length = &r;
    andop(rop, this, &query, true);
    return r;
}

template<typename SDR_t>
unsigned int SDR<SDR_t>::ands(SDR_t start_inclusive, SDR_t stop_exclusive) const {
    SDR<SDR_t> sdr;
    auto cend = cend();
    auto pos_it = lower_bound(cbegin(), cend, start_inclusive);
    if constexpr(is_vector<decltype(v)>::value) {
        auto end_it = lower_bound(pos_it, cend, stop_exclusive);
        return (unsigned int)(distance(pos_it, end_it));
    } else {
        unsigned int i = 0;
        while (pos_it != cend) {
            ++i;
            ++pos_it;
        }
        return i;
    }
}

template <typename SDR_t>
void SDR<SDR_t>::orop(SDROPResult r, const SDR<SDR_t>* a, const SDR<SDR_t>* b, bool size_only, bool exclusive) {
    assert(size_only || (a != r.sdr && b != r.sdr));
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    bool a_valid = true;
    SDR_t a_val;
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    bool b_valid = true;
    SDR_t b_val;
    if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // get from a, or update a_valid if no more elements
    if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
    while (a_valid || b_valid) {
        if ((a_valid && !b_valid) || (a_valid && b_valid && a_val < b_val)) {
            sdrop_add_to_output(r, size_only, a_pos, a_end, b_pos, b_end, a_val);
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
        } else if ((!a_valid && b_valid) || (a_valid && b_valid && a_val > b_val)) {
            sdrop_add_to_output(r, size_only, a_pos, a_end, b_pos, b_end, b_val);
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        } else {
            if (!exclusive) {
                sdrop_add_to_output(r, size_only, a_pos, a_end, b_pos, b_end, a_val);
            }
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        }
    }
    if constexpr(is_vector<decltype(a->v)>::value) {
        if (!size_only) {
            r.sdr->v.shrink_to_fit();
        }
    }
}

// template <typename SDR_t>
// void SDR<SDR_t>::orb(SDR_t val) const {

// }

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::orb(const SDR<SDR_t>& query) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &query, false);
    return r; // nrvo 
}

template<typename SDR_t>
void SDR<SDR_t>::ori(const SDR<SDR_t>& query) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &query, false, false);
    swap(r.v, v);
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::ors(const SDR<SDR_t>& query) const {
    unsigned int r = 0;
    SDROPResult rop;
    rop.length = &r;
    orop(rop, this, &query, true, false);
    return r;
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::xorb(const SDR<SDR_t>& query) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &query, true);
    return r; // nrvo 
}

template<typename SDR_t>
void SDR<SDR_t>::xori(const SDR<SDR_t>& query) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &query, false, true);
    swap(r.v, v);
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::xors(const SDR<SDR_t>& query) const {
    unsigned int r = 0;
    SDROPResult rop;
    rop.length = &r;
    orop(rop, this, &query, true, true);
    return r;
}

template<typename SDR_t>
void SDR<SDR_t>::rm(const SDR<SDR_t>& query) {
    auto query_pos = query.crbegin();
    auto query_end = query.crend();
    SDR_t query_val;
    auto this_start = v.begin();
    auto this_end = v.end();
    SDR_t this_val;
    while (query_pos != query_end) {
        query_val = *query_pos++;
        auto new_this_end = lower_bound(this_start, this_end, query_val);
        if (new_this_end == this_end) continue;
        if (query_val == *new_this_end){
            this_end = prev(new_this_end);
            v.erase(new_this_end);
            this_end = next(this_end);
        } else {
            this_end = new_this_end;
        }
    }
    if constexpr(is_vector<decltype(v)>::value) v.shrink_to_fit();
}

template<typename SDR_t>
void SDR<SDR_t>::set(SDR<SDR_t> query, bool value) {
    if (value) {
        ori(query);
    } else {
        rm(query);
    }
}