#pragma once

#include <assert.h>
#include <type_traits>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <math.h>
#include <stdbool.h>
using namespace std;

template<typename SDR_t = unsigned int, typename Container = std::set<SDR_t, less<SDR_t>>>
class SDR {
    public:
        SDR() {}
        SDR(initializer_list<SDR_t> l);
        SDR(const SDR<SDR_t>& sdr) : c{sdr.c} {}
        SDR(float input, unsigned int size, unsigned int underlying_array_length);
        SDR(float input, float period, unsigned int size, unsigned int underlying_array_length);

        // randomly turns off bits until it reaches the specified size amount. Returns this.
        SDR<SDR_t, Container>& sample(unsigned int amount);

        // and bit. returns the state of a bit.
        bool andb(SDR_t val) const;
        // and bits. returns the state of many bits.
        template<typename ArgContainer>
        SDR<SDR_t, Container> andb(const SDR<SDR_t, ArgContainer>& arg) const;
        // and bits. returns the state of many bits from start to stop.
        SDR<SDR_t, Container> andb(SDR_t start_inclusive, SDR_t stop_exclusive) const;
        // and inplace. turn off all bits not in arg (compute arg AND this, and place the result in this). Returns this.
        template<typename ArgContainer>
        SDR<SDR_t, Container>& andi(const SDR<SDR_t, ArgContainer>& arg);
        // and size. returns 0 if the bit is not contained in this, else 1.
        unsigned int ands(SDR_t val) const;
        // and size. returns the number of bits in both this and arg.
        template<typename ArgContainer>
        unsigned int ands(const SDR<SDR_t, ArgContainer>& arg) const;
        // and size. returns the number of bits from start to stop.
        unsigned int ands(SDR_t start_inclusive, SDR_t stop_exclusive) const;

        // or bits. 
        template<typename ArgContainer>
        SDR<SDR_t, Container> orb(const SDR<SDR_t, ArgContainer>& arg) const;
        // or inplace. turn on all bits in arg. Returns this.
        template<typename ArgContainer>
        SDR<SDR_t, Container>& ori(const SDR<SDR_t, ArgContainer>& arg);
        // or size. returns the number of bits in this or arg.
        template<typename ArgContainer>
        unsigned int ors(const SDR<SDR_t, ArgContainer>& arg) const;
        // xor bits. 
        template<typename ArgContainer>
        SDR<SDR_t, Container> xorb(const SDR<SDR_t, ArgContainer>& arg) const;
        // xor inplace. computes this xor arg, and places the result in this. Returns this.
        template<typename ArgContainer>
        SDR<SDR_t, Container>& xori(const SDR<SDR_t, ArgContainer>& arg);
        // xor size, aka hamming distance. returns the number of bits in this xor arg.
        template<typename ArgContainer>
        unsigned int xors(const SDR<SDR_t, ArgContainer>& arg) const;

        // turn off all bits in arg. Returns this.
        template<typename ArgContainer>
        SDR<SDR_t, Container>& rm(const SDR<SDR_t, ArgContainer>& arg);
        
        // Returns this.
        SDR<SDR_t, Container>& set(SDR_t index, bool value);
        // Returns this.
        template<typename ArgContainer>
        SDR<SDR_t, Container>& set(SDR<SDR_t, ArgContainer> arg, bool value);

        // Returns this, shifted by amount.
        SDR<SDR_t, Container>& shift(int amount);

        auto cbegin() const { return c.cbegin(); }
        auto cend() const { return c.cend(); }
        auto crbegin() const { return c.crbegin(); }
        auto crend() const { return c.crend(); }
        auto size() const { return c.size(); }

        friend ostream& operator<<(ostream& os, const SDR<SDR_t, Container>& sdr) {
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
        auto operator-(const other o) const { return SDR(*this).set(o, false); }
        template<typename other>
        auto operator%(const other o) const { return SDR(*this).sample(o); }
        template<typename other>
        auto operator%=(const other o) { return sample(o); }
        template<typename other>
        auto operator<<(const other o) const { return SDR(*this).shift(o); }
        template<typename other>
        auto operator>>(const other o) const { return SDR(*this).shift(-o); }
        template<typename other>
        auto operator<<=(const other o) { return shift(o); }
        template<typename other>
        auto operator>>=(const other o) { return shift(-o); }
    
    private:
        Container c;

        union SDROPResult {
            SDR<SDR_t, Container>* const sdr;
            unsigned int* const length;
        };

        // if r_pos is NULL, this indicates normal operation, and that the output is appended to r.
        // if r_pos is not NULL, this indicates that the operation is placing the result in one of it's operands, and r_pos indicates the overwrite position
        template <typename IT, typename ITA, typename ITB>
        static void sdrop_add_to_output(SDROPResult r, IT& r_pos, ITA a_pos, ITA a_end, ITB b_pos, ITB b_end, SDR_t elem, const bool size_only);
        // this is exclusively used in andop, merely because an internal operation is completed twice.
        template <typename IT, typename ITA, typename ITB>
        static bool sdrandop_funct(SDROPResult r, IT& r_pos, ITA& a_pos, const ITA a_end, ITB& b_pos, const ITB b_end, const bool size_only);
        // and operation. computes A & B, and places the result in r.
        template <typename ContainerA, typename ContainerB>
        static void andop(SDROPResult r, const SDR<SDR_t, ContainerA>* const a, const SDR<SDR_t, ContainerB>* const b, const bool size_only);
        // or operation. places the result in r.
        template <typename ContainerA, typename ContainerB>
        static void orop(SDROPResult r, const SDR<SDR_t, ContainerA>* const a, const SDR<SDR_t, ContainerB>* const b,  const bool size_only, const bool exclusive);

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
        template<typename A>
        struct is_vector<vector<SDR_t, A>> : true_type {};
        template<typename>
        struct is_list : false_type {};
        template<typename A>
        struct is_list<list<SDR_t, A>> : true_type {};
        template<typename>
        struct is_set : false_type {};
        template<typename A>
        struct is_set<std::set<SDR_t, less<SDR_t>, A>> : true_type {};
    
    static_assert(is_integral<SDR_t>::value, "SDR_t must be integral");
    static_assert(is_vector<Container>::value
               || is_list<Container>::value
               || is_set<Container>::value, "SDR's underlying container must hold type SDR_t, and be a vector, list, or set.");
};

template<typename SDR_t, typename Container>
SDR<SDR_t, Container>::SDR(initializer_list<SDR_t> l): c(l) {
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

template<typename SDR_t, typename Container>
SDR<SDR_t, Container>::SDR(float input, float period, unsigned int size, unsigned int underlying_array_length) {
    assert(size <= underlying_array_length && period != 0);
    if constexpr(is_vector<Container>::value) c.reserve(size);
    float progress = input / period;
    progress -= (int)progress;
    assert(progress >= 0);
    SDR_t start_index = roundf(progress * underlying_array_length);
    if (start_index + size > underlying_array_length) {
        SDR_t leading_indices = start_index + size - underlying_array_length;
        SDR_t non_leading_indice = underlying_array_length - leading_indices - 1;
        while (leading_indices > 0) {
            c.insert(c.end(), --leading_indices);
        }
        while (non_leading_indice < underlying_array_length) {
            c.insert(c.end(), non_leading_indice++);
        }
    } else {
        for (SDR_t i = 0; i < size; ++i) {
            c.insert(c.end(), start_index + i);
        }
    }
}

template<typename SDR_t, typename Container>
SDR<SDR_t, Container>::SDR(float input, unsigned int size, unsigned int underlying_array_length) {
    assert(size <= underlying_array_length);
    if constexpr(is_vector<Container>::value) c.reserve(size);
    SDR_t start_index = roundf((underlying_array_length - size) * input);
    for (SDR_t i = 0; i < size; ++i) {
        c.insert(c.end(), start_index + i);
    }
}

template<typename SDR_t, typename Container>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::sample(unsigned int amount) {
    if (amount > c.size()) return *this;
    if constexpr(is_set<Container>::value) {
        std::set<SDR_t> tmp;
        sample(c.cbegin(), c.cend(), back_inserter(tmp), amount, std::mt19937{std::random_device{}()});
        swap(c, tmp);
    } else {
        shuffle(c.begin(), c.end());
        c.resize(amount);
        sort(c.begin(), c.end());
    }
    return *this;
}

template<typename SDR_t, typename Container>
bool SDR<SDR_t, Container>::andb(SDR_t val) const {
    return lower_bound(cbegin(), cend(), val) == cend();
}

template<typename SDR_t, typename Container>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::set(SDR_t index, bool value) {
    auto it = lower_bound(cbegin(), cend(), index);
    if (value) {
        if (it == cend() || *it != index) c.insert(it, index);
    } else {
        if (it != cend() && *it == index) c.erase(it);
    }
    return *this;
}


template<typename SDR_t, typename Container>
template<typename IT, typename ITA, typename ITB>
void SDR<SDR_t, Container>::sdrop_add_to_output(SDROPResult r, IT& r_pos, ITA a_pos, ITA a_end, ITB b_pos, ITB b_end, SDR_t elem, const bool size_only) {
    if (size_only) {
        ++*r.length;
    } else {
        if (r_pos != (decltype(r.sdr->c.begin()))NULL) {
            // inplace will never happen if Container is a set. Just getting the compiler to cooperate
            if constexpr(!is_set<Container>::value) *r_pos++ = elem;
        } else {
            auto &rsdr = r.sdr->c;
            if constexpr(is_vector<Container>::value) {
                // allocate for half the max possible remaining elements
                if (rsdr.capacity() == rsdr.size()) {
                    unsigned int a_left = distance(a_pos, a_end);
                    unsigned int b_left = distance(b_pos, b_end);
                    unsigned int max_remaining = a_left < b_left ? a_left : b_left;
                    unsigned int cap_increase = (max_remaining + 1) / 2 + 1;
                    rsdr.reserve(rsdr.capacity() + cap_increase);
                }
            }
            rsdr.insert(rsdr.end(), elem);
        }
    }
}

template<typename SDR_t, typename Container>
template<typename IT, typename ITA, typename ITB>
bool SDR<SDR_t, Container>::sdrandop_funct(SDROPResult r, IT& r_pos, ITA& a_pos, const ITA a_end, ITB& b_pos, const ITB b_end, const bool size_only) {
    SDR_t a_elem = *a_pos++;
    b_pos = lower_bound(b_pos, b_end, a_elem);
    if (b_pos == b_end) return true; 
    SDR_t b_elem = *b_pos;
    if (a_elem == b_elem) {
        ++b_pos;
        sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_elem, size_only);
        if (b_pos == b_end) return true;
    }
    if (a_pos == a_end) return true;
    return false;
}

template<typename SDR_t, typename Container>
template<typename ContainerA, typename ContainerB>
void SDR<SDR_t, Container>::andop(SDROPResult r, const SDR<SDR_t, ContainerA>* const a, const SDR<SDR_t, ContainerB>* const b, const bool size_only) {
    // op is commutative over a and b. reduce 27 possible template specializations to 18
    constexpr bool swap_args = ({
        if constexpr(is_same<ContainerA, ContainerB>::value) false;
        if constexpr(is_same<Container, ContainerB>::value) true;
        if constexpr(is_same<Container, ContainerA>::value) false;
        if constexpr(is_list<Container>::value && is_vector<ContainerA>::value) true;
        if constexpr(is_vector<Container>::value && is_list<ContainerA>::value) true;
        if constexpr(is_set<Container>::value && is_list<ContainerA>::value) true;
        false;
    });
    if constexpr(swap_args) return andop(r, b, a, size_only);
    const bool is_inplace = ({
        if (!size_only) false;
        if constexpr(is_same<Container, ContainerA>::value) if (r.sdr == a) true;
        if constexpr(is_same<Container, ContainerB>::value) if (r.sdr == b) true;
        false;
    });
    assert(!is_inplace || !is_set<Container>::value);
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    auto r_pos = is_inplace ? r.sdr->c.begin() : (decltype(r.sdr->c.begin()))NULL;
    while (true) {       
        if (sdrandop_funct(r, r_pos, a_pos, a_end, b_pos, b_end, size_only)) break;
        if (sdrandop_funct(r, r_pos, b_pos, b_end, a_pos, a_end, size_only)) break;
    }
    // inplace will never happen if Container is a set. Just getting the compiler to cooperate
    if constexpr(!is_set<Container>::value) if (is_inplace) r.sdr->c.resize(distance(r.sdr->c.begin(), r_pos));
    if constexpr(is_vector<Container>::value) {
        if (!size_only) {
            r.sdr->c.shrink_to_fit();
        }
    }
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container> SDR<SDR_t, Container>::andb(const SDR<SDR_t, ArgContainer>& arg) const {
    SDR r;
    SDROPResult rop{.sdr=&r};
    andop(rop, this, &arg, false);
    return r; // nrvo 
}

template<typename SDR_t, typename Container>
SDR<SDR_t, Container> SDR<SDR_t, Container>::andb(SDR_t start_inclusive, SDR_t stop_exclusive) const {
    SDR sdr;
    auto start_it = lower_bound(cbegin(), cend(), start_inclusive);
    auto end_it = lower_bound(start_it, cend(), stop_exclusive);
    if constexpr(is_vector<Container>::value) sdr.c.reserve(distance(start_it, end_it));
    copy(start_it, end_it, back_inserter(sdr.v));
    return sdr; // nrvo
}

template<typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::andi(const SDR<SDR_t, ArgContainer>& arg) {
    if constexpr(!is_set<Container>::value) {
        SDROPResult rop{.sdr = this};
        andop(rop, this, &arg, false);
    } else {
        auto this_pos = c.begin();
        auto this_end = c.end();
        auto arg_pos = arg.cbegin();
        auto arg_end = arg.cend();
        while (this_pos != this_end) {
            SDR_t this_elem = *this_pos;
            cout << this_elem;
            auto search_pos = lower_bound(arg_pos, arg_end, this_elem);
            if (search_pos != arg_end && *search_pos == this_elem) {
                ++this_pos;
            } else {
                c.erase(this_pos++);
                arg_pos = search_pos;
            }
        }
    }
    return *this;
}

template <typename SDR_t, typename Container>
unsigned int SDR<SDR_t, Container>::ands(SDR_t val) const {
    return andb(val) ? 1 : 0;
};

template <typename SDR_t, typename Container>
template<typename ArgContainer>
unsigned int SDR<SDR_t, Container>::ands(const SDR<SDR_t, ArgContainer>& arg) const {
    unsigned int r = 0;
    SDROPResult rop;
    rop.length = &r;
    andop(rop, this, &arg, true);
    return r;
}


template<typename SDR_t, typename Container>
unsigned int SDR<SDR_t, Container>::ands(SDR_t start_inclusive, SDR_t stop_exclusive) const {
    SDR sdr;
    auto end = cend();
    auto pos = lower_bound(cbegin(), end, start_inclusive);
    if constexpr(is_vector<Container>::value) {
        auto end_it = lower_bound(pos, end, stop_exclusive);
        return (unsigned int)(distance(pos, end_it));
    } else {
        unsigned int i = 0;
        while (pos != end) {
            ++i;
            ++pos;
        }
        return i;
    }
}

template <typename SDR_t, typename Container>
template <typename ContainerA, typename ContainerB>
void SDR<SDR_t, Container>::orop(SDROPResult r, const SDR<SDR_t, ContainerA>* const a, const SDR<SDR_t, ContainerB>* const b, const bool size_only, const bool exclusive) {
    // op is commutative over a and b. reduce 27 possible template specializations to 18
    constexpr bool swap_args = ({
        if constexpr(is_same<ContainerA, ContainerB>::value) false;
        if constexpr(is_same<Container, ContainerB>::value) true;
        if constexpr(is_same<Container, ContainerA>::value) false;
        if constexpr(is_list<Container>::value && is_vector<ContainerA>::value) true;
        if constexpr(is_vector<Container>::value && is_list<ContainerA>::value) true;
        if constexpr(is_set<Container>::value && is_list<ContainerA>::value) true;
        false;
    });
    if constexpr(swap_args) return orop(r, b, a, size_only, exclusive);
    assert(({
        if (!size_only) true;
        if constexpr(is_same<Container, ContainerA>::value) if (r.sdr == a) false;
        if constexpr(is_same<Container, ContainerB>::value) if (r.sdr == b) false;
        true;
    }));
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    bool a_valid = true;
    SDR_t a_val;
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    bool b_valid = true;
    SDR_t b_val;
    auto r_pos = (decltype(typename Container::iterator()))NULL;
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
    if constexpr(is_vector<Container>::value) {
        if (!size_only) {
            r.sdr->v.shrink_to_fit();
        }
    }
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container> SDR<SDR_t, Container>::orb(const SDR<SDR_t, ArgContainer>& arg) const {
    SDR r;
    SDROPResult rop{.sdr=&r};
    orop(rop, this, &arg, false, false);
    return r; // nrvo 
}


template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::ori(const SDR<SDR_t, ArgContainer>& arg) {
    SDR r;
    SDROPResult rop{.sdr=&r};
    orop(rop, this, &arg, false, false);
    swap(r.c, c);
    return *this;
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
unsigned int SDR<SDR_t, Container>::ors(const SDR<SDR_t, ArgContainer>& arg) const {
    unsigned int r = 0;
    SDROPResult rop{.length = &r};
    orop(rop, this, &arg, true, false);
    return r;
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container> SDR<SDR_t, Container>::xorb(const SDR<SDR_t, ArgContainer>& arg) const {
    SDR r;
    SDROPResult rop{.sdr = &r};
    orop(rop, this, &arg, true);
    return r; // nrvo
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::xori(const SDR<SDR_t, ArgContainer>& arg) {
    SDR r;
    SDROPResult rop{.sdr = &r};
    orop(rop, this, &arg, false, true);
    swap(r.c, c);
    return *this;
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
unsigned int SDR<SDR_t, Container>::xors(const SDR<SDR_t, ArgContainer>& arg) const {
    unsigned int r = 0;
    SDROPResult rop{.length = &r};
    orop(rop, this, &arg, true, true);
    return r;
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::rm(const SDR<SDR_t, ArgContainer>& arg) {
    auto arg_pos = arg.crbegin();
    auto arg_end = arg.crend();
    SDR_t arg_val;
    auto this_start = c.begin();
    auto this_end = c.end();
    SDR_t this_val;
    while (arg_pos != arg_end) {
        arg_val = *arg_pos++;
        auto new_this_end = lower_bound(this_start, this_end, arg_val);
        if (new_this_end == this_end) continue;
        if (arg_val == *new_this_end){
            this_end = prev(new_this_end);
            c.erase(new_this_end);
            this_end = next(this_end);
        } else {
            this_end = new_this_end;
        }
    }
    if constexpr(is_vector<Container>::value) c.shrink_to_fit();
    return *this;
}

template <typename SDR_t, typename Container>
template<typename ArgContainer>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::set(SDR<SDR_t, ArgContainer> arg, bool value) {
    if (value) {
        ori(arg);
    } else {
        rm(arg);
    }
    return *this;
}

template <typename SDR_t, typename Container>
SDR<SDR_t, Container>& SDR<SDR_t, Container>::shift(int amount) {
    for (auto& elem : c) {
        #ifdef NDEBUG
        elem += amount;
        #else
        assert(!__builtin_add_overflow(amount, elem, &elem));
        #endif
    }
    return *this;
}
