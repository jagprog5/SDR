#pragma once

#include <unistd.h>
#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <functional>
#include <list>
#include <vector>
#include <set>
#include <iostream>
#include <chrono>

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

        SDR() {}
        SDR(container_t&& v);
        SDR<SDR_t, container_t>& operator=(container_t&& v);
        SDR(std::initializer_list<SDR_t> list);

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
         * and modify. Perform an operation on all elements based on a query.
         * each element in this which is found in the query is called as the argument to the visitor.
         * ensure that the visitor does not modify elements in a way that would put the sdr in an invalid state,
         * e.g. elements are no longer ascending in a vector based sdr.
         */
        template<typename arg_t, typename c_arg_t>
        void andm(const SDR<arg_t, c_arg_t>& query, std::function<void(SDR_t&)>& visitor);
        
        // or bits. 
        SDR<SDR_t, container_t> orb(const SDR<SDR_t, container_t>& arg) const;

        // or inplace. turn on all bits in arg. Returns this.
        SDR<SDR_t, container_t>& ori(const SDR<SDR_t, container_t>& arg);

        // or size. returns the number of bits in this or arg.
        size_type ors(const SDR<SDR_t, container_t>& arg) const;

        // xor bits. 
        SDR<SDR_t, container_t> xorb(const SDR<SDR_t, container_t>& arg) const;

        // xor inplace. computes this xor arg, and places the result in this. Returns this.
        SDR<SDR_t, container_t>& xori(const SDR<SDR_t, container_t>& arg);

        // xor size, aka hamming distance. returns the number of bits in this xor arg.
        size_type xors(const SDR<SDR_t, container_t>& arg) const;

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
         * rm modify. Perform an operation on all elements based on a query.
         * each element in this which is NOT found in the query is called as the argument to the visitor.
         * ensure that the visitor does not modify elements in a way that would put the sdr in an invalid state,
         * e.g. elements are no longer ascending in a vector based sdr.
         */
        template<typename arg_t, typename c_arg_t>
        void rmm(const SDR<arg_t, c_arg_t>& query, std::function<void(SDR_t&)>& visitor);
        
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

        union SDROPResult {
            SDR<SDR_t, container_t>* sdr;
            size_type* length;
            std::function<void(SDR_t&)>* visitor;
        };

        // if r_pos is NULL, this indicates normal operation, and that the output is appended to r.
        // if r_pos is not NULL, this indicates that the operation is placing the result in one of it's operands, and r_pos indicates the overwrite position
        // this function happens to be used only in andop and orop. Select an ideal allocation strategy for andop by setting and_op param to true, else false.
        template <typename CIT, typename AIT, typename BIT>
        static void sdrop_add_to_output(SDROPResult r, CIT& r_pos, AIT a_pos, AIT a_end, BIT b_pos, BIT b_end, SDR_t elem, bool size_only = false, bool and_op = false);
        // and operation. computes A & B, and places the result in r (or alternatively uses the visitor).
        template <typename arg_t, typename c_arg_t>
        static void andop(SDROPResult r, const SDR<SDR_t, container_t>* const a, const SDR<arg_t, c_arg_t>* const b, bool size_only = false, bool use_visitor = false);
        // or operation. places the result in r.
        static void orop(SDROPResult r, const SDR<SDR_t, container_t>* const a, const SDR<SDR_t, container_t>* const b, bool size_only = false, bool exclusive = false);
        // rm operation. places the result in r (or alternatively uses the visitor).
        template <typename arg_t, typename c_arg_t>
        static void rmop(SDROPResult r, const SDR<SDR_t, container_t>* const me, const SDR<arg_t, c_arg_t>* const arg, bool size_only = false, bool use_visitor = false);

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
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::operator=(container_t&& v) {
    this->v = v;
    return *this;
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>::SDR(container_t&& v): v(v) {
    assert_ascending();
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>::SDR(std::initializer_list<SDR_t> list): v(list) {
    assert_ascending();
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
template <typename CIT, typename AIT, typename BIT>
void SDR<SDR_t, container_t>::sdrop_add_to_output(SDROPResult r, CIT& r_pos, [[maybe_unused]] AIT a_pos, [[maybe_unused]] AIT a_end, [[maybe_unused]] BIT b_pos, [[maybe_unused]] BIT b_end, SDR_t elem, bool size_only, bool and_op) {
    // TODO this could be unified as a visitor.
    if (size_only) {
        ++*r.length;
    } else {
        if (r_pos != (decltype(r.sdr->v.begin()))NULL) {
            // inplace will never happen if container_t is a set. Just getting the compiler to cooperate
            if constexpr(!usesSet) *r_pos++ = elem;
        } else {
            auto &rsdr = r.sdr->v;
            if constexpr(usesVector) {
                // allocate based on the remaining elements / 2
                if (rsdr.capacity() == rsdr.size()) {
                    size_type a_left = distance(a_pos, a_end);
                    size_type b_left = distance(b_pos, b_end);
                    size_type cap_increase;
                    if (and_op) {
                        size_type max_remaining = a_left < b_left ? a_left : b_left;
                        cap_increase = max_remaining / 2;
                    } else {
                        size_type both_remaining = a_left + b_left;
                        cap_increase = both_remaining / 2;
                    }
                    rsdr.reserve(rsdr.capacity() + cap_increase);
                }
            }
            rsdr.insert(rsdr.end(), elem);
        }
    }
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
void SDR<SDR_t, container_t>::andop(SDROPResult r, const SDR<SDR_t, container_t>* const a, const SDR<arg_t, c_arg_t>* const b, bool size_only, bool use_visitor) {
    assert(size_only || use_visitor || r.sdr != (const SDR<SDR_t, container_t>*)b);
    bool is_inplace = !size_only && r.sdr == a;
    assert(!(is_inplace && usesSet));
    auto a_pos = a->cbegin();
    auto a_end = a->cend();
    auto b_pos = b->cbegin();
    auto b_end = b->cend();
    SDR_t a_elem;
    [[maybe_unused]]arg_t b_elem;
    auto r_pos = is_inplace ? r.sdr->v.begin() : (decltype(r.sdr->v.begin()))NULL;
    if (a_pos == a_end) goto end;
    while (true) {
        a_elem = *a_pos;
        if constexpr(isSet<c_arg_t>::value) {
            b_pos = b->v.lower_bound(a_elem);
        } else {
            b_pos = lower_bound(b_pos, b_end, a_elem);
        }
        if (b_pos == b_end) goto end;
        if (*b_pos == a_elem) {
            if (use_visitor) {
                (*r.visitor)(*const_cast<SDR_t*>(&*a_pos));
            } else {
                sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_elem, size_only, true);
            }
            ++b_pos;
            if (b_pos == b_end) goto end;
        }
        ++a_pos;
        // ============= a and b swapped ===^=V================
        if constexpr (!small_args) {
            b_elem = *b_pos;
            if constexpr(usesSet) {
                a_pos = a->v.lower_bound((SDR_t)b_elem);
            } else {
                a_pos = lower_bound(a_pos, a_end, b_elem);
            }
            if (a_pos == a_end) goto end;
            a_elem = *a_pos;
            if (a_elem == b_elem) {
                if (use_visitor) {
                    (*r.visitor)(*const_cast<SDR_t*>(&*a_pos));
                } else {
                    sdrop_add_to_output(r, r_pos, b_pos, b_end, a_pos, a_end, a_elem, size_only, true);
                }
                ++a_pos;
                if (a_pos == a_end) goto end;
            }
            ++b_pos;
        } else {
            if (a_pos == a_end) goto end;
        }
    }
    end:
    if (is_inplace) {
        // this will never be false
        if constexpr (usesVector || usesList) {
            r.sdr->v.resize(distance(r.sdr->v.begin(), r_pos));
        }
    }
    if constexpr (usesVector) {
        if (!size_only) r.sdr->v.shrink_to_fit();
    }
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::andb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    andop(rop, this, &arg, false);
    return r; // nrvo 
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
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::andi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr (usesSet) {
        SDR r;
        SDROPResult rop;
        rop.sdr = &r;
        andop(rop, this, &arg, false);
        swap(r.v, v);
    } else {
        SDROPResult rop;
        rop.sdr = this;
        andop(rop, this, &arg, false);
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
    SDROPResult rop;
    rop.length = &r;
    andop(rop, this, &arg, true);
    return r;
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
template<typename arg_t, typename c_arg_t>
void SDR<SDR_t, container_t>::andm(const SDR<arg_t, c_arg_t>& query, std::function<void(SDR_t&)>& visitor) {
    SDROPResult rop;
    rop.visitor = &visitor;
    andop(rop, this, &query, false, true);
}

template<typename SDR_t, typename container_t>
void SDR<SDR_t, container_t>::orop(SDROPResult r, const SDR<SDR_t, container_t>* const a, const SDR<SDR_t, container_t>* const b, bool size_only, bool exclusive) {
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
    #pragma GCC diagnostic push
    #if !defined(__has_warning) || __has_warning("-Wmaybe-uninitialized")
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    #endif
    while (a_valid || b_valid) {
        if ((a_valid && !b_valid) || (a_valid && b_valid && a_val < b_val)) {
            sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_val, size_only, false);
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
        } else if ((!a_valid && b_valid) || (a_valid && b_valid && a_val > b_val)) {
            sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, b_val, size_only, false);
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        } else {
            if (!exclusive) {
                sdrop_add_to_output(r, r_pos, a_pos, a_end, b_pos, b_end, a_val, size_only, false);
            }
            if (a_pos != a_end) a_val = *a_pos++; else a_valid = false; // a
            if (b_pos != b_end) b_val = *b_pos++; else b_valid = false; // b
        }
    }
    #pragma GCC diagnostic pop
    if constexpr(usesVector) if (!size_only) r.sdr->v.shrink_to_fit();
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::orb(const SDR<SDR_t, container_t>& arg) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, false);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::ori(const SDR<SDR_t, container_t>& arg) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, false);
    swap(r.v, v);
    return *this;
}

template<typename SDR_t, typename container_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ors(const SDR<SDR_t, container_t>& arg) const {
    size_type r = 0;
    SDROPResult rop;
    rop.length = &r;
    orop(rop, this, &arg, true, false);
    return r;
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::xorb(const SDR<SDR_t, container_t>& arg) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, true);
    return r; // nrvo
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::xori(const SDR<SDR_t, container_t>& arg) {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    orop(rop, this, &arg, false, true);
    swap(r.v, v);
    return *this;
}

template<typename SDR_t, typename container_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::xors(const SDR<SDR_t, container_t>& arg) const {
    size_type r = 0;
    SDROPResult rop;
    rop.length = &r;
    orop(rop, this, &arg, true, true);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::rmi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesSet) {
        // revert back to normal remove, with a swap 
        SDR replacement;
        SDROPResult rop;
        rop.sdr = &replacement;
        rmop(rop, this, &arg, false);
        swap(replacement.v, v);
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
template<typename arg_t, typename c_arg_t>
void SDR<SDR_t, container_t>::rmop(SDROPResult r, const SDR<SDR_t, container_t>* const me, const SDR<arg_t, c_arg_t>* const arg, bool size_only, bool use_visitor) {
    assert(size_only || use_visitor || (r.sdr != me && r.sdr != (const SDR<SDR_t, container_t>*)arg));
    SDR<SDR_t, container_t> ret;
    auto arg_pos = arg->cbegin();
    auto arg_end = arg->cend();
    auto this_pos = me->cbegin();
    auto this_end = me->cend();
    arg_t arg_elem;
    SDR_t this_elem;

    auto get_next_arg = [&this_elem, &arg_elem, &arg_pos, &arg_end]() -> bool {
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
            if (use_visitor) {
                (*r.visitor)(*const_cast<SDR_t*>(&*this_pos));
            } else {
                if (size_only) {
                    ++*r.length;
                } else {
                    if constexpr(usesVector) {
                        if (r.sdr->v.capacity() == r.sdr->v.size()) {
                            r.sdr->v.reserve(r.sdr->v.capacity() + distance(this_pos, this_end) / 2);
                        }
                    }
                    r.sdr->v.insert(r.sdr->v.end(), this_elem);
                }
            }
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
                if (use_visitor) {
                    (*r.visitor)(*const_cast<SDR_t*>(&*this_pos));
                } else {
                    if (size_only) {
                        ++*r.length;
                    } else {
                        if constexpr(usesVector) {
                            if (r.sdr->v.capacity() == r.sdr->v.size()) {
                                r.sdr->v.reserve(r.sdr->v.capacity() + distance(this_pos, this_end) / 2);
                            }
                        }
                        r.sdr->v.insert(r.sdr->v.end(), this_elem);
                    }
                    ++this_pos;
                    goto dump_remaining_this;
                }
                ++this_pos;
                if (this_pos == this_end) return;
            }
        }
    }
    dump_remaining_this:
    if (size_only) {
        if constexpr (usesVector) {
            *r.length += this_end - this_pos;
        } else {
            while (this_pos != this_end) {
                ++*r.length;
                ++this_pos;
            }
        }
    } else {
        while (this_pos != this_end) {
            if (use_visitor) {
                (*r.visitor)(*const_cast<SDR_t*>(&*this_pos++));
            } else {
                if constexpr(usesVector) {
                    if (r.sdr->v.capacity() == r.sdr->v.size()) {
                        r.sdr->v.reserve(r.sdr->v.capacity() + distance(this_pos, this_end) / 2);
                    }
                }
                r.sdr->v.insert(r.sdr->v.end(), *this_pos++);
            }
        }
        if constexpr (usesVector) {
            r.sdr->v.shrink_to_fit();
        }
    }
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::rmb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR r;
    SDROPResult rop;
    rop.sdr = &r;
    rmop(rop, this, &arg, false);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    SDROPResult rop;
    rop.length = &r;
    rmop(rop, this, &arg, true);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
void SDR<SDR_t, container_t>::rmm(const SDR<arg_t, c_arg_t>& query, std::function<void(SDR_t&)>& visitor) {
    SDROPResult rop;
    rop.visitor = &visitor;
    rmop(rop, this, &query, false, true);
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
