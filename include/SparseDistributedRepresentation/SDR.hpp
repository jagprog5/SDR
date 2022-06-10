#pragma once

#include <assert.h>
#include <initializer_list>
#include <cmath>
#include <algorithm>
#include <functional>

#include "SparseDistributedRepresentation/Templates.hpp"
#include "SparseDistributedRepresentation/SDR_t.hpp"

namespace sparse_distributed_representation {

/*
 * Inspired from ideas explained in this series:
 * https://youtu.be/ZDgCdWTuIzc
 * Numenta: SDR Capacity & Comparison (Episode 2)
 */
template<typename SDR_t = SDR_t<>, typename container_t = std::vector<SDR_t>>
class SDR {
    static_assert(!std::is_fundamental<SDR_t>::value, "Instead of SDR<fundamental_type_here>, use SDR<SDR_t<fundamental_type_here>>");
    
    public:
        using value_type = SDR_t;
        using container_type = container_t;
        using size_type = typename container_t::size_type;
        using iterator = typename container_t::iterator;
        using const_iterator = typename container_t::const_iterator;

        static constexpr bool usesVector = isVector<container_t>::value;
        static constexpr bool usesForwardList = isForwardList<container_t>::value;
        static constexpr bool usesSet = isSet<container_t>::value;

        // default ctor
        SDR() {
            if constexpr(usesForwardList)
                this->maybe_size.size = 0;
        }

        // copy ctor
        SDR(const SDR& sdr): v(sdr.v) {
            if constexpr(usesForwardList)
                this->maybe_size.size = sdr.maybe_size.size;
        }

        // copy assignment ctor
        SDR& operator=(const SDR& sdr) {
            this->v = sdr.v;
            if constexpr(usesForwardList)
                this->maybe_size.size = sdr.maybe_size.size;
            return *this;
        }

        // move ctor
        SDR(SDR&& sdr): v(std::move(sdr.v)) {
            if constexpr(usesForwardList)
                this->maybe_size.size = sdr.maybe_size.size;
        }

        // move assignment ctor
        SDR& operator=(SDR&& sdr) {
            this->v = std::move(sdr.v);
            if constexpr(usesForwardList)
                this->maybe_size.size = std::move(sdr.maybe_size.size);
            return *this;
        }

        // iterator ctor
        // note for if the iterators point to SDR_t elements:
        //      each element's data is NOT checked for relevance before insertion
        template<typename Iterator>
        SDR(Iterator begin, Iterator end);

        // initializer list ctor
        // if the list has SDR_t elements, then each element's data is checked for relevance before insertion
        template<typename T>
        SDR(std::initializer_list<T> list);

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
        template<typename RandomGenerator>
        SDR<SDR_t, container_t>& sample_portion(float amount, RandomGenerator& g);

        // apply a visitor. Perform an operation on each element in this,
        // The visitor arg is called with each element as visitor(const value_type::id_type&, value_type::data_type&)
        template<typename Visitor>
        void visitor(Visitor visitor);

        // and element. used for checking for the existence of an element.
        // if the element exists, returns a pointer to its data; else, returns null.
        const typename SDR_t::data_type* ande(typename SDR_t::id_type val) const;
        typename SDR_t::data_type* ande(typename SDR_t::id_type val);

        // and elements. returns the state of many elements.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> ande(const SDR<arg_t, c_arg_t>& arg) const;

        // and elements. returns the state of many elements from start to stop.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t>
        SDR<ret_t, c_ret_t> ande(arg_t start_inclusive, arg_t stop_exclusive) const;

        // and inplace. turn off all elements not in arg (compute arg AND this, and place the result in this). Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& andi(const SDR<arg_t, c_arg_t>& arg);

        // and size. returns the number of elements in both this and arg.
        template<typename arg_t, typename c_arg_t>
        size_type ands(const SDR<arg_t, c_arg_t>& arg) const;

        // and size. returns the number of elements from start to stop.
        template<typename arg_t>
        size_type ands(arg_t start_inclusive, arg_t stop_exclusive) const;
        
        /**
         * apply an and visitor. Perform an operation on each element in this AND in the query.
         * each selected element pair is called in the visitor as visitor(const value_type::id_type&, value_type::data_type&, arg_t::value_type::data_type&)
         */
        template<typename arg_t, typename c_arg_t, typename Visitor>
        void andv(SDR<arg_t, c_arg_t>& query, Visitor visitor);
        
        // or elements.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> ore(const SDR<arg_t, c_arg_t>& arg) const;

        // or inplace. turn on all elements in arg. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& ori(const SDR<arg_t, c_arg_t>& arg);

        // or size. returns the number of elements in this or arg.
        template<typename arg_t, typename c_arg_t>
        size_type ors(const SDR<arg_t, c_arg_t>& arg) const;

        // xor elements.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> xore(const SDR<arg_t, c_arg_t>& arg) const;

        // xor inplace. computes this xor arg, and places the result in this. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& xori(const SDR<arg_t, c_arg_t>& arg);

        // xor size, aka hamming distance. returns the number of elements in this xor arg.
        template<typename arg_t, typename c_arg_t>
        size_type xors(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * apply an or visitor. Perform an operation on each element in this OR in arg
         * three visitors are defined:
         *      the element only exists in this: visitor_this(const value_type::id_type&, value_type::data_type&)
         *      the element only exists in the query: visitor_query(const arg_t::value_type::id_type&, arg_t::value_type::data_type&)
         *      the element is in both: visitor_both(const value_type::id_type&, value_type::data_type&, arg_t::value_type::data_type&)
         */
        template<typename arg_t, typename c_arg_t, typename VisitorThis, typename VisitorQuery, typename VisitorBoth>
        void orv(SDR<arg_t, c_arg_t>& query, VisitorThis visitor_this, VisitorQuery visitor_query, VisitorBoth visitor_both);

        // Returns a copy of this which lacks any bit from arg.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> rme(const SDR<arg_t, c_arg_t>& arg) const;

        // Remove inplace. Remove all bits in arg from this, then returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& rmi(const SDR<arg_t, c_arg_t>& arg);

        // Returns the number of elements in this that are not in arg.
        template<typename arg_t, typename c_arg_t>
        size_type rms(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * apply a rm visitor. Perform an operation on elements based on a query.
         *  visitors:
         *      the element only exists in this: visitor_this(const value_type::id_type&, value_type::data_type&)
         *      the element is in both: visitor_both(const value_type::id_type&, value_type::data_type&, arg_t::value_type::data_type&)     
         */
        template<typename arg_t, typename c_arg_t, typename VisitorThis, typename VisitorBoth>
        void rmv(SDR<arg_t, c_arg_t>& query, VisitorThis visitor_this, VisitorBoth visitor_both);

        /*
         * apply a rm visitor. Perform an operation on elements based on a query.
         * visitors:
         *      the element only exists in this: visitor_this(const value_type::id_type&, value_type::data_type&)
         *      the element is in both: visitor_both(const value_type::id_type&, value_type::data_type&, arg_t::value_type::data_type&)   
         * 
         * the back_position is an iterator in this.
         * between the back_position and the end, there are no elements whose ids' also exist in the arg.
         * rather than calling these positions in visitor_this, the case can be optimized;
         * the position is made available in back_position(SDR::iterator)
         */
        template<typename arg_t, typename c_arg_t, typename VisitorThis, typename VisitorBoth, typename BackPosition>
        void rmv(SDR<arg_t, c_arg_t>& query, VisitorThis visitor_this, VisitorBoth visitor_both, BackPosition back_position);

        // Returns this, shifted by amount.
        SDR<SDR_t, container_t>& shift(int amount);

        // concatenate an SDR to an SDR. Every element in arg must be greater than every element in this. Returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& append(const SDR<arg_t, c_arg_t>& arg);

        auto cbegin() const { return v.cbegin(); }
        auto cend() const { return v.cend(); }
        auto begin() const { return v.cbegin(); }
        auto end() const { return v.cend(); }
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

        // calls push_back or insert (at the end) on the underlying container
        // assert checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        void push_back(const E& i);

        template<typename T = container_t, typename E>
        void push_back(E&& i);

        // calls pop_front on the forward_list underlying container
        // assertion checks not empty
        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, void>::type pop_front();

        // calls push_front on the forward_list underlying container
        // assertion checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        typename std::enable_if<isForwardList<T>::value, void>::type push_front(const E& i);

        template<typename T = container_t, typename E>
        typename std::enable_if<isForwardList<T>::value, void>::type push_front(E&& i);

        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, const_iterator>::type before_begin() { return v.before_begin(); }

        // calls insert_after on the forward_list underlying container
        // assertion checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        typename std::enable_if<isForwardList<T>::value, const_iterator>::type insert_after(const_iterator pos, const E& i);

        template<typename T = container_t, typename E>
        typename std::enable_if<isForwardList<T>::value, const_iterator>::type insert_after(const_iterator pos, E&& i);

        template<typename SDR_t_inner, typename container_t_inner>
        friend std::ostream& operator<<(std::ostream& os, const SDR<SDR_t_inner, container_t_inner>& sdr);

        template<typename other>
        auto operator&(const other& o) { return ande(o); }
        template<typename other>
        auto operator&(const other& o) const { return ande(o); }
        template<typename other>
        auto operator&&(const other& o) const { return ands(o); }
        template<typename other>
        auto operator&=(const other& o) { return andi(o); }
        template<typename other>
        auto operator*(const other& o) const { return ande(o); }
        template<typename other>
        auto operator*=(const other& o) { return andi(o); }
        template<typename other>
        auto operator|(const other& o) const { return ore(o); }
        template<typename other>
        auto operator||(const other& o) const { return ors(o); }
        template<typename other>
        auto operator|=(const other& o) { return ori(o); }
        template<typename other>
        auto operator^(const other& o) const { return xore(o); }
        template<typename other>
        auto operator^=(const other& o) { return xori(o); }
        template<typename other>
        auto operator+(const other& o) const { return ore(o); }
        template<typename other>
        auto operator+=(const other& o) { return ori(o); }
        template<typename other>
        auto operator-(const other& o) const { return rme(o); }
        template<typename other>
        auto operator-=(const other& o) { return rmi(o); }
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
                if (this_elem.id != other_elem.id) return false;
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

    private:
        container_t v;

        void assert_ascending();

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

        MaybeSize<container_t> maybe_size;

        // used in the output stream op
        static constexpr bool print_type = false;

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
                if (prev_elem.id >= elem.id) {
                    assert(!"Elements must be in ascending order and with no duplicates.");
                }
                prev_elem = elem;
            }   
        #endif
    }
}

template<typename SDR_t, typename container_t>
template<typename Iterator>
SDR<SDR_t, container_t>::SDR(Iterator begin, Iterator end) {
    if constexpr(usesForwardList) {
        this->maybe_size.size = 0;
        auto insert_it = this->v.before_begin();
        while (begin != end) {
            insert_it = this->v.insert_after(insert_it, *begin++);
            ++this->maybe_size.size;
        }
    } else if constexpr(usesVector && std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>::value) {
        auto count = end - begin;
        this->v.resize(count);
        for (decltype(count) i = 0; i < count; ++i) {
            this->v[i] = *begin++; 
        }
    } else {
        while (begin != end) {
            if constexpr(usesVector) {
                v.push_back(*begin++);
            } else {
                v.insert(v.end(), *begin++);
            }
        }
    }
}

template<typename SDR_t, typename container_t>
template<typename T>
SDR<SDR_t, container_t>::SDR(std::initializer_list<T> list) {
    if constexpr(usesForwardList) {
        auto pos = list.begin();
        auto end = list.end();
        this->maybe_size.size = 0;
        auto insert_it = this->v.before_begin();
        while (pos != end) {
            T elem = *pos++;
            bool relevant;
            if constexpr(std::is_same<T, typename SDR_t::id_type>::value) {
                relevant = true;
            } else {
                relevant = elem.data.relevant();
            }
            if (relevant) {
                insert_it = this->v.insert_after(insert_it, SDR_t(elem));
                ++this->maybe_size.size;
            }
        }
    } else {
        for (const auto& elem : list) {
            bool relevant;
            if constexpr(std::is_same<T, typename SDR_t::id_type>::value) {
                relevant = true;
            } else {
                relevant = elem.data.relevant();
            }
            if (relevant) {
                if constexpr(usesVector) {
                    this->v.push_back(SDR_t(elem));
                } else {
                    this->v.insert(this->v.end(), SDR_t(elem));
                }
            }
        }
    }

    assert_ascending();
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
            auto insert_it = v.before_begin();
            for (size_type i = 0; i < wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDR_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDR_t(start_index + i));
            }
        } else if constexpr(usesVector) {
            for (size_type i = 0; i < wrapped_elements; ++i) {
                v[i] = SDR_t(i);
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                v[i + wrapped_elements] = SDR_t(start_index + i);
            }
        } else {
            for (size_type i = 0; i < wrapped_elements; ++i) {
                v.insert(v.end(), SDR_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                v.insert(v.end(), SDR_t(start_index + i));
            }
        }
    } else {
        // no elements are wrapped from the end
        if constexpr(usesForwardList) {
            auto insert_it = v.before_begin();
            for (size_type i = 0; i < size; ++i) {
                insert_it = v.insert_after(insert_it, SDR_t(start_index + i));
            }
        } else if constexpr(usesVector) {
            for (size_type i = 0; i < size; ++i) {
                v[i] = SDR_t(start_index + i);
            }
        } else {
            for (size_type i = 0; i < size; ++i) {
                v.insert(v.end(), SDR_t(start_index + i));
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
        auto insert_it = v.before_begin();
        for (size_type i = 0; i < size; ++i) {
            insert_it = v.insert_after(insert_it, start_index + i);
        }
    } else if constexpr(usesVector) {
        for (size_type i = 0; i < size; ++i) {
            v[i] = SDR_t(start_index + i);
        }
    } else {
        for (size_type i = 0; i < size; ++i) {
            v.insert(v.end(), SDR_t(start_index + i));
        }
    }
    if constexpr(usesForwardList) {
        maybe_size.size = size;
    }
}

template<typename SDR_t, typename container_t>
template<typename RandomGenerator>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::sample_portion(float amount, RandomGenerator& g) {
    assert(amount >= 0 && amount <= 1);
    auto check_val = amount * (float)g.max();
    if constexpr(usesForwardList) {
        typename container_t::size_type remove_count = 0;
        auto pos = v.before_begin();
        while (true) {
            auto next = std::next(pos);
            if (next == v.end()) break;
            if (g() >= check_val) {
                v.erase_after(pos);
                ++remove_count;
            } else {
                pos = next;
            }
        }
        maybe_size.size -= remove_count;
    } else if constexpr(usesVector) {
        auto to_offset = v.begin();
        auto from_offset = v.cbegin();
        auto end = v.end();
        while (from_offset != end) {
            if (g() < check_val) {
                *to_offset++ = *from_offset;
            }
            from_offset++;
        }
        v.resize(to_offset - v.cbegin());
    } else {
        auto pos = v.begin();
        while (pos != v.end()) {
            if (g() >= check_val) {
                pos = v.erase(pos);
            } else {
                ++pos;
            }
        }
    } 
    return *this;
}

template<typename SDR_t, typename container_t>
const typename SDR_t::data_type* SDR<SDR_t, container_t>::ande(typename SDR_t::id_type val) const {
    decltype(std::lower_bound(v.cbegin(), v.cend(), val)) pos;
    if constexpr(usesSet) {
        pos = v.lower_bound(val);
    } else {
        pos = std::lower_bound(v.cbegin(), v.cend(), val);
    }
    if (pos == v.cend() || pos->id != val) {
        return NULL;
    } else {
        return &pos->data;
    }
}

template<typename SDR_t, typename container_t>
typename SDR_t::data_type* SDR<SDR_t, container_t>::ande(typename SDR_t::id_type val) {
    // reuse above
    return const_cast<typename SDR_t::data_type*>(static_cast<const SDR<SDR_t, container_t>&>(*this).ande(val));
}

template<typename SDR_t, typename container_t>
template<typename Visitor>
void SDR<SDR_t, container_t>::visitor(Visitor visitor) {
    for (auto& elem : this->v) {
        visitor(elem.id, elem.data);
    }
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::ande(arg_t start_inclusive, arg_t stop_exclusive) const {
    assert(start_inclusive <= stop_exclusive);
    SDR<ret_t, c_ret_t> sdr;
    typename container_t::const_iterator start_it;
    if constexpr(usesSet) {
        start_it = v.lower_bound(start_inclusive);
    } else {
        start_it = std::lower_bound(v.cbegin(), v.cend(), start_inclusive);
    }
    typename container_t::const_iterator end_it;
    if constexpr(usesForwardList) {
        end_it = start_it;
        while (true) {
            if (end_it == cend() || end_it->id >= stop_exclusive) {
                break;
            }
            ++end_it;
            ++sdr.maybe_size.size;
        }
    } else if constexpr(usesSet) {
        end_it = v.lower_bound(stop_exclusive);
    } else {
        end_it = std::lower_bound(start_it, v.cend(), stop_exclusive);
    }
    if constexpr(isVector<c_ret_t>::value) {
        sdr.v.resize(end_it - start_it);
    }
    if constexpr (isForwardList<c_ret_t>::value) {
        auto insert_it = sdr.v.before_begin();
        for (auto it = start_it; it != end_it; ++it) {
            insert_it = sdr.v.insert_after(insert_it, (ret_t)*it);
        }
    } else if constexpr(isVector<c_ret_t>::value) {
        auto insert_it = sdr.v.begin();
        for (auto it = start_it; it != end_it; ++it) {
            *insert_it++ = (ret_t)*it;
        }
    }  else {
        for (auto it = start_it; it != end_it; ++it) {
            sdr.v.insert(sdr.v.end(), (ret_t)*it);
        }
    } 
    return sdr; // nrvo
}

template<typename SDR_t, typename container_t>
template<typename arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ands(arg_t start_inclusive, arg_t stop_exclusive) const {
    SDR sdr;
    if constexpr(usesSet) {
        auto pos = this->v.lower_bound(start_inclusive);
        size_type count = 0;
        while (pos != v.cend()) {
            if (pos++->id >= stop_exclusive) {
                break;
            }
            ++count;
        }
        return count;
    } else if constexpr(usesVector) {
        auto pos = std::lower_bound(cbegin(), cend(), start_inclusive);
        auto end_it = std::lower_bound(pos, cend(), stop_exclusive);
        return (size_type)(end_it - pos);
    } else {
        size_type count = 0;
        auto pos = std::lower_bound(cbegin(), cend(), start_inclusive);
        while (pos != cend() && pos++->id < stop_exclusive) {
            ++count;
        }
        return count;
    }
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename Visitor>
void SDR<SDR_t, container_t>::andv(SDR<arg_t, c_arg_t>& query, Visitor visitor) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    auto query_pos = query.v.begin();
    auto query_end = query.v.end();
    SDR_t this_elem;
    [[maybe_unused]] arg_t query_elem;

    if (this_pos == this_end) return;
    while (true) {
        this_elem = *this_pos;
        if constexpr(isSet<c_arg_t>::value) {
            query_pos = query.v.lower_bound(this_elem.id);
        } else {
            query_pos = std::lower_bound(query_pos, query_end, this_elem.id);
        }
        if (query_pos == query_end) return;
        if (*query_pos == this_elem) {
            visitor(this_pos->id, const_cast<typename SDR_t::data_type&>(this_pos->data), const_cast<typename arg_t::data_type&>(query_pos->data));
            ++query_pos;
            if (query_pos == query_end) return;
        }
        ++this_pos;
        // ============= a and b swapped ===^=V================
        query_elem = *query_pos;
        if constexpr(usesSet) {
            this_pos = this->v.lower_bound(query_elem.id);
        } else {
            this_pos = std::lower_bound(this_pos, this_end, query_elem.id);
        }
        if (this_pos == this_end) return;
        this_elem = *this_pos;
        if (this_elem == query_elem) {
            visitor(this_pos->id, const_cast<typename SDR_t::data_type&>(this_pos->data), const_cast<typename arg_t::data_type&>(query_pos->data));
            ++this_pos;
            if (this_pos == this_end) return;
        }
        ++query_pos;
    }
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::ande(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.ande((typename SDR_t::data_type)arg_data);
            if (data.relevant()) {
                ++r.maybe_size.size;
                ret_t elem(this_id, data);
                it = r.v.insert_after(it, elem);
            }
        };
    } else {
        visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.ande((typename SDR_t::data_type)arg_data);
            if (data.relevant()) {
                ret_t elem(this_id, data);
                r.push_back(elem);
            }
        };
    }
    const_cast<SDR<SDR_t, container_t>&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::andi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        auto pos = this->v.begin();
        auto visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = this_data.ande((typename SDR_t::data_type)arg_data);
            if (data.relevant()) {
                SDR_t elem(this_id, data);
                *pos++ = elem;
            }
        };
        andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
        this->v.resize(pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        size_type i = 0;
        auto lagger = this->v.before_begin();
        auto pos = this->v.begin();
        auto visitor = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = this_data.ande((typename SDR_t::data_type)arg_data);
            if (data.relevant()) {
                SDR_t elem(this_id, data);
                ++lagger;
                ++i;
                *pos++ = elem;
            }
        };
        andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
        while (std::next(lagger) != v.end()) {
            v.erase_after(lagger);
        }
        this->maybe_size.size = i;
    } else {
        SDR r = ande(arg);
        this->v = std::move(r.v);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ands(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&](const typename SDR_t::id_type&, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
        auto data = this_data.ande((typename SDR_t::data_type)arg_data);
        if (data.relevant()) {
            ++r;
        }
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename VisitorThis, typename VisitorQuery, typename VisitorBoth>
void SDR<SDR_t, container_t>::orv(SDR<arg_t, c_arg_t>& query, VisitorThis visitor_this, VisitorQuery visitor_query, VisitorBoth visitor_both) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    bool this_valid = true;
    SDR_t this_val;
    auto query_pos = query.v.begin();
    auto query_end = query.v.end();
    bool query_valid = true;
    arg_t query_val;

    if (this_pos != this_end) this_val = *this_pos; else this_valid = false; // get from this, or update this_valid if no more elements
    if (query_pos != query_end) query_val = *query_pos; else query_valid = false; // query
    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wmaybe-uninitialized")
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        #endif
    #endif
    
    while (this_valid || query_valid) {
        if ((this_valid && !query_valid) || (this_valid && query_valid && this_val.id < query_val.id)) {
            visitor_this(this_pos->id, const_cast<typename SDR_t::data_type&>(this_pos->data));
            ++this_pos;
            if (this_pos != this_end) this_val = *this_pos; else this_valid = false; // this
        } else if ((!this_valid && query_valid) || (this_valid && query_valid && this_val.id > query_val.id)) {
            visitor_query(query_pos->id, const_cast<typename arg_t::data_type&>(query_pos->data));
            ++query_pos;
            if (query_pos != query_end) query_val = *query_pos; else query_valid = false; // query
        } else {
            visitor_both(this_pos->id, const_cast<typename SDR_t::data_type&>(this_pos->data), const_cast<typename arg_t::data_type&>(query_pos->data));
            ++this_pos;
            ++query_pos;
            if (this_pos != this_end) this_val = *this_pos; else this_valid = false; // this
            if (query_pos != query_end) query_val = *query_pos; else query_valid = false; // query
        }
    }
    #pragma GCC diagnostic pop
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::ore(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&)> visitor_this;
    std::function<void(const typename arg_t::id_type&, typename arg_t::data_type&)> visitor_query;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitor_both;
    if constexpr(isForwardList<c_ret_t>::value) {
        auto it = r.v.before_begin();
        visitor_this = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            it = r.v.insert_after(it, ret_t(id, data));
            ++r.maybe_size.size;
        };
        visitor_query = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
            ++r.maybe_size.size;
        };
        visitor_both = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.ore((typename SDR_t::data_type)arg_data);
            // there is no relevance check here, since it is assumed that elements which already exist in an SDR are relevant,
            // and that ore can only produce relevant elements from relevant elements
            ret_t elem(this_id, data);
            it = r.v.insert_after(it, ret_t(this_id, data));
            ++r.maybe_size.size;
        };
        const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_query, visitor_both);
    } else {
        visitor_this = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            r.push_back(ret_t(id, data));
        };
        visitor_query = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            r.push_back(ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
        };
        visitor_both = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.ore((typename SDR_t::data_type)arg_data);
            ret_t elem(this_id, data);
            r.push_back(elem);
        };
        const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_query, visitor_both);
    }
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::ori(const SDR<arg_t, c_arg_t>& arg) {
    SDR r = ore(arg);
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
    auto visitor_this = [&](const typename SDR_t::id_type&, typename SDR_t::data_type&) {
        ++r;
    };
    auto visitor_query = [&](const typename arg_t::id_type&, typename arg_t::data_type&) {
        ++r;
    };
    auto visitor_both = [&](const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&) {
        ++r;
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_query, visitor_both);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::xore(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&)> visitor_this;
    std::function<void(const typename arg_t::id_type&, typename arg_t::data_type&)> visitor_query;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitor_both;
    // scoping weirdness requires it declared here
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitor_this = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            it = r.v.insert_after(it, ret_t(id, data));
            ++r.maybe_size.size;
        };
        visitor_query = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
            ++r.maybe_size.size;
        };
        visitor_both = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.xore((typename SDR_t::data_type)arg_data);
            if (data.rm_relevant()) {
                it = r.v.insert_after(it, ret_t(this_id, data));
                ++r.maybe_size.size;
            }
        };
    } else {
        visitor_this = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            r.push_back(ret_t(id, data));
        };
        visitor_query = [&](const typename arg_t::id_type& id, typename arg_t::data_type& data) {
            r.push_back(ret_t((typename ret_t::id_type)id, (typename ret_t::data_type)data));
        };
        visitor_both = [&](const typename SDR_t::id_type& this_id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.xore((typename SDR_t::data_type)arg_data);
            if (data.rm_relevant()) {
                ret_t elem(this_id, data);
                r.push_back(elem);
            }
        };
    }
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_query, visitor_both);
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::xori(const SDR<arg_t, c_arg_t>& arg) {
    SDR r = xore(arg);
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
    auto visitor_this = [&]([[maybe_unused]] const typename SDR_t::id_type&, [[maybe_unused]] typename SDR_t::data_type&) {
        ++r;
    };
    auto visitor_query = [&]([[maybe_unused]] const typename arg_t::id_type&, [[maybe_unused]] typename arg_t::data_type&) {
        ++r;
    };
    auto visitor_both = [&](const typename SDR_t::id_type&, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
        auto data = this_data.xore((typename SDR_t::data_type)arg_data);
        if (data.rm_relevant()) {
            ++r;
        }
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_query, visitor_both);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::rmi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        // a forward list can't iterate backward
        constexpr bool forward = isForwardList<c_arg_t>::value;

        auto get_arg_begin = [&arg](){
            if constexpr(forward) {
                return arg.cbegin(); 
            } else {
                return arg.crbegin(); 
            }
        };

        auto get_arg_end = [&arg]() {
            if constexpr(forward) {
                return arg.cend();
            } else {
                return arg.crend();
            }
        };

        auto get_this_begin = [this]() {
            if constexpr(forward) {
                return this->v.begin();
            } else {
                return this->v.rbegin();
            }
        };

        auto get_this_end = [this]() {
            if constexpr(forward) {
                return this->v.end();
            } else {
                return this->v.rend();
            }
        };

        auto arg_pos = get_arg_begin();
        auto arg_end = get_arg_end();
        auto this_pos = get_this_begin();
        auto this_end = get_this_end();
        if (arg_pos == arg_end) goto end;
        while (true) {
            arg_t arg_elem = *arg_pos++;
            this_pos = std::lower_bound(this_pos, this_end, arg_elem.id, lesser_or_greater<forward, SDR_t>());
            if (this_pos == this_end) goto end;
            SDR_t this_elem = *this_pos;
            if (this_elem.id == arg_elem.id) {
                auto data = this_elem.data.rme((typename SDR_t::data_type)arg_elem.data);
                if (!data.rm_relevant()) {
                    if constexpr(forward) {
                        this->v.erase(this_pos++);
                    } else {
                        this->v.erase((++this_pos).base());
                    }
                    if (this_end != get_this_end()) {
                        // revalidate iterators
                        this_pos = get_this_end() - (this_end - this_pos);
                        this_end = get_this_end();
                    }
                } else {
                    this_pos->data = data;
                    ++this_pos;
                }
                if (this_pos == this_end) goto end;
                this_elem = *this_pos;
            }
            // =====
            // get this in the arg
            arg_pos = std::lower_bound(arg_pos, arg_end, this_elem.id, lesser_or_greater<forward, arg_t>());
            if (arg_pos == arg_end) goto end;
            if (arg_pos->id == this_elem.id) {
                auto data = this_elem.data.rme((typename SDR_t::data_type)arg_pos->data);
                if (!data.rm_relevant()) {
                    if constexpr(forward) {
                        this->v.erase(this_pos++);
                    } else {
                        this->v.erase((++this_pos).base());
                    }
                    if (this_end != get_this_end()) {
                        // revalidate iterators
                        this_pos = get_this_end() - (this_end - this_pos);
                        this_end = get_this_end();
                    }
                } else {
                    this_pos->data = data;
                    ++this_pos;
                }
            }
        }
        end:
        this->v.shrink_to_fit();
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
            if (this_elem.id < arg_elem.id) {
                ++this_lagger;
                ++this_pos;
                if (this_pos == this_end) break;
                this_elem = *this_pos;
            } else if (this_elem.id == arg_elem.id) {
                auto data = this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
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
    } else {
        // revert back to normal remove, with a swap 
        SDR r = this->rme(arg);
        this->v = std::move(r.v);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename VisitorThis, typename VisitorBoth>
void SDR<SDR_t, container_t>::rmv(SDR<arg_t, c_arg_t>& query, VisitorThis visitor_this, VisitorBoth visitor_both) {
    auto back_handler = [&](iterator pos) {
        while (pos != this->cend()) {
            visitor_this(pos->id, const_cast<typename SDR_t::data_type&>(pos->data));
            ++pos;
        }
    };
    rmv(query, visitor_this, visitor_both, back_handler);
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename VisitorThis, typename VisitorBoth, typename BackHandler>
void SDR<SDR_t, container_t>::rmv(SDR<arg_t, c_arg_t>& arg, VisitorThis visitor_this, VisitorBoth visitor_both, BackHandler back_handler) {
    auto arg_pos = arg.v.begin();
    auto arg_end = arg.v.end();
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    arg_t arg_elem;
    SDR_t this_elem;

    auto get_next_arg = [&]() -> bool {
        arg_pos = std::lower_bound(arg_pos, arg_end, this_elem.id);
        if (arg_pos == arg_end) return false;
        arg_elem = *arg_pos;
        return true;
    };

    if (arg_pos == arg_end) goto dump_remaining_this;
    arg_elem = *arg_pos;

    if (this_pos == this_end) return;
    this_elem = *this_pos;

    while (true) {
        if (this_elem.id < arg_elem.id) {
            visitor_this(this_pos->id, const_cast<typename SDR_t::data_type&>(this_pos->data));
            ++this_pos;
            if (this_pos == this_end) return;
            this_elem = *this_pos;
        } else if (this_elem.id == arg_elem.id) {
            visitor_both(this_pos->id, const_cast<typename SDR_t::data_type&>(this_pos->data), const_cast<typename arg_t::data_type&>(arg_pos->data));
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
    back_handler(this_pos);
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::rme(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&)> visitor_this;
    std::function<void(const typename SDR_t::id_type&, typename SDR_t::data_type&, typename arg_t::data_type&)> visitor_both;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitor_this = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, ret_t(id, data));
        };
        visitor_both = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.rme((typename SDR_t::data_type)arg_data);
            if (data.rm_relevant()) {
                ++r.maybe_size.size;
                it = r.v.insert_after(it, ret_t(id, data));
            }
        };
    } else {
        visitor_this = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& data) {
            r.push_back(ret_t(id, data));
        };
        visitor_both = [&](const typename SDR_t::id_type& id, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
            auto data = (typename ret_t::data_type)this_data.rme((typename SDR_t::data_type)arg_data);
            if (data.rm_relevant()) {
                r.push_back(ret_t(id, data));
            }
        };
    }
    const_cast<SDR<SDR_t, container_t>&>(*this).rmv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_both);
    if constexpr (isVector<c_ret_t>::value) {
        r.v.shrink_to_fit();
    }
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor_this = [&]([[maybe_unused]] const typename SDR_t::id_type&, [[maybe_unused]] typename SDR_t::data_type&) {
        ++r;
    };
    auto visitor_both = [&]([[maybe_unused]] const typename SDR_t::id_type&, typename SDR_t::data_type& this_data, typename arg_t::data_type& arg_data) {
        auto data = this_data.rme((typename SDR_t::data_type)arg_data);
        if (data.rm_relevant()) {
            ++r;
        }
    };
    auto back_handler = [&](iterator pos) {
        if constexpr (usesVector) {
            r += this->cend() - pos;
        } else {
            while (pos != this->cend()) {
                ++r;
                ++pos;
            }
        }
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).rmv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor_this, visitor_both, back_handler);
    return r; // nrvo
}

template<typename SDR_t, typename container_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::shift(int amount) {
    for (auto& elem : v) {
        #ifdef NDEBUG
        const_cast<typename SDR_t::id_type&>(elem.id) += amount;
        #else
        assert(!__builtin_add_overflow(amount, elem.id, const_cast<typename SDR_t::id_type*>(&elem.id)));
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
        auto it = v.before_begin();
        while (true) {
            auto next = std::next(it);
            if (next == v.end()) break;
            it = next;
        }
        // 'it' is before end
        assert(empty() || arg.empty() || *it < *arg.v.cbegin());
        for (auto e : arg.v) {
            it = v.insert_after(it, e);
            ++this->maybe_size.size;
        }
    } else if constexpr(usesVector) {
        auto old_size = this->v.size();
        this->v.resize(this->v.size() + arg.v.size());
        auto insert_it = this->v.begin() + old_size;
        for (auto it = arg.v.begin(); it != arg.v.end(); ++it) {
            *insert_it++ = *it;
        }
    } else {
        for (auto e : arg.v) { push_back(e); }
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename T, typename E>
void SDR<SDR_t, container_t>::push_back(const E& i) {
    SDR_t elem = SDR_t(i);
    assert(v.empty() || v.crbegin()->id < elem.id);
    if constexpr(usesVector) {
        v.push_back(elem);
    } else {
        v.insert(v.end(), elem);
    }
}

template<typename SDR_t, typename container_t>
template<typename T, typename E>
void SDR<SDR_t, container_t>::push_back(E&& i) {
    SDR_t elem = SDR_t(i);
    assert(v.empty() || v.crbegin()->id < elem.id);
    if constexpr(usesVector) {
        v.push_back(elem);
    } else {
        v.insert(v.end(), elem);
    }
}

template<typename SDR_t, typename container_t>
template<typename T>
typename std::enable_if<isForwardList<T>::value, void>::type SDR<SDR_t, container_t>::pop_front() {
    assert(!v.empty());
    v.pop_front();
    --maybe_size.size;
}

template<typename SDR_t, typename container_t>
template<typename T, typename E>
typename std::enable_if<isForwardList<T>::value, void>::type SDR<SDR_t, container_t>::push_front(const E& i)  {
    SDR_t elem = SDR_t(i);
    assert(v.empty() || v.cbegin()->id > elem.id);
    ++maybe_size.size;
    v.push_front(elem);
}

template<typename SDR_t, typename container_t>
template<typename T, typename E>
typename std::enable_if<isForwardList<T>::value, void>::type SDR<SDR_t, container_t>::push_front(E&& i)  {
    SDR_t elem = SDR_t(i);
    assert(v.empty() || v.cbegin()->id > elem.id);
    ++maybe_size.size;
    v.push_front(elem);
}

template<typename SDR_t, typename container_t>
template<typename T, typename E>
typename std::enable_if<isForwardList<T>::value, typename container_t::const_iterator>::type SDR<SDR_t, container_t>::insert_after(const_iterator pos, const E& i) {
    SDR_t elem = SDR_t(i);
    assert(pos == before_begin() || elem.id > pos->id);
    auto ret = v.insert_after(pos, elem);
    ++maybe_size.size;
    auto next = std::next(ret);
    assert(next == cend() || elem.id < next->id);
    return ret;
}

template<typename SDR_t, typename container_t>
template<typename T, typename E>
typename std::enable_if<isForwardList<T>::value, typename container_t::const_iterator>::type SDR<SDR_t, container_t>::insert_after(const_iterator pos, E&& i) {
    SDR_t elem = SDR_t(i);
    assert(pos == before_begin() || elem.id > pos->id);
    auto ret = v.insert_after(pos, elem);
    ++maybe_size.size;
    auto next = std::next(ret);
    assert(next == cend() || elem.id < next->id);
    return ret;
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
        if (std::next(it) != end) os << ",";
    }
    os << ']';
    return os;
}

} // namespace sparse_distributed_representation