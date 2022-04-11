#pragma once

#include <SDRDefinitions.hpp>
#include <unistd.h>
#include <assert.h>
#include <initializer_list>
#include <algorithm>
#include <random>
#include <functional>
#include <optional>

namespace SparseDistributedRepresentation {

inline std::mt19937 twister(time(NULL) * getpid());

/*
 * Inspired from ideas explained in this series:
 * https://youtu.be/ZDgCdWTuIzc
 * Numenta: SDR Capacity & Comparison (Episode 2)
 */
template<typename SDR_t = SDR_t<>, typename container_t = std::vector<SDR_t>>
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
        using element_type = SDR_t;
        using size_type = typename container_t::size_type;
        using iterator = typename container_t::iterator;
        using const_iterator = typename container_t::const_iterator;

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

        container_t&& get_raw() { return std::move(v); }

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

        // Each bit has a chance of being turned off, specified by amount, where 0 always clears the sdr, and 1 nearly always leaves it unchanged.
        SDR<SDR_t, container_t>& sample_portion(float amount);

        // and bit. returns the state of a bit.
        template<typename arg_t>
        bool andb(arg_t val) const;

        // and bits. returns the state of many bits.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> andb(const SDR<arg_t, c_arg_t>& arg) const;

        // and bits. returns the state of many bits from start to stop.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t>
        SDR<ret_t, c_ret_t> andb(arg_t start_inclusive, arg_t stop_exclusive) const;

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
         * and visitor. Perform an operation on each element in this AND in the query.
         * each selected element pair is called in the visitor as visitor(const SDR_t::id_type&, SDR_t::data_type&, arg_t::data_type&)
         */
        template<typename arg_t, typename c_arg_t, typename Visitor>
        void andv(const SDR<arg_t, c_arg_t>& query, Visitor visitor);
        
        // or bits.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> orb(const SDR<arg_t, c_arg_t>& arg) const;

        // or inplace. turn on all bits in arg. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& ori(const SDR<arg_t, c_arg_t>& arg);

        // or size. returns the number of bits in this or arg.
        template<typename arg_t, typename c_arg_t>
        size_type ors(const SDR<arg_t, c_arg_t>& arg) const;

        // xor bits.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> xorb(const SDR<arg_t, c_arg_t>& arg) const;

        // xor inplace. computes this xor arg, and places the result in this. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& xori(const SDR<arg_t, c_arg_t>& arg);

        // xor size, aka hamming distance. returns the number of bits in this xor arg.
        template<typename arg_t, typename c_arg_t>
        size_type xors(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * or visitor. Perform an operation on each element in this OR in arg
         * three visitors are defined:
         *      the element only exists in a: visitora(const SDR_t::id_type&, SDR_t::data_type&)
         *      the element only exists in b: visitorb(const arg_t::id_type&, arg_t::data_type&)
         *      the element is in both: visitorc(const SDR_t::id_type&, SDR_t::data_type&, arg_t::data_type&)
         */
        template<typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, typename VisitorC>
        void orv(const SDR<arg_t, c_arg_t>& query, VisitorA visitora, VisitorB visitorb, VisitorC visitorc);

        // Returns a copy of this which lacks any bit from arg.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> rmb(const SDR<arg_t, c_arg_t>& arg) const;

        // Remove inplace. Remove all bits in arg from this, then returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& rmi(const SDR<arg_t, c_arg_t>& arg);

        // Returns the number of elements in this that are not in arg.
        template<typename arg_t, typename c_arg_t>
        size_type rms(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * rm visitor. Perform an operation on all elements based on a query.
         *  2 visitors are defined:
         *      the element only exists in this: visitora(const SDR_t::id_type&, SDR_t::data_type&)
         *      the element exists in both: visitorb(const SDR_t::id_type&, SDR_t::data_type&, arg_t::data_type&)        
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

        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, void>::type pop_front()  {
            v.pop_front();
        }

        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, void>::type push_front(SDR_t i)  {
            assert(v.empty() || *v.cbegin() > i);
            v.push_front(i);
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
        auto operator+=(const other& o) { return ori(o); }
        template<typename other>
        auto operator-(const other& o) const { return rmb(o); }
        template<typename other>
        auto operator-=(const other& o) { return rmi(o); }
        template<typename other>
        auto operator*(const other& o) { return SDR(*this).sample_portion(o); }
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

        template<typename arg_t, typename c_arg_t>
        auto operator==(const SDR<arg_t, c_arg_t>& other) const {
            auto this_pos = this->cbegin();
            auto this_end = this->cend();
            auto other_pos = other.cbegin();
            auto other_end = other.cend();
            while (true) {
                if (this_pos == this_end) return other_pos == other_end;
                if (other_pos == other_end) return false;
                auto this_elem = *this_pos++;
                auto other_elem = *other_pos++;
                if (this_elem != other_elem) return false;
            }
        }

        template<typename arg_t, typename c_arg_t>
        auto operator!=(const SDR<arg_t, c_arg_t>& other) const { return !(*this == other); }

        template<typename arg_t, typename c_arg_t>
        auto operator<(const SDR<arg_t, c_arg_t>& other) const {
            return std::lexicographical_compare(this->v.cbegin(), this->v.cend(), other.v.cbegin(), other.v.cend());
        }

        template<typename arg_t, typename c_arg_t>
        auto operator>=(const SDR<arg_t, c_arg_t>& other) const { return !(*this < other); }

        template<typename arg_t, typename c_arg_t>
        auto operator>(const SDR<arg_t, c_arg_t>& other) const { return other < *this; }

        template<typename arg_t, typename c_arg_t>
        auto operator<=(const SDR<arg_t, c_arg_t>& other) const { return !(*this > other); }

        static constexpr bool usesVector = isVector<container_t>::value;
        static constexpr bool usesForwardList = isForwardList<container_t>::value;
        static constexpr bool usesSet = isSet<container_t>::value;
        static_assert(usesVector || usesForwardList || usesSet);

    private:
        container_t v;

        void assert_ascending();

        // and operation. computes A & B.
        // each selected element match is called in the visitor
        // visitor(const SDR_t::id_type&, SDR_t::data_type&, arg_t::data_type&)
        template <typename arg_t, typename c_arg_t, typename Visitor>
        static void andop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, Visitor visitor);

        // or operation. computes A | B
        // 3 visitors are defined:
        //      the element only exists in a: visitora(const SDR_t::id_type&, SDR_t::data_type&)
        //      the element only exists in b: visitorb(const arg_t::id_type&, arg_t::data_type&)
        //      the element exists in both: visitorc(const SDR_t::id_type&, SDR_t::data_type&, arg_t::data_type&)
        template <typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, typename VisitorC>
        static void orop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, VisitorA visitora, VisitorB visitorb, VisitorC visitorc);

        // rm operation.
        // 2 visitors are defined:
        //      the element only exists in "me": visitora(const SDR_t::id_type&, SDR_t::data_type&)
        //      the element exists in both: visitorb(const SDR_t::id_type&, SDR_t::data_type&, arg_t::data_type&)        
        // the trailing elements (not in the arg) start position is called in DumpEnd(const_iterator)
        template <typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, typename DumpEnd>
        static void rmop(const SDR<SDR_t, container_t>& me, const SDR<arg_t, c_arg_t>& arg, VisitorA visitora, VisitorB visitorb, DumpEnd dumpEnd);

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
    auto check_val = amount * (float)twister.max();
    if constexpr(usesVector) {
        auto to_offset = v.begin();
        auto from_offset = v.cbegin();
        auto end = v.end();
        while (from_offset != end) {
            if (twister() < check_val) {
                *to_offset++ = *from_offset;
            }
            from_offset++;
        }
        v.resize(to_offset - v.cbegin());
    } else if constexpr(usesSet) {
        auto pos = v.begin();
        while (pos != v.end()) {
            if (twister() >= check_val) {
                pos = v.erase(pos);
            } else {
                ++pos;
            }
        }
    } else if constexpr(usesForwardList) {
        typename container_t::size_type remove_count = 0;
        auto pos = v.before_begin();
        while (true) {
            auto next = std::next(pos);
            if (next == v.end()) break;
            if (twister() >= check_val) {
                v.erase_after(pos);
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
        pos = v.lower_bound(val.id);
    } else {
        pos = lower_bound(v.cbegin(), v.cend(), val);
    }
    return pos != v.end() && *pos == val;
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::andb(arg_t start_inclusive, arg_t stop_exclusive) const {
    assert(start_inclusive <= stop_exclusive);
    SDR<ret_t, c_ret_t> sdr;
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
    if constexpr(isVector<c_ret_t>::value) {
        sdr.v.resize(end_it - start_it);
    }
    if constexpr (isForwardList<c_ret_t>::value) {
        auto insert_it = sdr.v.before_begin();
        for (auto it = start_it; it != end_it; ++it) {
            insert_it = sdr.v.insert_after(insert_it, (ret_t)*it);
        }
    } else if constexpr(isSet<c_ret_t>::value) {
        for (auto it = start_it; it != end_it; ++it) {
            sdr.v.insert(sdr.v.end(), (ret_t)*it);
        }
    } else {
        auto insert_it = sdr.v.begin();
        for (auto it = start_it; it != end_it; ++it) {
            *insert_it++ = (ret_t)*it;
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
            b_pos = b.v.lower_bound(a_elem.id);
        } else {
            b_pos = lower_bound(b_pos, b_end, a_elem);
        }
        if (b_pos == b_end) return;
        if (*b_pos == a_elem) {
            visitor(a_pos->id, *const_cast<typename SDR_t::data_type*>(&a_pos->data), *const_cast<typename arg_t::data_type*>(&b_pos->data));
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
                visitor(a_pos->id, *const_cast<typename SDR_t::data_type*>(&a_pos->data), *const_cast<typename arg_t::data_type*>(&b_pos->data));
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
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::andb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.andb(arg_data);
            if (data.relevant()) {
                ++r.maybe_size.size;
                ret_t elem(this_id, data);
                it = r.v.insert_after(it, elem);
            }
        };
    } else {
        visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.andb(arg_data);
            if (data.relevant()) {
                ret_t elem(this_id, data);
                r.push_back(elem);
            }
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
        auto visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename SDR_t::data_type data = this_data.andb(arg_data);
            if (data.relevant()) {
                SDR_t elem(this_id, data);
                *pos++ = elem;
            }
        };
        andop(*this, arg, visitor);
        this->v.resize(pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        size_type i = 0;
        auto lagger = this->v.before_begin();
        auto pos = this->v.begin();
        auto visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename SDR_t::data_type data = this_data.andb(arg_data);
            if (data.relevant()) {
                SDR_t elem(this_id, data);
                ++lagger;
                ++i;
                *pos++ = elem;
            }
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
    auto visitor = [&]([[maybe_unused]] const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
        typename SDR_t::data_type data = this_data.andb(arg_data);
        if (data.relevant()) {
            ++r;
        }
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
template <typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, typename VisitorC>
void SDR<SDR_t, container_t>::orop(const SDR<SDR_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, VisitorA visitora, VisitorB visitorb, VisitorC visitorc) {
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
            visitora(a_pos->id, *const_cast<typename SDR_t::data_type*>(&a_pos->data));
            ++a_pos;
            if (a_pos != a_end) a_val = *a_pos; else a_valid = false; // a
        } else if ((!a_valid && b_valid) || (a_valid && b_valid && a_val.id > b_val.id)) {
            visitorb(b_pos->id, *const_cast<typename arg_t::data_type*>(&b_pos->data));
            ++b_pos;
            if (b_pos != b_end) b_val = *b_pos; else b_valid = false; // b
        } else {
            visitorc(a_pos->id, *const_cast<typename SDR_t::data_type*>(&a_pos->data), *const_cast<typename arg_t::data_type*>(&b_pos->data));
            ++a_pos;
            ++b_pos;
            if (a_pos != a_end) a_val = *a_pos; else a_valid = false; // a
            if (b_pos != b_end) b_val = *b_pos; else b_valid = false; // b
        }
    }
    #pragma GCC diagnostic pop
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::orb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&)> visitora;
    std::function<void(const typename arg_t::id_type&, typename arg_t::data_type&)> visitorb;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitorc;
    if constexpr(isForwardList<c_ret_t>::value) {
        auto it = r.v.before_begin();
        visitora = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            it = r.v.insert_after(it, ret_t(id, data));
            ++r.maybe_size.size;
        };
        visitorb = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
            ++r.maybe_size.size;
        };
        visitorc = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.orb(arg_data);
            // there is no relevance check here, since it is assumed that elements which already exist in an SDR are relevant,
            // and that orb can only produce relevant elements from relevant elements
            ret_t elem(this_id, data);
            it = r.v.insert_after(it, ret_t(this_id, data));
            ++r.maybe_size.size;
        };
        orop<arg_t, c_arg_t, decltype(visitora), decltype(visitorb), decltype(visitorc)>(*this, arg, visitora, visitorb, visitorc);
    } else {
        visitora = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            r.push_back(ret_t(id, data));
        };
        visitorb = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            r.push_back(ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
        };
        visitorc = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.orb(arg_data);
            ret_t elem(this_id, data);
            r.push_back(elem);
        };
        orop<arg_t, c_arg_t, decltype(visitora), decltype(visitorb), decltype(visitorc)>(*this, arg, visitora, visitorb, visitorc);
    }
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
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
    auto visitora = [&](const typename SDR_t::id_type&, typename SDR_t::data_type&) {
        ++r;
    };
    auto visitorb = [&](const typename arg_t::id_type&, typename arg_t::data_type&) {
        ++r;
    };
    auto visitorc = [&](const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&) {
        ++r;
    };
    orop<arg_t, c_arg_t, decltype(visitora), decltype(visitorb), decltype(visitorc)>(*this, arg, visitora, visitorb, visitorc);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::xorb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&)> visitora;
    std::function<void(const typename arg_t::id_type&, typename arg_t::data_type&)> visitorb;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitorc;
    // scoping weirdness requires it declared here
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitora = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            it = r.v.insert_after(it, ret_t(id, data));
            ++r.maybe_size.size;
        };
        visitorb = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
            ++r.maybe_size.size;
        };
        visitorc = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename SDR_t::data_type data = this_data.xorb(arg_data);
            if (data.rm_relevant()) {
                it = r.v.insert_after(it, ret_t(this_id, data));
                ++r.maybe_size.size;
            }
        };
    } else {
        visitora = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            r.push_back(ret_t(id, data));
        };
        visitorb = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            r.push_back(ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
        };
        visitorc = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.xorb(arg_data);
            if (data.rm_relevant()) {
                ret_t elem(this_id, data);
                r.push_back(elem);
            }
        };
    }
    orop<arg_t, c_arg_t, decltype(visitora), decltype(visitorb), decltype(visitorc)>(*this, arg, visitora, visitorb, visitorc);
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
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
    auto visitora = [&]([[maybe_unused]] const typename SDR_t::id_type&, [[maybe_unused]] typename SDR_t::data_type&) {
        ++r;
    };
    auto visitorb = [&]([[maybe_unused]] const typename arg_t::id_type&, [[maybe_unused]] typename arg_t::data_type&) {
        ++r;
    };
    auto visitorc = [&](const typename SDR_t::id_type&, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
        typename SDR_t::data_type data = this_data.xorb(arg_data);
        if (data.rm_relevant()) {
            ++r;
        }
    };
    orop<arg_t, c_arg_t, decltype(visitora), decltype(visitorb), decltype(visitorc)>(*this, arg, visitora, visitorb, visitorc);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, typename VisitorC>
void SDR<SDR_t, container_t>::orv(const SDR<arg_t, c_arg_t>& query, VisitorA visitora, VisitorB visitorb, VisitorC visitorc) {
    orop<arg_t, c_arg_t, VisitorA, VisitorB, decltype(visitorc)>(*this, query, visitora, visitorb, visitorc);
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
                arg_t arg_elem = *arg_pos++;
                this_pos = lower_bound(this_pos, this_end, arg_elem.id, std::greater<SDR_t>());
                if (this_pos == this_end) goto end;
                SDR_t this_elem = *this_pos;
                if (this_elem == arg_elem) {
                    auto data = this_elem.data.rmb(arg_elem.data);
                    if (!data.rm_relevant()) {
                        this->v.erase((++this_pos).base());
                        if (this_end != this->v.rend()) {
                            // revalidate iterators
                            this_pos = this->v.rend() - (this_end - this_pos);
                            this_end = this->v.rend();
                        }
                    } else {
                        this_pos->data = data;
                        ++this_pos;
                    }
                    if (this_pos == this_end) goto end;
                    this_elem = *this_pos;
                }
                // =====
                if constexpr (!small_args) {
                    // get this in the arg
                    arg_pos = lower_bound(arg_pos, arg_end, this_elem.id, std::greater<arg_t>());
                    if (arg_pos == arg_end) goto end;
                    if (*arg_pos == this_elem) {
                        auto data = this_elem.data.rmb(arg_pos->data);
                        if (!data.rm_relevant()) {
                            this->v.erase((++this_pos).base());
                            if (this_end != this->v.rend()) {
                                // revalidate iterators
                                this_pos = this->v.rend() - (this_end - this_pos);
                                this_end = this->v.rend();
                            }
                        } else {
                            this_pos->data = data;
                            ++this_pos;
                        }
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
                auto data = this_pos->data.rmb(arg_pos->data);
                if (!data.rm_relevant()) {
                    ++this_pos;
                    // remove the element
                    this->v.erase_after(this_lagger);
                    --this->maybe_size.size;
                } else {
                    this_pos->data = data;
                    ++this_pos;
                    ++this_lagger;
                }
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
template <typename arg_t, typename c_arg_t, typename VisitorA, typename VisitorB, typename DumpEnd>
void SDR<SDR_t, container_t>::rmop(const SDR<SDR_t, container_t>& me, const SDR<arg_t, c_arg_t>& arg, VisitorA visitora, VisitorB visitorb, DumpEnd dump_end) {
    // this function can't place the result in the operands! see rmi instead
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
        arg_elem = *arg_pos;
        return true;
    };

    if (arg_pos == arg_end) goto dump_remaining_this;
    arg_elem = *arg_pos;

    if (this_pos == this_end) return;
    this_elem = *this_pos;

    while (true) {
        if (this_elem < arg_elem) {
            visitora(this_pos->id, *const_cast<typename SDR_t::data_type*>(&this_pos->data));
            ++this_pos;
            if (this_pos == this_end) return;
            this_elem = *this_pos;
        } else if (this_elem == arg_elem) {
            visitorb(this_pos->id, *const_cast<typename SDR_t::data_type*>(&this_pos->data), *const_cast<typename arg_t::data_type*>(&arg_pos->data));
            ++this_pos;
            ++arg_pos;
            if (!get_next_arg()) {
                goto dump_remaining_this;
            }
            if (this_pos == this_end) return;
            this_elem = *this_pos;
        } else {
            ++arg_pos;
            if (!get_next_arg()) {
                goto dump_remaining_this;
            }
        }
    }
    dump_remaining_this:
    dump_end(this_pos);
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::rmb(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&)> visitora;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitorb;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitora = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, ret_t(id, data));
        };
        visitorb = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.rmb(arg_data);
            if (data.rm_relevant()) {
                ++r.maybe_size.size;
                it = r.v.insert_after(it, ret_t(id, data));
            }
        };
    } else {
        visitora = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            r.push_back(ret_t(id, data));
        };
        visitorb = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            typename ret_t::data_type data = this_data.rmb(arg_data);
            if (data.rm_relevant()) {
                r.push_back(ret_t(id, data));
            }
        };
    }
    auto dump_end = [&](const_iterator pos) {
        while (pos != this->cend()) {
            visitora(pos->id, *const_cast<typename SDR_t::data_type*>(&pos->data));
            ++pos;
        }
        if constexpr (isVector<c_ret_t>::value) {
            r.v.shrink_to_fit();
        }
    };
    rmop(*this, arg, visitora, visitorb, dump_end);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitora = [&]([[maybe_unused]] const typename SDR_t::id_type&, [[maybe_unused]] typename SDR_t::data_type&) {
        ++r;
    };
    auto visitorb = [&]([[maybe_unused]] const typename SDR_t::id_type&, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
        typename SDR_t::data_type data = this_data.rmb(arg_data);
        if (data.rm_relevant()) {
            ++r;
        }
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
    rmop(*this, arg, visitora, visitorb, dump_end);
    return r; // nrvo
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename Visitor>
void SDR<SDR_t, container_t>::rmv(const SDR<arg_t, c_arg_t>& query, Visitor visitor) {
    auto dump_end = [&](const_iterator pos) {
        while (pos != this->cend()) {
            visitor(pos->id, *const_cast<typename SDR_t::data_type*>(&pos->data));
            ++pos;
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

} // namespace SparseDistributedRepresentation