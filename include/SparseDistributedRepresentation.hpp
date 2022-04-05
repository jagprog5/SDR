#pragma once

#include <unistd.h>
#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <functional>
#include <forward_list>
#include <vector>
#include <set>
#include <iostream>

namespace {

template <typename T>
struct isVector : std::false_type {};

template <typename T, typename A>
struct isVector<std::vector<T, A>> : std::true_type {};

template <typename T>
struct isForwardList : std::false_type {};

template <typename T, typename A>
struct isForwardList<std::forward_list<T, A>> : std::true_type {};

template <typename T>
struct isSet : std::false_type {};

template <typename T, typename C, typename A>
struct isSet<std::set<T, C, A>> : std::true_type {};

template <typename container_t, bool enable_member>
struct MaybeSize;

template <typename container_t>
struct MaybeSize<container_t, true> {
    typename container_t::size_type size;
};

template <typename container_t>
struct MaybeSize<container_t, false> {
};

inline auto& get_twister() {
    static std::mt19937 twister(time(NULL) * getpid() * 33);
    return twister;
}
}

/*
 * Based off the ideas explained in this series:
 * https://youtu.be/ZDgCdWTuIzc
 * Numenta: SDR Capacity & Comparison (Episode 2)
 */
template<typename SDR_t = unsigned int, typename container_t = std::vector<SDR_t>>
class SDR {
    private:
        // used in ctors
        void initFLSize() {
            if constexpr(usesForwardList) {
                this->maybe_size.size = 0;
                auto it = this->v.cbefore_begin();
                while (it != this->v.cend()) {
                    ++it;
                    ++this->maybe_size.size;
                }
            }
        }
    public:
        using index_type = SDR_t;
        using size_type = typename container_t::size_type;
        using const_iterator = typename container_t::const_iterator;
        using iterator = typename container_t::iterator;

        SDR() {
            if constexpr(usesForwardList)
                this->maybe_size.size = 0;
        }

        SDR(const SDR& sdr): v(sdr.v) {
            if constexpr(usesForwardList)
                this->maybe_size.size = sdr.maybe_size.size;
        }

        SDR& operator=(const SDR& sdr) {
            this->v = sdr.v;
            if constexpr(usesForwardList)
                this->maybe_size.size = sdr.v.maybe_size.size;
            return *this;
        }

        SDR(SDR&& sdr): v(std::move(sdr.v)) {}

        SDR& operator=(SDR&& sdr) {
            this->v = std::move(sdr.v);
            return *this;
        }

        // constructors from underlying container
        SDR(const container_t& v): v(v) {
            assert_ascending();
            initFLSize();
        }

        SDR& operator=(const container_t& v) {
            this->v = v;
            assert_ascending();
            initFLSize();
            return *this;
        }

        SDR(container_t&& v): v(v) {
            assert_ascending();
            initFLSize();
        }

        SDR& operator=(container_t&& v) {
            this->v = v;
            assert_ascending();
            initFLSize();
            return *this;
        }

        container_t&& data() { return std::move(v); }

        SDR(std::initializer_list<SDR_t> list) : v(list) {
            assert_ascending();
            if constexpr(usesForwardList)
                this->maybe_size.size = list.size();
        };

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
        
        // Sets bit in this, then returns if the sdr was modified.
        bool set(SDR_t index, bool value);

        // Sets bits in this, then returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& set(SDR<arg_t, c_arg_t> arg, bool value);

        // Returns this, shifted by amount.
        SDR<SDR_t, container_t>& shift(int amount);

        // concatenate an SDR to an SDR. Every indice in arg must be greater than every indice in this. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& append(const SDR<arg_t, c_arg_t>& arg);

        auto cbegin() const { return v.cbegin(); }
        auto cend() const { return v.cend(); }
        auto empty() const { return v.empty(); }

        auto size() const {
            if constexpr(usesForwardList) {
                return maybe_size.size;
            } else {
                return v.size();
            }
        }

        template<typename T = container_t>
        typename T::const_reverse_iterator crbegin() const { return v.crbegin(); }

        template<typename T = container_t>
        typename T::const_reverse_iterator crend() const { return v.crend(); }

        template<typename T = container_t>
        typename std::enable_if<!isForwardList<T>::value, void>::type push_back(SDR_t i) {
            assert(v.empty() || *v.crbegin() < i);
            v.insert(v.end(), i);
        }

        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, typename T::iterator>::type before_begin() {
            return v.before_begin();
        }


        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, typename T::iterator>::type before_end() {
            auto it = v.before_begin();
            while (true) {
                auto next = std::next(it);
                if (next == v.end()) return it;
                it = next;
            }
        }

        // Append to a forward_list based sdr.
        // @param position points to before the end.
        // @param i is the element to append
        // @return the next position before the end.
        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, typename T::iterator>::type insert_end(typename T::iterator position, SDR_t i)  {
            assert(std::next(position) == v.cend() || *position < i);
            ++maybe_size.size;
            return v.insert_after(position, i);
        }

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
        auto operator+(const other& o) const { return orb(o); }
        template<typename other>
        auto operator+=(const other& o) { return set(o, true); }
        template<typename other>
        auto operator-(const other& o) const { return rmb(o); }
        template<typename other>
        auto operator-=(const other& o) { return set(o, false); }
        template<typename other>
        auto operator*(const other& o) const { return SDR(*this).sample_portion(o); }
        template<typename other>
        auto operator*=(const other& o) { return sample_portion(o); }
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
            // this doesn't have any meaning; merely allows storage in trees, etc
            return std::lexicographical_compare(this->v.cbegin(), this->v.cend(), other.v.cbegin(), other.v.cend());
        }
        auto operator>=(const SDR<SDR_t, container_t>& other) const { return !(*this < other); }
        auto operator>(const SDR<SDR_t, container_t>& other) const { return other < *this; }
        auto operator<=(const SDR<SDR_t, container_t>& other) const { return !(*this > other); }

        static constexpr bool usesVector = isVector<container_t>::value;
        static constexpr bool usesForwardList = isForwardList<container_t>::value;
        static constexpr bool usesSet = isSet<container_t>::value;
        static_assert(usesVector || usesForwardList || usesSet);

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

        // augmenting the forward_list with a size
        MaybeSize<container_t, usesForwardList> maybe_size;

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
            if (v.empty()) return;
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
    if constexpr(usesVector) v.resize(size);

    float progress = input / period;
    progress -= (int)progress;
    size_type start_index = std::round(progress * underlying_array_length);

    if (start_index + size > underlying_array_length) {
        // if elements would go off the end of the array, wrap them back to the start

        // the number of elements that wrap off the end
        size_type wrapped_elements = start_index + size - underlying_array_length;
        
        // the number of elements that don't wrap off the end
        size_type non_wrapped_elements = size - wrapped_elements;

        if constexpr(usesForwardList) {
            auto insert_it = before_begin();
            for (size_type i = 0; i < wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDR_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDR_t(start_index + i));
            }
        } else if constexpr(usesSet) {
            for (size_type i = 0; i < wrapped_elements; ++i) {
                v.insert(v.end(), SDR_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                v.insert(v.end(), SDR_t(start_index + i));
            }
        } else {
            for (size_type i = 0; i < wrapped_elements; ++i) {
                v[i] = SDR_t(i);
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                v[i + non_wrapped_elements] = SDR_t(i);
            }
        }
    } else {
        // no elements are wrapped from the end
        if constexpr(usesForwardList) {
            auto insert_it = before_begin();
            for (size_type i = 0; i < size; ++i) {
                insert_it = v.insert_after(insert_it, SDR_t(start_index + i));
            }
        } else if constexpr(usesSet) {
            for (size_type i = 0; i < size; ++i) {
                v.insert(v.end(), SDR_t(start_index + i));
            }
        } else {
            for (size_type i = 0; i < size; ++i) {
                v[i] = SDR_t(start_index + i);
            }
        }
    }
    if constexpr(usesForwardList) {
        maybe_size.size = size;
    }
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>::SDR(float input, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length);
    assert(input >= 0);
    if constexpr(usesVector) v.resize(size);
    size_type start_index = std::round((underlying_array_length - size) * input);
    if constexpr(usesForwardList) {
        auto insert_it = before_begin();
        for (size_type i = 0; i < size; ++i) {
            insert_it = v.insert_after(insert_it, start_index + i);
        }
    } else if constexpr(usesSet) {
        for (size_type i = 0; i < size; ++i) {
            v.insert(v.end(), SDR_t(start_index + i));
        }
    } else {
        for (size_type i = 0; i < size; ++i) {
            v[i] = SDR_t(start_index + i);
        }
    }
    if constexpr(usesForwardList) {
        maybe_size.size = size;
    }
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::sample_portion(float amount) {
    assert(amount >= 0 && amount <= 1);
    auto check_val = amount * (get_twister().max() / 2);
    if constexpr(usesVector) {
        auto to_offset = v.begin();
        auto from_offset = v.cbegin();
        auto end = v.end();
        while (from_offset != end) {
            if ((get_twister()() / 2) >= check_val) {
                *to_offset++ = *from_offset;
            }
            from_offset++;
        }
        v.resize(from_offset - v.cbegin());
    } else if constexpr(usesSet) {
        for (auto pos = v.begin(); pos != v.end(); ++pos) {
            if ((get_twister()() / 2) < check_val) {
                pos = v.erase(pos);
            }
        }
    } else if constexpr(usesForwardList) {
        typename container_t::size_type remove_count = 0;
        auto pos = v.before_begin();
        while (true) {
            auto next = std::next(pos);
            if (next == v.end()) break;
            if ((get_twister()() / 2) < check_val) {
                v.erase_after(pos); // this seg faults. also, make it use the random int mersenne instead
                ++remove_count;
            } else {
                pos = next;
            }
        }
        maybe_size.size -= remove_count;
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
        pos = lower_bound(v.cbegin(), v.cend(), val);
    }
    return pos != v.end() && *pos == val;
}

template<typename SDR_t, typename container_t>
template<typename arg_t>
SDR<SDR_t, container_t> SDR<SDR_t, container_t>::andb(arg_t start_inclusive, arg_t stop_exclusive) const {
    assert(start_inclusive <= stop_exclusive);
    SDR sdr;
    typename container_t::const_iterator start_it;
    if constexpr(!usesSet) {
        start_it = lower_bound(v.cbegin(), v.cend(), start_inclusive);
    } else {
        start_it = v.lower_bound(start_inclusive);
    }
    typename container_t::const_iterator end_it;
    if constexpr(usesForwardList) {
        end_it = start_it;
        while (true) {
            if (end_it == cend() || *end_it >= stop_exclusive) {
                break;
            }
            ++end_it;
            ++sdr.maybe_size.size;
        }
    } else if constexpr(usesVector) {
        end_it = lower_bound(start_it, v.cend(), stop_exclusive);
    } else if constexpr(usesSet) {
        end_it = v.lower_bound(stop_exclusive);
    }
    if constexpr(usesVector) {
        sdr.v.resize(end_it - start_it);
    }
    if constexpr (usesForwardList) {
        auto insert_it = sdr.v.before_begin();
        for (auto it = start_it; it != end_it; ++it) {
            insert_it = sdr.v.insert_after(insert_it, *it);
        }
    } else if constexpr(usesSet) {
        for (auto it = start_it; it != end_it; ++it) {
            sdr.v.insert(sdr.v.end(), *it);
        }
    } else {
        auto insert_it = sdr.v.begin();
        for (auto it = start_it; it != end_it; ++it) {
            *insert_it++ = *it;
        }
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
bool SDR<SDR_t, container_t>::set(SDR_t index, bool value) {
    if constexpr(usesForwardList) {
        if (value) {
            auto it = v.before_begin();
            while (true) {
                auto next = std::next(it);
                if (next == v.end()) {
                    return false;
                } else if (*next == index) {
                    return false;
                } else if (*next > index) {
                    v.insert_after(it, next);
                    return true;
                }
                it = next;
            }
        } else {
            auto it = v.before_begin();
            while (true) {
                auto next = std::next(it);
                if (next == v.end()) {
                    return false;
                } else if (*next == index) {
                    v.erase_after(it);
                    return true;
                } else if (*next > index) {
                    return false;
                }
                it = next;
            }
        }
    } else {
        typename container_t::iterator it;
        if constexpr(usesSet) {
            it = v.lower_bound(value);
        } else {
            it = lower_bound(v.begin(), v.end(), value);
        }
        if (value) {
            if (it == v.end() || *it != index) {
                v.insert(it, index);
                return true;
            } else {
                return false;
            }
        } else {
            if (it != v.end() && *it == index) {
                v.erase(it);
            } else {
                return false;
            }
        }
    }
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
    std::function<void(SDR_t&)> visitor;
    [[maybe_unused]] typename container_t::iterator it;
    if constexpr(usesForwardList) {
        it = r.v.before_begin();
        visitor = [&](SDR_t& elem) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, elem);
        };
    } else {
        visitor = [&](SDR_t& elem) {
            r.push_back(elem);
        };
    }
    andop(*this, arg, visitor);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::andi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr (usesSet) {
        SDR r = andb(arg);
        this->v = std::move(r.v);
    } else if constexpr(usesVector) {
        auto pos = this->v.begin();
        auto visitor = [&](SDR_t& elem) {
            *pos++ = elem;
        };
        andop(*this, arg, visitor);
        this->v.resize(pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        size_type i = 0;
        auto lagger = this->v.before_begin();
        auto pos = this->v.begin();
        auto visitor = [&](SDR_t& elem) {
            ++lagger;
            ++i;
            *pos++ = elem;
        };
        andop(*this, arg, visitor);
        while (std::next(lagger) != v.end()) {
            v.erase_after(lagger);
        }
        this->maybe_size.size = i;

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
    #if defined(__has_warning)
        #if __has_warning("-Wmaybe-uninitialized")
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        #endif
    #endif
    
    while (a_valid || b_valid) {
        if ((a_valid && !b_valid) || (a_valid && b_valid && a_val < b_val)) {
            visitora(*const_cast<SDR_t*>(&*a_pos));
            ++a_pos;
            if (a_pos != a_end) a_val = *a_pos; else a_valid = false; // a
        } else if ((!a_valid && b_valid) || (a_valid && b_valid && a_val > b_val)) {
            visitorb(*const_cast<arg_t*>(&*b_pos));
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
    std::function<void(SDR_t&)> visitor;
    if constexpr(usesForwardList) {
        auto it = r.v.before_begin();
        visitor = [&](SDR_t& elem) {
            it = r.v.insert_after(it, elem);
            ++r.maybe_size.size;
        };
        orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), false>(*this, arg, visitor, visitor);
    } else {
        visitor = [&](SDR_t& elem) {
            r.push_back(elem);
        };
        orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), false>(*this, arg, visitor, visitor);
    }
    if constexpr(usesVector) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::ori(const SDR<arg_t, c_arg_t>& arg) {
    SDR r = orb(arg);
    this->v = std::move(r.v);
    if constexpr(usesForwardList) {
        this->maybe_size.size = r.maybe_size.size;
    }
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
    std::function<void(SDR_t&)> visitor;
    [[maybe_unused]] typename container_t::iterator it;
    if constexpr(usesForwardList) {
        it = r.v.before_begin();
        visitor = [&](SDR_t& elem) {
            it = r.v.insert_after(it, elem);
            ++r.maybe_size.size;
        };
    } else {
        visitor = [&](SDR_t& elem) {
            r.push_back(elem);
        };
    }
    orop<arg_t, c_arg_t, decltype(visitor), decltype(visitor), true>(*this, arg, visitor, visitor);
    if constexpr(usesVector) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::xori(const SDR<arg_t, c_arg_t>& arg) {
    SDR r = xorb(arg);
    this->v = std::move(r.v);
    if constexpr(usesForwardList) {
        this->maybe_size.size = r.maybe_size.size;
    }
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
        this->v = std::move(r.v);
    } else if constexpr(usesVector) {
        if constexpr(isForwardList<c_arg_t>::value) {
            // this is vector based, and arg is forward_list based
            SDR r = this->rmb(arg);
            this->v = std::move(r.v);
        } else {
            auto arg_pos = arg.crbegin();
            auto arg_end = arg.crend();
            auto this_pos = this->v.rbegin();
            auto this_end = this->v.rend();
            if (arg_pos == arg_end) goto end;
            while (true) {
                arg_t elem = *arg_pos++;
                this_pos = lower_bound(this_pos, this_end, elem, std::greater<arg_t>());
                if (this_pos == this_end) goto end;
                if (*this_pos == elem) {
                    this->v.erase((++this_pos).base());
                    if (this_end != this->v.rend()) {
                        // revalidate iterators
                        this_pos = this->v.rend() - (this_end - this_pos);
                        this_end = this->v.rend();
                    }
                    if (this_pos == this_end) goto end;
                    elem = *this_pos;
                }
                // =====
                if constexpr (!small_args) {
                    arg_pos = lower_bound(arg_pos, arg_end, elem, std::greater<arg_t>());
                    if (arg_pos == arg_end) goto end;
                    if (*arg_pos == elem) {
                        this->v.erase((++this_pos).base());
                        if (this_end != this->v.rend()) {
                            // revalidate iterators
                            this_pos = this->v.rend() - (this_end - this_pos);
                            this_end = this->v.rend();
                        }
                        if (this_pos == this_end) goto end;
                    }
                } else {
                    if (arg_pos == arg_end) goto end;
                }
            }
            end:
            this->v.shrink_to_fit();
        }
    } else if constexpr(usesForwardList) {
        auto arg_pos = arg.cbegin();
        auto arg_end = arg.cend();
        auto this_lagger = this->v.before_begin();
        auto this_pos = this->v.begin();
        auto this_end = this->v.end();
        SDR_t this_elem;
        arg_t arg_elem;

        if (this_pos == this_end) goto skip;
        this_elem = *this_pos;

        if (arg_pos == arg_end) goto skip;
        arg_elem = *arg_pos;

        while (true) {
            if (this_elem < arg_elem) {
                ++this_lagger;
                ++this_pos;
                if (this_pos == this_end) break;
                this_elem = *this_pos;
            } else if (this_elem == arg_elem) {
                ++this_pos;
                this->v.erase_after(this_lagger);
                --this->maybe_size.size;
                if (this_pos == this_end) break;
                this_elem = *this_pos;
                ++arg_pos;
                if (arg_pos == arg_end) break;
                arg_elem = *arg_pos;
            } else {
                ++arg_pos;
                if (arg_pos == arg_end) break;
                arg_elem = *arg_pos;
            }
        }
        skip:
            (void)0;
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
    std::function<void(SDR_t&)> visitor;
    [[maybe_unused]] typename container_t::iterator it;
    if constexpr(usesForwardList) {
        it = r.v.before_begin();
        visitor = [&](SDR_t& elem) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, elem);
        };
    } else {
        visitor = [&](SDR_t& elem) {
            r.push_back(elem);
        };
    }
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
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::set(SDR<arg_t, c_arg_t> arg, bool value) {
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
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::append(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(!usesForwardList) {
        assert(empty() || arg.empty() || *v.crbegin() < *arg.v.cbegin());
    }
    if constexpr(usesForwardList) {
        auto it = before_end();
        assert(empty() || arg.empty() || *it < *arg.v.cbegin());
        for (auto e : arg.v) { it = insert_end(it, e); }
    } else if constexpr(usesSet) {
        for (auto e : arg.v) { push_back(e); }
    } else {
        auto old_size = this->v.size();
        this->v.resize(this->v.size() + arg.v.size());
        auto insert_it = this->v.begin() + old_size;
        for (auto it = arg.v.begin(); it != arg.v.end(); ++it) {
            *insert_it++ = *it;
        }
    }
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
