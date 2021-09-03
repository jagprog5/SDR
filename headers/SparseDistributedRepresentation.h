#pragma once

#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <functional>
#include <random>
#include <vector>
#include <iostream>
#include <math.h>
#include <stdbool.h>
#include <chrono>
using namespace std;

template<typename SDR_t = unsigned int>
class SDR {
    public:
        SDR() {}
        SDR(initializer_list<SDR_t> l);
        SDR(const SDR<SDR_t>& sdr) : v{sdr.v} {}
        SDR(float input, unsigned int size, unsigned int underlying_array_length);
        SDR(float input, float period, unsigned int size, unsigned int underlying_array_length);

        // Turns off bits such that the length matches the specified amount.
        SDR<SDR_t>& sample_length(unsigned int amount);

        // Each bit has a chance of being turned off, specified by amount, where 0 nearly always clears the sdr, and 1 leaves it unchanged.
        SDR<SDR_t>& sample_portion(float amount);

        // and bit. returns the state of a bit.
        bool andb(SDR_t val) const;
        // and bits. returns the state of many bits.
        SDR<SDR_t> andb(const SDR<SDR_t>& arg) const;
        // and bits. returns the state of many bits from start to stop.
        SDR<SDR_t> andb(SDR_t start_inclusive, SDR_t stop_exclusive) const;
        // and inplace. turn off all bits not in arg (compute arg AND this, and place the result in this). Returns this.
        SDR<SDR_t>& andi(const SDR<SDR_t>& arg);
        // and size. returns 0 if the bit is not contained in this, else 1.
        unsigned int ands(SDR_t val) const;
        // and size. returns the number of bits in both this and arg.
        unsigned int ands(const SDR<SDR_t>& arg) const;
        // and size. returns the number of bits from start to stop.
        unsigned int ands(SDR_t start_inclusive, SDR_t stop_exclusive) const;

        // or bits. 
        SDR<SDR_t> orb(const SDR<SDR_t>& arg) const;
        // or inplace. turn on all bits in arg. Returns this.
        SDR<SDR_t>& ori(const SDR<SDR_t>& arg);
        // or size. returns the number of bits in this or arg.
        unsigned int ors(const SDR<SDR_t>& arg) const;
        // xor bits. 
        SDR<SDR_t> xorb(const SDR<SDR_t>& arg) const;
        // xor inplace. computes this xor arg, and places the result in this. Returns this.
        SDR<SDR_t>& xori(const SDR<SDR_t>& arg);
        // xor size, aka hamming distance. returns the number of bits in this xor arg.
        unsigned int xors(const SDR<SDR_t>& arg) const;

        // turn off all bits in arg. Returns this.
        SDR<SDR_t>& rm(const SDR<SDR_t>& arg);
        
        // Returns this.
        SDR<SDR_t>& set(SDR_t index, bool value);
        // Returns this.
        SDR<SDR_t>& set(SDR<SDR_t> arg, bool value);

        // Returns this, shifted by amount.
        SDR<SDR_t>& shift(int amount);

        // concatenate an SDR to an SDR. Every indice in arg must be greater than every indice in this. Returns this.
        SDR<SDR_t>& join(const SDR<SDR_t>& arg);

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

        template<typename other>
        auto operator&(const other o) const { return andb(o); }
        template<typename other>
        auto operator&&(const other o) const { return ands(o); }
        template<typename other>
        auto operator&=(const other o) { return andi(o); }
        template<typename other>
        auto operator|(const other o) const { return orb(o); }
        template<typename other>
        auto operator||(const other o) const { return ors(o); }
        template<typename other>
        auto operator|=(const other o) { return ori(o); }
        template<typename other>
        auto operator^(const other o) const { return xorb(o); }
        template<typename other>
        auto operator<=(const other o) const { return xors(o); } // should be ^^. <= kinda works (hamming distance from a <= b).
        template<typename other>
        auto operator^=(const other o) { return xori(o); }
        template<typename other>
        auto operator+(const other o) const { return SDR(*this).set(o, true); }
        template<typename other>
        auto operator+=(const other o) { return set(o, true); }
        template<typename other>
        auto operator-(const other o) const { return SDR(*this).set(o, false); }
        template<typename other>
        auto operator-=(const other o) { return set(o, false); }
        template<typename other>
        auto operator%(const other o) const { return SDR(*this).sample_length(o); }
        template<typename other>
        auto operator%=(const other o) { return sample_length(o); }
        template<typename other>
        auto operator*(const other o) const { return SDR(*this).sample_portion(o); }
        template<typename other>
        auto operator*=(const other o) { return sample_potion(o); }
        template<typename other>
        auto operator<<(const other o) const { return SDR(*this).shift(o); }
        template<typename other>
        auto operator>>(const other o) const { return SDR(*this).shift(-o); }
        template<typename other>
        auto operator<<=(const other o) { return shift(o); }
        template<typename other>
        auto operator>>=(const other o) { return shift(-o); }
        template<typename other>
        auto operator<(const other o) { return join(o); }

        // rough benchmark for performance, also serves as some unit tests
        static void do_benchark();
    
    private:
        vector<SDR_t> v;

        static auto& get_twister() {
            static mt19937 twister(100);
            return twister;
        }

        union SDROPResult {
            SDR<SDR_t>* const sdr;
            unsigned int* const length;
        };

        // if r_pos is NULL, this indicates normal operation, and that the output is appended to r.
        // if r_pos is not NULL, this indicates that the operation is placing the result in one of it's operands, and r_pos indicates the overwrite position
        template <typename CIT, typename IT>
        static void sdrop_add_to_output(SDROPResult r, CIT& r_pos, IT a_pos, IT a_end, IT b_pos, IT b_end, SDR_t elem, const bool size_only);
        // and operation. computes A & B, and places the result in r.
        static void andop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<SDR_t>* const b, const bool size_only);
        // or operation. places the result in r.
        static void orop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<SDR_t>* const b,  const bool size_only, const bool exclusive);

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
    
    static_assert(is_integral<SDR_t>::value, "SDR_t must be integral");
};

template<typename SDR_t>
SDR<SDR_t>::SDR(initializer_list<SDR_t> l): v(l) {
    if (l.size() == 0) return;
    auto pos = l.begin();
    auto end = l.end();
    SDR_t prev_elem = *pos++;
    while (pos != end) {
        SDR_t elem = *pos++;
        assert(prev_elem < elem);
        prev_elem = elem;
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
        while (leading_indices > 0) v.push_back(--leading_indices);
        while (non_leading_indice < underlying_array_length) v.push_back(non_leading_indice++);
    } else {
        for (SDR_t i = 0; i < size; ++i) v.push_back(start_index + i);
    }
}

template<typename SDR_t>
SDR<SDR_t>::SDR(float input, unsigned int size, unsigned int underlying_array_length) {
    assert(size <= underlying_array_length);
    v.reserve(size);
    SDR_t start_index = roundf((underlying_array_length - size) * input);
    for (SDR_t i = 0; i < size; ++i) v.push_back(start_index + i);
}

template<typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::sample_length(unsigned int amount) {
    if (amount > v.size()) return *this;
    shuffle(v.begin(), v.end());
    v.resize(amount);
    sort(v.begin(), v.end());
    return *this;
}

template<typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::sample_portion(float amount) {
    assert(amount >= 0 && amount <= 1);
    unsigned int check_val = amount * (get_twister().max() / 2);
    auto to_offset = v.begin();
    auto from_offset = cbegin();
    auto end = v.end();
    while (from_offset != end) {
        if ((get_twister()() / 2) < check_val) {
            *to_offset++ = *from_offset;
        }
        from_offset++;
    }
    v.resize(distance(cbegin() - from_offset));
    return *this;
}

template<typename SDR_t>
bool SDR<SDR_t>::andb(SDR_t val) const {
    return lower_bound(cbegin(), cend(), val) == cend();
}

template<typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::set(SDR_t index, bool value) {
    auto it = lower_bound(cbegin(), cend(), index);
    if (value) {
        if (it == cend() || *it != index) v.insert(it, index);
    } else {
        if (it != cend() && *it == index) v.erase(it);
    }
    return *this;
}


template<typename SDR_t>
template<typename CIT, typename IT>
void SDR<SDR_t>::sdrop_add_to_output(SDROPResult r, CIT& r_pos, IT a_pos, IT a_end, IT b_pos, IT b_end, SDR_t elem, const bool size_only) {
    if (size_only) {
        ++*r.length;
    } else {
        if (r_pos != (decltype(r.sdr->v.begin()))NULL) {
            *r_pos++ = elem;
        } else {
            auto &rsdr = r.sdr->v;
            // allocate for half the max possible remaining elements
            if (rsdr.capacity() == rsdr.size()) {
                unsigned int a_left = distance(a_pos, a_end);
                unsigned int b_left = distance(b_pos, b_end);
                unsigned int max_remaining = a_left < b_left ? a_left : b_left;
                unsigned int cap_increase = (max_remaining + 1) / 2 + 1;
                rsdr.reserve(rsdr.capacity() + cap_increase);
            }
            rsdr.insert(rsdr.end(), elem);
        }
    }
}

template<typename SDR_t>
void SDR<SDR_t>::andop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<SDR_t>* const b, const bool size_only) {
    const bool is_inplace = !size_only && (r.sdr == a || r.sdr == b);
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    auto r_pos = is_inplace ? r.sdr->v.begin() : (decltype(r.sdr->v.begin()))NULL;
    SDR_t elem;
    decltype(a_pos) tmp;
    if (a_pos == a_end) goto end;
    loop:
        elem = *a_pos++;
        b_pos = lower_bound(b_pos, b_end, elem);
        if (b_pos == b_end) goto end;
        if (*b_pos == elem) {
            ++b_pos;
            sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, elem, size_only);
            if (b_pos == b_end) goto end;
        }
        tmp = a_pos;
        a_pos = b_pos;
        b_pos = tmp;
        tmp = a_end;
        a_end = b_end;
        b_end = tmp;
    goto loop;
    end:
    if (is_inplace) r.sdr->v.resize(distance(r.sdr->v.begin(), r_pos));
    if (!size_only) r.sdr->v.shrink_to_fit();
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::andb(const SDR<SDR_t>& arg) const {
    SDR r;
    SDROPResult rop{.sdr=&r};
    andop(rop, this, &arg, false);
    return r; // nrvo 
}

template<typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::andb(SDR_t start_inclusive, SDR_t stop_exclusive) const {
    SDR sdr;
    auto start_it = lower_bound(cbegin(), cend(), start_inclusive);
    auto end_it = lower_bound(start_it, cend(), stop_exclusive);
    sdr.v.reserve(distance(start_it, end_it));
    copy(start_it, end_it, back_inserter(sdr.v));
    return sdr; // nrvo
}

template<typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::andi(const SDR<SDR_t>& arg) {
    SDROPResult rop{.sdr = this};
    andop(rop, this, &arg, false);
    return *this;
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::ands(SDR_t val) const {
    return andb(val) ? 1 : 0;
};

template <typename SDR_t>
unsigned int SDR<SDR_t>::ands(const SDR<SDR_t>& arg) const {
    unsigned int r = 0;
    SDROPResult rop{.length=&r};
    andop(rop, this, &arg, true);
    return r;
}


template<typename SDR_t>
unsigned int SDR<SDR_t>::ands(SDR_t start_inclusive, SDR_t stop_exclusive) const {
    SDR sdr;
    auto end = cend();
    auto pos = lower_bound(cbegin(), end, start_inclusive);
    auto end_it = lower_bound(pos, end, stop_exclusive);
    return (unsigned int)(distance(pos, end_it));
}

template <typename SDR_t>
void SDR<SDR_t>::orop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<SDR_t>* const b, const bool size_only, const bool exclusive) {
    assert(size_only || (r.sdr != a && r.sdr != b));
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    bool a_valid = true;
    SDR_t a_val;
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    bool b_valid = true;
    SDR_t b_val;
    auto r_pos = (decltype(v.begin()))NULL;
    if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // get from a, or update a_valid if no more elements
    if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
    while (a_valid || b_valid) {
        if ((a_valid && !b_valid) || (a_valid && b_valid && a_val < b_val)) {
            sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_val, size_only);
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
        } else if ((!a_valid && b_valid) || (a_valid && b_valid && a_val > b_val)) {
            sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, b_val, size_only);
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        } else {
            if (!exclusive) {
                sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_val, size_only);
            }
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        }
    }
    if (!size_only) r.sdr->v.shrink_to_fit();
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::orb(const SDR<SDR_t>& arg) const {
    SDR r;
    SDROPResult rop{.sdr=&r};
    orop(rop, this, &arg, false, false);
    return r; // nrvo 
}


template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::ori(const SDR<SDR_t>& arg) {
    SDR r;
    SDROPResult rop{.sdr=&r};
    orop(rop, this, &arg, false, false);
    swap(r.v, v);
    return *this;
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::ors(const SDR<SDR_t>& arg) const {
    unsigned int r = 0;
    SDROPResult rop{.length = &r};
    orop(rop, this, &arg, true, false);
    return r;
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::xorb(const SDR<SDR_t>& arg) const {
    SDR r;
    SDROPResult rop{.sdr = &r};
    orop(rop, this, &arg, false, true);
    return r; // nrvo
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::xori(const SDR<SDR_t>& arg) {
    SDR r;
    SDROPResult rop{.sdr = &r};
    orop(rop, this, &arg, false, true);
    swap(r.c, v);
    return *this;
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::xors(const SDR<SDR_t>& arg) const {
    unsigned int r = 0;
    SDROPResult rop{.length = &r};
    orop(rop, this, &arg, true, true);
    return r;
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::rm(const SDR<SDR_t>& arg) {
    auto arg_pos = arg.crbegin();
    auto arg_end = arg.crend();
    auto this_start = v.begin();
    auto this_end = v.end();
    while (arg_pos != arg_end) {
        SDR_t arg_val = *arg_pos++;
        auto new_this_end = lower_bound(this_start, this_end, arg_val);
        bool arg_found = new_this_end != this_end && arg_val == *new_this_end;
        this_end = new_this_end;
        if (arg_found) {
            v.erase(this_end);
        } else {
            // this part isn't needed, but works better if this is sparse compared to arg.
            if (this_end == this_start) break;
            SDR_t this_val = *(this_end - 1);
            auto new_arg_pos = lower_bound(arg_pos, arg_end, this_val, greater<int>());
            bool this_found = new_arg_pos != arg_pos && this_val == *new_arg_pos;
            arg_pos = new_arg_pos;
            if (this_found) {
                v.erase(this_end - 1);
                ++arg_pos;
            }
        }
    }
    v.shrink_to_fit();
    return *this;
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::set(SDR<SDR_t> arg, bool value) {
    if (value) {
        ori(arg);
    } else {
        rm(arg);
    }
    return *this;
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::shift(int amount) {
    for (auto& elem : v) {
        #ifdef NDEBUG
        elem += amount;
        #else
        assert(!__builtin_add_overflow(amount, elem, &elem));
        #endif
    }
    return *this;
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::join(const SDR<SDR_t>& arg) {
    assert(ands(arg) == 0 && (size() == 0 || arg.size() == 0 || *v.crbegin() < *arg.v.cbegin()));
    v.reserve(v.size() + arg.v.size());
    for (auto e : arg.v) { v.push_back(e); }
    return *this;
}

template <typename SDR_t>
void SDR<SDR_t>::do_benchark() {
    enum test_op {andb, ands, orb, ors, xorb, xors, andi, rm};
    static struct {
        inline void operator() (SDR a[], SDR b[], int num_sdr, string name, test_op op) const {
            auto start = chrono::high_resolution_clock::now();
            for (int i = 0; i < num_sdr; ++i) {
                switch (op) {
                    case andb : a[i] & b[i]; break;
                    case ands : a[i] && b[i]; break;
                    case orb : a[i] | b[i]; break;
                    case ors : a[i] || b[i]; break;
                    case xorb : a[i] ^ b[i]; break;
                    case xors : a[i] <= b[i]; break;
                    case andi : a[i] &= b[i]; break;
                    case rm : a[i] -= b[i]; break;
                }
                
            }
            auto stop = chrono::high_resolution_clock::now();
            cout << name << ": " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << endl;
        }
    } time_funct;

    constexpr static int num_sdr = 5;
    constexpr static SDR_t index_max = 100000;
    SDR a[num_sdr];
    SDR b[num_sdr];
    auto start = chrono::high_resolution_clock::now();
    
    for (auto& elem : a) {
        for (SDR_t i = 0; i < index_max; ++i) {
            elem += get_twister()() % index_max;
        }
    }
    for (auto& elem : b) {
        for (SDR_t i = 0; i < index_max; ++i) {
            elem += get_twister()() % index_max;
        }
    }
    auto stop = chrono::high_resolution_clock::now();
    cout << "init: " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << endl;
    cout << "sizes: " << a[0].size() << endl;
    time_funct(a, b, num_sdr, "andb", test_op::andb);
    time_funct(a, b, num_sdr, "ands", test_op::ands);
    time_funct(a, b, num_sdr, "orb", test_op::orb);
    time_funct(a, b, num_sdr, "ors", test_op::ors);
    time_funct(a, b, num_sdr, "xorb", test_op::xorb);
    time_funct(a, b, num_sdr, "xors", test_op::xors);
    time_funct(a, b, num_sdr, "andi", test_op::andi);
    time_funct(a, b, num_sdr, "rm", test_op::rm);
}