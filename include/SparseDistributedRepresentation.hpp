#pragma once

#include <unistd.h>
#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <list>
#include <vector>
#include <set>
#include <iostream>

namespace {

template <typename T>
struct isVector : std::false_type {};

template <typename T, typename A>
struct isVector<std::vector<T, A>> : std::true_type {};

template <typename T>
struct isList : std::false_type {};

template <typename T, typename A>
struct isList<std::list<T, A>> : std::true_type {};

template <typename T>
struct isSet : std::false_type {};

template <typename T, typename C, typename A>
struct isSet<std::set<T, C, A>> : std::true_type {};
}

/*
 * Based off the ideas explained in this series:
 * https://youtu.be/ZDgCdWTuIzc
 * Numenta: SDR Capacity & Comparison (Episode 2)
 */
template<typename SDR_t = unsigned int, typename container_t = std::vector<SDR_t>>
class SDR {
    public:
        using index_type = SDR_t;
        using size_type = typename container_t::size_type;
        using const_iterator = typename container_t::const_iterator;
        using iterator = typename container_t::iterator;

        // rule of 5
        SDR() {}
        SDR(const SDR& sdr): v(sdr.v) {}
        SDR& operator=(const SDR& sdr) { this->v = sdr.v; return *this; }
        SDR(SDR&& sdr): v(std::move(sdr.v)) {}
        SDR& operator=(SDR&& sdr) { this->v = std::move(sdr.v); return *this; }

        // constructors from underlying container
        SDR(const container_t& v): v(v) { assert_ascending(); }

        SDR& operator=(const container_t& v) {
            this->v = v;
            assert_ascending();
            return *this;
        }

        SDR(container_t&& v): v(v) { assert_ascending(); }

        SDR& operator=(container_t&& v) {
            this->v = v;
            assert_ascending();
            return *this;
        }

        container_t&& data() { return std::move(v); }

        SDR(std::initializer_list<SDR_t> list) : v(list) { assert_ascending(); };

        // Encode a float as an SDR.
        // @param input the float to encode. Should be from 0 to 1 inclusively. Must be non-negative.
        // @param size the size of the instantiated SDR result.
        // @param underlying_array_length the size of the dense array being represented.
        SDR(float input, size_type size, size_type underlying_array_length);

        // Encode a float as an SDR.
        // @param input the float to encode. Must be non-negative.
        // @param period encodes the input such that it wraps back to 0 as it approaches a multiple of the period. Must be non-negative.
        // @param size the size of the instantiated SDR result.
        // @param underlying_array_length the size of the dense array being represented.
        SDR(float input, float period, size_type size, size_type underlying_array_length);

        // Turns off bits such that the length matches the specified amount.
        SDR<SDR_t, container_t>& sample_length(size_type amount);

        // Each bit has a chance of being turned off, specified by amount, where 0 nearly always clears the sdr, and 1 leaves it unchanged.
        SDR<SDR_t, container_t>& sample_portion(float amount);

        // and bit. returns the state of a bit.
        template<typename arg_t>
        bool andb(arg_t val) const;

        // and bits. returns the state of many bits.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t> andb(const SDR<arg_t, c_arg_t>& arg) const;

        // and bits. returns the state of many bits from start to stop.
        template<typename arg_t>
        SDR<SDR_t, container_t> andb(arg_t start_inclusive, arg_t stop_exclusive) const;

        // and inplace. turn off all bits not in arg (compute arg AND this, and place the result in this). Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& andi(const SDR<arg_t, c_arg_t>& arg);

        // and size. returns 0 if the bit is not contained in this, else 1.
        template<typename arg_t>
        size_type ands(arg_t val) const;

        // and size. returns the number of bits in both this and arg.
        template<typename arg_t, typename c_arg_t>
        size_type ands(const SDR<arg_t, c_arg_t>& arg) const;

        // and size. returns the number of bits from start to stop.
        template<typename arg_t>
        size_type ands(arg_t start_inclusive, arg_t stop_exclusive) const;
        
        /**
         * and visitor. Perform an operation on all elements based on a query.
         * each element in this which is found in the query is called as the argument to the visitor as visitor(SDR_t&).
         * ensure that the visitor does not modify elements in a way that would put the sdr in an invalid state,
         * e.g. elements are no longer ascending in a vector based sdr.
         */
        template<typename arg_t, typename c_arg_t, typename Visitor>
        void andv(const SDR<arg_t, c_arg_t>& query, Visitor visitor);
        
        // or bits.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t> orb(const SDR<arg_t, c_arg_t>& arg) const;

        // or inplace. turn on all bits in arg. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& ori(const SDR<arg_t, c_arg_t>& arg);

        // or size. returns the number of bits in this or arg.
        template<typename arg_t, typename c_arg_t>
        size_type ors(const SDR<arg_t, c_arg_t>& arg) const;

        // xor bits.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t> xorb(const SDR<arg_t, c_arg_t>& arg) const;

        // xor inplace. computes this xor arg, and places the result in this. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& xori(const SDR<arg_t, c_arg_t>& arg);

        // xor size, aka hamming distance. returns the number of bits in this xor arg.
        template<typename arg_t, typename c_arg_t>
        size_type xors(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * xor visitor. Perform an operation on all elements based on a query.
         * each element in this which is found in the query is called as the argument to the visitor as visitora(SDR_t&) or visitorb(arg_t&).
         * ensure that the visitor does not modify elements in a way that would put the sdr in an invalid state,
         * e.g. elements are no longer ascending in a vector based sdr.
         */
        template<typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB>
        void xorv(const SDR<arg_t, c_arg_t>& query, VisitorA visitora, VisitorB visitorb);

        // Returns a copy of this which lacks any bit from arg.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t> rmb(const SDR<arg_t, c_arg_t>& arg) const;

        // Remove inplace. Remove all bits in arg from this, then returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& rmi(const SDR<arg_t, c_arg_t>& arg);

        // Returns the number of elements in this that are not in arg.
        template<typename arg_t, typename c_arg_t>
        size_type rms(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * rm visitor. Perform an operation on all elements based on a query.
         * each element in this which is NOT found in the query is called as the argument to the visitor.
         * ensure that the visitor does not modify elements in a way that would put the sdr in an invalid state,
         * e.g. elements are no longer ascending in a vector based sdr.
         */
        template<typename arg_t, typename c_arg_t, typename Visitor>
        void rmv(const SDR<arg_t, c_arg_t>& query, Visitor visitor);
        
        // Sets bit in this, then returns this.
        SDR<SDR_t, container_t>& set(SDR_t index, bool value);

        // Sets bits in this, then returns this.
        SDR<SDR_t, container_t>& set(SDR<SDR_t, container_t> arg, bool value);

        // Returns this, shifted by amount.
        SDR<SDR_t, container_t>& shift(int amount);

        // concatenate an SDR to an SDR. Every indice in arg must be greater than every indice in this. Returns this.
        SDR<SDR_t, container_t>& join(const SDR<SDR_t, container_t>& arg);

        auto cbegin() const { return v.cbegin(); }
        auto cend() const { return v.cend(); }
        auto crbegin() const { return v.crbegin(); }
        auto crend() const { return v.crend(); }
        auto size() const { return v.size(); }
        void push_back(SDR_t i) { assert(size() == 0 || *v.crbegin() < i); v.insert(v.end(), i); }

        template<typename SDR_t_inner, typename container_t_inner>
        friend std::ostream& operator<<(std::ostream& os, const SDR<SDR_t_inner, container_t_inner>& sdr);

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
        auto operator/(const other& o) const { return xors(o); } // should be ^^
        template<typename other>
        auto operator^=(const other& o) { return xori(o); }
        template<typename other>
        auto operator+(const other& o) const { return SDR(*this).set(o, true); }
        template<typename other>
        auto operator+=(const other& o) { return set(o, true); }
        template<typename other>
        auto operator-(const other& o) const { return rmb(o); }
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
        auto operator==(const SDR<SDR_t, container_t>& other) const { return this->v == other.v; }
        auto operator!=(const SDR<SDR_t, container_t>& other) const { return !(*this == other); }
        auto operator<(const SDR<SDR_t, container_t>& other) const {
            return std::lexicographical_compare(this->v.crbegin(), this->v.crend(), other.v.crbegin(), other.v.crend());
        }
        auto operator>=(const SDR<SDR_t, container_t>& other) const { return !(*this < other); }
        auto operator>(const SDR<SDR_t, container_t>& other) const { return other < *this; }
        auto operator<=(const SDR<SDR_t, container_t>& other) const { return !(*this > other); }

        // static ref to mersenne twister with result type SDR_t
        static auto& get_twister() {
            // same as mt19937
            static std::mersenne_twister_engine<SDR_t,32,624,397,31,0x9908b0df,11,0xffffffff,7,0x9d2c5680,15,0xefc60000,18,1812433253> twister(time(NULL) * getpid());
            return twister;
        }

        // uses the mersenne twister, SDR<SDR_t, container_t>::get_twister()
        static SDR_t get_random_number() {
            return get_twister()();
        }

    private:
        container_t v;

        void assert_ascending();

        // and operation. computes A & B. each selected element is called in the visitor as visitor(SDR_t&)
        template <typename arg_t, typename c_arg_t, typename Visitor>
        static void andop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, Visitor visitor);

        // or operation. each selected element from a is called to visitor a as visitora(SDR_t&), and from b is visitorb(arg_t&)
        template <typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, bool exclusive>
        static void orop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, VisitorA visitora, VisitorB visitorb);

        // rm operation. each selected element is called in visitor(SDR_t&)
        // the trailing elements start position is called in DumpEnd(const_iterator)
        template <typename arg_t, typename c_arg_t, typename Visitor, typename DumpEnd>
        static void rmop(const SDR<SDR_t, container_t>& me, const SDR<arg_t, c_arg_t>& arg, Visitor visitor, DumpEnd dumpEnd);

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

        static constexpr bool usesVector = isVector<container_t>::value;
        static constexpr bool usesList = isList<container_t>::value;
        static constexpr bool usesSet = isSet<container_t>::value;
        static_assert(usesVector || usesList || usesSet);

        // used in the output stream op
        static constexpr bool print_type = false;

        // set this to true if it is expected that arguments will be small compared to the object being called on
        // e.g. large.andb(small)
        static constexpr bool small_args = false;

        template<typename friend_SDR_t, typename friend_container_t>
        friend class SDR;
};

template<typename SDR_t, typename container_t>
void SDR<SDR_t, container_t>::assert_ascending() {
    if constexpr(!usesSet) {
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
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>::SDR(float input, float period, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length && period >= 0 && input >= 0);
    if constexpr(usesVector) v.reserve(size);

    float progress = input / period;
    progress -= (int)progress;
    SDR_t start_index = std::round(progress * underlying_array_length);

    if (start_index + size > underlying_array_length) {
        // if elements would go off the end of the array, wrap them back to the start

        // the number of elements that wrap off the end
        SDR_t wrapped_elements = start_index + size - underlying_array_length;
        
        // the number of elements that don't wrap off the end
        SDR_t non_wrapped_elements = size - wrapped_elements;

        for (SDR_t i = 0; i < (SDR_t)wrapped_elements; ++i) v.insert(v.end(), i);
        for (SDR_t i = 0; i < (SDR_t)non_wrapped_elements; ++i) v.insert(v.end(), start_index + i);
    } else {
        // no elements are wrapped from the end
        for (SDR_t i = 0; i < (SDR_t)size; ++i) v.insert(v.end(), start_index + i);
    }
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>::SDR(float input, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length);
    assert(input >= 0);
    if constexpr(usesVector) v.reserve(size);
    SDR_t start_index = std::round((underlying_array_length - size) * input);
    for (SDR_t i = 0; i < (SDR_t)size; ++i) {
        v.insert(v.end(), start_index + i);
    }
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::sample_length(size_type amount) {
    container_t temp;
    if constexpr(usesVector) temp.reserve(amount);
    sample(cbegin(), cend(), back_inserter(temp), amount, get_twister());
    swap(temp, v);
    return *this;
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::sample_portion(float amount) {
    assert(amount >= 0 && amount <= 1);
    SDR_t check_val = amount * (get_twister().max() / 2);
    if constexpr(usesSet) {
        for (auto pos = v.begin(); pos != v.end(); ++pos) {
            if ((get_twister()() / 2) < check_val) {
                v.erase(pos);
            }
        }
    } else {
        auto to_offset = v.begin();
        auto from_offset = cbegin();
        auto end = v.end();
        while (from_offset != end) {
            if ((get_twister()() / 2) != check_val) {
                *to_offset++ = *from_offset;
            }
            from_offset++;
        }
        v.resize(distance(cbegin() - from_offset));
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t>
bool SDR<SDR_t, container_t>::andb(arg_t val) const {
    decltype(lower_bound(cbegin(), cend(), val)) pos;
    if constexpr(usesSet) {
        pos = v.lower_bound(val);
    } else {
        pos = lower_bound(cbegin(), cend(), val);
    }
    return pos != v.end() && *pos == val;
}

template<typename SDR_t, typename container_t>
template<typename arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::andb(arg_t start_inclusive, arg_t stop_exclusive) const {
    assert(start_inclusive <= stop_exclusive);
    SDR sdr;
    decltype(lower_bound(cbegin(), cend(), start_inclusive)) start_it;
    if constexpr(!usesSet) {
        start_it = lower_bound(cbegin(), cend(), start_inclusive);
    } else {
        start_it = v.lower_bound(start_inclusive);
    }
    decltype(start_it) end_it;
    if constexpr(!usesSet) {
        end_it = lower_bound(start_it, cend(), stop_exclusive);
    } else {
        end_it = v.lower_bound(stop_exclusive);
    }
    if constexpr(usesVector) {
        sdr.v.reserve(distance(start_it, end_it));
    }
    for (auto it = start_it; it != end_it; ++it) {
        sdr.v.insert(sdr.v.end(), *it);
    }
    return sdr; // nrvo
}

template<typename SDR_t, typename container_t>
template<typename arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ands(arg_t start_inclusive, arg_t stop_exclusive) const {
    SDR sdr;
    if (usesSet) {
        auto pos = v.lower_bound(start_inclusive);
        size_type count = 0;
        while (pos != v.cend()) {
            if (*pos == stop_exclusive) {
                return count;
            }
            ++count;
            ++pos;
        }
        return count;
    } else {
        auto pos = lower_bound(cbegin(), cend(), start_inclusive);
        auto end_it = lower_bound(pos, cend(), stop_exclusive);
        return (size_type)(pos - end_it);
    }
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::set(SDR_t index, bool value) {
    decltype(lower_bound(cbegin(), cend(), value)) it;
    if constexpr(!usesSet) {
        it = lower_bound(cbegin(), cend(), value);
    } else {
        it = v.lower_bound(value);
    }
    if (value) {
        if (it == v.end() || *it != index) v.insert(it, index);
    } else {
        if (it != v.end() && *it == index) v.erase(it);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template <typename arg_t, typename c_arg_t, typename Visitor>
void SDR<SDR_t, container_t>::andop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, Visitor visitor) {
    auto a_pos = a.cbegin();
    auto a_end = a.cend();
    auto b_pos = b.cbegin();
    auto b_end = b.cend();
    SDR_t a_elem;
    [[maybe_unused]] arg_t b_elem;

    if (a_pos == a_end) return;
    while (true) {
        a_elem = *a_pos;
        if constexpr(isSet<c_arg_t>::value) {
            b_pos = b.v.lower_bound(a_elem);
        } else {
            b_pos = lower_bound(b_pos, b_end, a_elem);
        }
        if (b_pos == b_end) return;
        if (*b_pos == a_elem) {
            visitor(*const_cast<SDR_t*>(&*a_pos));
            ++b_pos;
            if (b_pos == b_end) return;
        }
        ++a_pos;
        // ============= a and b swapped ===^=V================
        if constexpr (!small_args) {
            b_elem = *b_pos;
            if constexpr(usesSet) {
                a_pos = a.v.lower_bound((SDR_t)b_elem);
            } else {
                a_pos = lower_bound(a_pos, a_end, b_elem);
            }
            if (a_pos == a_end) return;
            a_elem = *a_pos;
            if (a_elem == b_elem) {
                visitor(*const_cast<SDR_t*>(&*a_pos));
                ++a_pos;
                if (a_pos == a_end) return;
            }
            ++b_pos;
        } else {
            if (a_pos == a_end) return;
        }
    }
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::andb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR r;
    auto visitor = [&](SDR_t& elem) {
        r.push_back(elem);
    };
    andop(*this, arg, visitor);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::andi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr (usesSet) {
        SDR r = andb(arg);
        swap(r.v, this->v);
    } else {
        [[maybe_unused]] size_type i = 0;
        auto pos = this->v.begin();
        auto visitor = [&]([[maybe_unused]] SDR_t& elem) {
            *pos++ = elem;
            if constexpr(usesList) { ++i; };
        };
        andop(*this, arg, visitor);
        if constexpr(usesList) {
            this->v.resize(i);
        } else {
            this->v.resize(pos - this->v.begin());
        }
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ands(arg_t val) const {
    return andb(val) ? 1 : 0;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ands(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&]([[maybe_unused]] SDR_t& elem) {
        ++r;
    };
    andop(*this, arg, visitor);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename Visitor>
void SDR<SDR_t, container_t>::andv(const SDR<arg_t, c_arg_t>& query, Visitor visitor) {
    andop(*this, query, visitor);
}

template<typename SDR_t, typename container_t>
template <typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, bool exclusive>
void SDR<SDR_t, container_t>::orop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, VisitorA visitora, VisitorB visitorb) {
    auto a_pos = a.cbegin();
    auto a_end = a.cend();
    bool a_valid = true;
    SDR_t a_val;
    auto b_pos = b.cbegin();
    auto b_end = b.cend();
    bool b_valid = true;
    arg_t b_val;

    if (a_pos != a_end) a_val = *a_pos; else a_valid = false; // get from a, or update a_valid if no more elements
    if (b_pos != b_end) b_val = *b_pos; else b_valid = false; // b
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    while (a_valid || b_valid) {
        if ((a_valid && !b_valid) || (a_valid && b_valid && a_val < b_val)) {
            visitora(*const_cast<SDR_t*>(&*a_pos));
            ++a_pos;
            if (a_pos != a_end) a_val = *a_pos; else a_valid = false; // a
        } else if ((!a_valid && b_valid) || (a_valid && b_valid && a_val > b_val)) {
            visitorb(*const_cast<SDR_t*>(&*b_pos));
            ++b_pos;
            if (b_pos != b_end) b_val = *b_pos; else b_valid = false; // b
        } else {
            if constexpr(!exclusive) {
                visitora(*const_cast<SDR_t*>(&*a_pos));
            }
            ++a_pos;
            ++b_pos;
            if (a_pos != a_end) a_val = *a_pos; else a_valid = false; // a
            if (b_pos != b_end) b_val = *b_pos; else b_valid = false; // b
        }
    }
    #pragma GCC diagnostic pop
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::orb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR r;
    auto visitor = [&](SDR_t& elem) {
        r.push_back(elem);
    };
    orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), false>(*this, arg, visitor, visitor);
    if constexpr(usesVector) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::ori(const SDR<arg_t, c_arg_t>& arg) {
    SDR r = orb(arg);
    swap(r.v, this->v);
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&]([[maybe_unused]] SDR_t& elem) {
        ++r;
    };
    orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), false>(*this, arg, visitor, visitor);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::xorb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR r;
    auto visitor = [&](SDR_t& elem) {
        r.push_back(elem);
    };
    orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), true>(*this, arg, visitor, visitor);
    if constexpr(usesVector) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::xori(const SDR<arg_t, c_arg_t>& arg) {
    SDR r = xorb(arg);
    swap(r.v, this->v);
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::xors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&]([[maybe_unused]] SDR_t& elem) {
        ++r;
    };
    orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), true>(*this, arg, visitor, visitor);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB>
void SDR<SDR_t, container_t>::xorv(const SDR<arg_t, c_arg_t>& query, VisitorA visitora, VisitorB visitorb) {
    orop<arg_t, c_arg_t, VisitorA, VisitorB, true>(*this, query, visitora, visitorb);
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::rmi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesSet) {
        // revert back to normal remove, with a swap 
        SDR r = this->rmb(arg);
        swap(r.v, this->v);
    } else {
        auto const_type_helper = [arg]() {
            if constexpr(usesVector) {
                return arg.crbegin();
            } else {
                return arg.cbegin();
            }
        };

        auto type_helper = [this]() {
            if constexpr(usesVector) {
                return v.rbegin();
            } else {
                return v.begin();
            }
        };

        decltype(const_type_helper()) arg_pos;
        decltype(const_type_helper()) arg_end;
        decltype(type_helper()) this_pos;
        decltype(type_helper()) this_end;

        if constexpr (usesVector) {
            // for a vector, it's best to remove from the end first
            arg_pos = arg.crbegin();
            arg_end = arg.crend();
            this_pos = v.rbegin();
            this_end = v.rend();
        } else {
            // for a list, removing with a reverse iterator can get messy
            arg_pos = arg.cbegin();
            arg_end = arg.cend();
            this_pos = v.begin();
            this_end = v.end();
        }

        if (arg_pos == arg_end) goto end;
        while (true) {
            arg_t elem = *arg_pos++;
            if constexpr (usesVector) {
                this_pos = lower_bound(this_pos, this_end, elem, std::greater<arg_t>());
            } else {
                this_pos = lower_bound(this_pos, this_end, elem);
            }
            if (this_pos == this_end) goto end;
            if (*this_pos == elem) {
                if constexpr(usesVector) {
                    v.erase((++this_pos).base());
                } else {
                    this_pos = v.erase(this_pos);
                }
                if (this_pos == this_end) goto end;
                if constexpr(usesVector) {
                    if (this_end != v.rend()) {
                        // revalidate iterators
                        this_pos = v.rend() - (this_end - this_pos);
                        this_end = v.rend();
                    }
                }
                elem = *this_pos;
            }
            // =====
            if constexpr (!small_args) {
                if constexpr (usesVector) {
                    arg_pos = lower_bound(arg_pos, arg_end, elem, std::greater<arg_t>());
                } else {
                    arg_pos = lower_bound(arg_pos, arg_end, elem);
                }
                if (arg_pos == arg_end) goto end;
                if (*arg_pos == elem) {
                    if constexpr(usesVector) {
                        v.erase((++this_pos).base());
                    } else {
                        this_pos = v.erase(this_pos);
                    }
                    if (this_pos == this_end) goto end;
                    if constexpr(usesVector) {
                        if (this_end != v.rend()) {
                            // revalidate iterators
                            this_pos = v.rend() - (this_end - this_pos);
                            this_end = v.rend();
                        }
                    }
                }
            } else {
                if (arg_pos == arg_end) goto end;
            }
        }
        end:
        if constexpr(usesVector)
            v.shrink_to_fit();
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template <typename arg_t, typename c_arg_t, typename Visitor, typename DumpEnd>
void SDR<SDR_t, container_t>::rmop(const SDR<SDR_t, container_t>& me, const SDR<arg_t, c_arg_t>& arg, Visitor visitor, DumpEnd dump_end) {
    // this function can't place the result in the operands! see rmi instead
    SDR<SDR_t, container_t> ret;
    auto arg_pos = arg.cbegin();
    auto arg_end = arg.cend();
    auto this_pos = me.cbegin();
    auto this_end = me.cend();
    arg_t arg_elem;
    SDR_t this_elem;

    auto get_next_arg = [&]() -> bool {
        if constexpr (!small_args) {
            arg_pos = std::lower_bound(arg_pos, arg_end, this_elem);
        }
        if (arg_pos == arg_end) return false;
        arg_elem = *arg_pos++;
        return true;
    };

    if (arg_pos == arg_end) goto dump_remaining_this;
    arg_elem = *arg_pos++;

    if (this_pos == this_end) return;
    this_elem = *this_pos;

    while (true) {
        if (this_elem < arg_elem) {
            visitor(*const_cast<SDR_t*>(&*this_pos));
            ++this_pos;
            if (this_pos == this_end) return;
            this_elem = *this_pos;
        } else if (this_elem == arg_elem) {
            ++this_pos;
            if (!get_next_arg()) goto dump_remaining_this;
            if (this_pos == this_end) return;
            this_elem = *this_pos;
        } else {
            if (!get_next_arg()) {
                visitor(*const_cast<SDR_t*>(&*this_pos));
                ++this_pos;
                if (this_pos == this_end) return;
            }
        }
    }
    dump_remaining_this:
    dump_end(this_pos);
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::rmb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR r;
    auto visitor = [&](SDR_t& elem) {
        r.push_back(elem);
    };
    auto dump_end = [&](const_iterator pos) {
        while (pos != this->cend()) {
            visitor(*const_cast<SDR_t*>(&*pos++));
        }
        if constexpr (usesVector) {
            r.v.shrink_to_fit();
        }
    };
    rmop(*this, arg, visitor, dump_end);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&]([[maybe_unused]] SDR_t& elem) {
        ++r;
    };
    auto dump_end = [&](const_iterator pos) {
        if constexpr (usesVector) {
            r += this->cend() - pos;
        } else {
            while (pos != this->cend()) {
                ++r;
                ++pos;
            }
        }
    };
    rmop(*this, arg, visitor, dump_end);
    return r; // nrvo
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename Visitor>
void SDR<SDR_t, container_t>::rmv(const SDR<arg_t, c_arg_t>& query, Visitor visitor) {
    auto dump_end = [&](const_iterator pos) {
        while (pos != this->cend()) {
            visitor(*const_cast<SDR_t*>(&*pos++));
        }
    };
    rmop(*this, query, visitor, dump_end);
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::set(SDR<SDR_t, container_t> arg, bool value) {
    if (value) {
        return ori(arg);
    } else {
        return rmi(arg);
    }
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::shift(int amount) {
    for (auto& elem : v) {
        #ifdef NDEBUG
        elem += amount;
        #else
        assert(!__builtin_add_overflow(amount, elem, &elem));
        #endif
    }
    return *this;
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::join(const SDR<SDR_t, container_t>& arg) {
    assert(size() == 0 || arg.size() == 0 || *v.crbegin() < *arg.v.cbegin());
    v.reserve(v.size() + arg.v.size());
    for (auto e : arg.v) { push_back(e); }
    return *this;
}

template<typename SDR_t, typename container_t>
std::ostream& operator<<(std::ostream& os, const SDR<SDR_t, container_t>& sdr) {
    if constexpr(SDR<SDR_t, container_t>::print_type) {
        static constexpr typename SDR<SDR_t, container_t>::FormatText beginning;
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
