#pragma once

#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <functional>
#include <random>
#include <vector>
#include <iostream>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <chrono>

template<typename SDR_t = unsigned int>
class SDR {
    public:
        using IndexType = SDR_t;
        using size_type = typename std::vector<SDR_t>::size_type;

        SDR() {}
        SDR(std::vector<SDR_t>&& v);
        SDR(std::initializer_list<SDR_t> l);
        SDR(float input, unsigned int size, unsigned int underlying_array_length);
        SDR(float input, float period, unsigned int size, unsigned int underlying_array_length);

        // Turns off bits such that the length matches the specified amount.
        SDR<SDR_t>& sample_length(unsigned int amount);

        // Each bit has a chance of being turned off, specified by amount, where 0 nearly always clears the sdr, and 1 leaves it unchanged.
        SDR<SDR_t>& sample_portion(float amount);

        // and bit. returns the state of a bit.
        bool andb(SDR_t val) const;
        // and bits. returns the state of many bits.
        template<typename arg_t = unsigned int>
        SDR<SDR_t> andb(const SDR<arg_t>& arg) const;
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
        // and positions. returns the positions in this in which the elements of arg reside.
        std::vector<size_type> andp(const SDR<SDR_t>& arg) const;

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
        
        // Sets bit in this, then returns this.
        SDR<SDR_t>& set(SDR_t index, bool value);
        // Sets bits in this, then returns this.
        SDR<SDR_t>& set(SDR<SDR_t> arg, bool value);

        // Returns this, shifted by amount.
        SDR<SDR_t>& shift(int amount);

        // concatenate an SDR to an SDR. Every indice in arg must be greater than every indice in this. Returns this.
        SDR<SDR_t>& join(const SDR<SDR_t>& arg);

        // removes the bits in common between a and b.
        template<typename SDR_t_inner>
        friend void separate(SDR<SDR_t_inner>& a, SDR<SDR_t_inner>& b);

        auto cbegin() const { return v.cbegin(); }
        auto cend() const { return v.cend(); }
        auto crbegin() const { return v.crbegin(); }
        auto crend() const { return v.crend(); }
        auto size() const { return v.size(); }
        auto operator[](unsigned int index) const { return v[index]; };
        void resize(size_type n) { assert(n <= v.size()); v.resize(n); }
        void reserve(size_type n) { v.reserve(n); }
        void shrink_to_fit() { v.shrink_to_fit(); }
        void push_back(SDR_t i) { assert(size() == 0 || v[v.size() - 1] < i); v.push_back(i); }

        template<typename SDR_t_inner>
        friend std::ostream& operator<<(std::ostream& os, const SDR<SDR_t_inner>& sdr);

        template<typename other>
        auto operator&(const other& o) const { return andb(o); }
        template<typename other>
        auto operator&&(const other& o) const { return ands(o); }
        template<typename other>
        auto operator&=(const other& o) { return andi(o); }
        template<typename other>
        auto operator|(const other& o) const { return orb(o); }
        template<typename other>
        auto operator||(const other& o) const { return ors(o); }
        template<typename other>
        auto operator|=(const other& o) { return ori(o); }
        template<typename other>
        auto operator^(const other& o) const { return xorb(o); }
        template<typename other>
        auto operator<=(const other& o) const { return xors(o); } // should be ^^. <= kinda works (hamming distance from a <= b).
        template<typename other>
        auto operator^=(const other& o) { return xori(o); }
        template<typename other>
        auto operator+(const other& o) const { return SDR(*this).set(o, true); }
        template<typename other>
        auto operator+=(const other& o) { return set(o, true); }
        template<typename other>
        auto operator-(const other& o) const { return SDR(*this).set(o, false); }
        template<typename other>
        auto operator-=(const other& o) { return set(o, false); }
        template<typename other>
        auto operator%(const other& o) const { return SDR(*this).sample_length(o); }
        template<typename other>
        auto operator%=(const other& o) { return sample_length(o); }
        template<typename other>
        auto operator*(const other& o) const { return SDR(*this).sample_portion(o); }
        template<typename other>
        auto operator*=(const other& o) { return sample_potion(o); }
        template<typename other>
        auto operator<<(const other& o) const { return SDR(*this).shift(o); }
        template<typename other>
        auto operator>>(const other& o) const { return SDR(*this).shift(-o); }
        template<typename other>
        auto operator<<=(const other& o) { return shift(o); }
        template<typename other>
        auto operator>>=(const other& o) { return shift(-o); }
        template<typename other>
        auto operator<(const other& o) { return join(o); }
        auto operator==(const SDR<SDR_t>& other) const { return other.v == this->v; }
        auto operator!=(const SDR<SDR_t>& other) const { return !(other == *this); }

        // static ref to mersenne twister with result type SDR_t
        static auto& get_twister() {
            // same as mt19937
            static std::mersenne_twister_engine<SDR_t,32,624,397,31,0x9908b0df,11,0xffffffff,7,0x9d2c5680,15,0xefc60000,18,1812433253> twister(time(NULL) * getpid());
            return twister;
        }

        static SDR_t get_random_number() {
            return get_twister()();
        }

        // rough benchmark for performance
        static void do_benchark();
    
    private:
        std::vector<SDR_t> v;

        void assert_ascending(); // used in constructors

        union SDROPResult {
            SDR<SDR_t>* sdr;
            unsigned int* length;
        };

        // if r_pos is NULL, this indicates normal operation, and that the output is appended to r.
        // if r_pos is not NULL, this indicates that the operation is placing the result in one of it's operands, and r_pos indicates the overwrite position
        template <typename CIT, typename AIT, typename BIT>
        static void sdrop_add_to_output(SDROPResult r, CIT& r_pos, AIT a_pos, AIT a_end, BIT b_pos, BIT b_end, SDR_t elem, bool size_only);
        // and operation. computes A & B, and places the result in r.
        template <typename arg_t>
        static void andop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<arg_t>* const b, bool size_only);
        // or operation. places the result in r.
        static void orop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<SDR_t>* const b,  bool size_only, bool exclusive);

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
    
    // static_assert(std::is_integral<SDR_t>::value, "SDR_t must be integral");
    static constexpr bool print_type = false;
};

template<typename SDR_t>
void SDR<SDR_t>::assert_ascending() {
    #ifndef NDEBUG
        if (v.size() == 0) return;
        auto pos = v.begin();
        auto end = v.end();
        SDR_t prev_elem = *pos++;
        while (pos != end) {
            SDR_t elem = *pos++;
            assert(prev_elem < elem);
            prev_elem = elem;
        }
    #endif
}

template<typename SDR_t>
SDR<SDR_t>::SDR(std::vector<SDR_t>&& v): v(std::move(v)) {
    assert_ascending();
}

template<typename SDR_t>
SDR<SDR_t>::SDR(std::initializer_list<SDR_t> l): v(l) {
    assert_ascending();
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
    std::vector<SDR_t> temp;
    temp.reserve(amount);
    sample(cbegin(), cend(), back_inserter(temp), amount, get_twister());
    swap(temp, v);
    return *this;
}

template<typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::sample_portion(float amount) {
    assert(amount >= 0 && amount <= 1);
    SDR_t check_val = amount * (get_twister().max() / 2);
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
template <typename CIT, typename AIT, typename BIT>
void SDR<SDR_t>::sdrop_add_to_output(SDROPResult r, CIT& r_pos, AIT a_pos, AIT a_end, BIT b_pos, BIT b_end, SDR_t elem, bool size_only) {
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
template<typename arg_t>
void SDR<SDR_t>::andop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<arg_t>* const b, bool size_only) {
    assert(size_only || r.sdr != (const SDR<SDR_t>*)b);
    bool is_inplace = !size_only && r.sdr == a;
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    SDR_t a_elem;
    arg_t b_elem;
    auto r_pos = is_inplace ? r.sdr->v.begin() : (decltype(r.sdr->v.begin()))NULL;
    decltype(a_pos) tmp;
    if (a_pos == a_end) goto end;
    loop:
        a_elem = *a_pos++;
        b_pos = lower_bound(b_pos, b_end, a_elem);
        if (b_pos == b_end) goto end;
        if (*b_pos == a_elem) {
            ++b_pos;
            sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_elem, size_only);
            if (b_pos == b_end) goto end;
        }
        // ============= a and b swapped ===^=V================
        b_elem = *b_pos++;
        a_pos = lower_bound(a_pos, a_end, b_elem);
        if (a_pos == a_end) goto end;
        a_elem = *a_pos;
        if (a_elem == b_elem) {
            ++a_pos;
            sdrop_add_to_output(r, r_pos, b_pos, b_end, a_pos, a_end, a_elem, size_only);
            if (a_pos == a_end) goto end;
        }
    goto loop;
    end:
    if (is_inplace) r.sdr->v.resize(distance(r.sdr->v.begin(), r_pos));
    if (!size_only) r.sdr->v.shrink_to_fit();
}

template <typename SDR_t>
template<typename arg_t>
SDR<SDR_t> SDR<SDR_t>::andb(const SDR<arg_t>& arg) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
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
    SDROPResult rop;
    rop.sdr = this;
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
    SDROPResult rop;
    rop.length = &r;
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

template<typename SDR_t>
std::vector<typename SDR<SDR_t>::size_type> SDR<SDR_t>::andp(const SDR<SDR_t>& arg) const {
    std::vector<typename SDR<SDR_t>::size_type> ret;
    auto begin = cbegin();
    auto pos = begin;
    auto end = cend();
    for (SDR_t a : arg.v) {
        pos = lower_bound(pos, end, a);
        if (pos == end) return ret;
        if (*pos == a) ret.v.push_back(pos - begin);
    }
    return ret;
}

template <typename SDR_t>
void SDR<SDR_t>::orop(SDROPResult r, const SDR<SDR_t>* const a, const SDR<SDR_t>* const b, bool size_only, bool exclusive) {
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
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, false);
    return r; // nrvo 
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::ori(const SDR<SDR_t>& arg) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, false);
    swap(r.v, v);
    return *this;
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::ors(const SDR<SDR_t>& arg) const {
    unsigned int r = 0;
    SDROPResult rop;
    rop.length = &r;
    orop(rop, this, &arg, true, false);
    return r;
}

template <typename SDR_t>
SDR<SDR_t> SDR<SDR_t>::xorb(const SDR<SDR_t>& arg) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, true);
    return r; // nrvo
}

template <typename SDR_t>
SDR<SDR_t>& SDR<SDR_t>::xori(const SDR<SDR_t>& arg) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, true);
    swap(r.c, v);
    return *this;
}

template <typename SDR_t>
unsigned int SDR<SDR_t>::xors(const SDR<SDR_t>& arg) const {
    unsigned int r = 0;
    SDROPResult rop;
    rop.length = &r;
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
            if (this_end == this_start) break;
            SDR_t this_val = *(this_end - 1);
            auto new_arg_pos = lower_bound(arg_pos, arg_end, this_val, std::greater<int>());
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
void separate(SDR<SDR_t>& a, SDR<SDR_t>& b) {
    auto a_pos = a.v.begin();
    auto a_end = a.v.end();
    auto b_pos = b.v.begin();
    auto b_end = b.v.end();
    SDR_t elem;
    if (a_pos == a_end) goto end;
    loop:
        elem = *a_pos++;
        b_pos = lower_bound(b_pos, b_end, elem);
        if (b_pos == b_end) goto end;
        if (*b_pos == elem) {
            b_pos = b.v.erase(b_pos);
            if (b_pos == b_end) goto end;
        }
        // ======
        elem = *b_pos++;
        a_pos = lower_bound(a_pos, a_end, elem);
        if (a_pos == a_end) goto end;
        if (*a_pos == elem) {
            a_pos = a.v.erase(a_pos);
            if (a_pos == a_end) goto end;
        }
    goto loop;
    end:
    a.v.shrink_to_fit();
    b.v.shrink_to_fit();
}

template<typename SDR_t>
std::ostream& operator<<(std::ostream& os, const SDR<SDR_t>& sdr) {
    if constexpr(SDR<SDR_t>::print_type) {
        static constexpr typename SDR<SDR_t>::FormatText beginning;
        os << beginning.arr;
    } else {
        os << '[';
    }
    for (auto it = sdr.cbegin(), end = sdr.cend(); it != end; ++it) { 
        const auto i = *it;
        os << i;
        if (next(it) != end) os << ",";
    }
    os << ']';
    return os;
}

template <typename SDR_t>
void SDR<SDR_t>::do_benchark() {
    enum test_op {andb, ands, orb, ors, xorb, xors, andi, rm};
    static struct {
        inline void operator() (SDR a[], SDR b[], int num_sdr, std::string name, test_op op) const {
            auto start = std::chrono::high_resolution_clock::now();
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
            auto stop = std::chrono::high_resolution_clock::now();
            std::cout << name << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
        }
    } time_funct;

    constexpr static int num_sdr = 5;
    constexpr static SDR_t index_max = 100000;
    SDR a[num_sdr];
    SDR b[num_sdr];
    auto start = std::chrono::high_resolution_clock::now();
    
    for (auto& elem : a) {
        for (SDR_t i = 0; i < index_max; ++i) {
            elem += get_random_number() % index_max;
        }
    }
    for (auto& elem : b) {
        for (SDR_t i = 0; i < index_max; ++i) {
            elem += get_random_number() % index_max;
        }
    }
    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << "init: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << std::endl;
    std::cout << "sizes: " << a[0].size() << std::endl;
    time_funct(a, b, num_sdr, "andb", test_op::andb);
    time_funct(a, b, num_sdr, "ands", test_op::ands);
    time_funct(a, b, num_sdr, "orb", test_op::orb);
    time_funct(a, b, num_sdr, "ors", test_op::ors);
    time_funct(a, b, num_sdr, "xorb", test_op::xorb);
    time_funct(a, b, num_sdr, "xors", test_op::xors);
    time_funct(a, b, num_sdr, "andi", test_op::andi);
    time_funct(a, b, num_sdr, "rm", test_op::rm);
}