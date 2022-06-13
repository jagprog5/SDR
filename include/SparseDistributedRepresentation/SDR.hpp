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
         * apply an and visitor. Perform an operation on each element in this AND in the arg.
         * each selected element pair is called in the visitor as visitor(iterator this_position, c_arg_t::iterator arg_position)
         * the visitor should not invalidate proceeding iterators (after this_position or arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename Visitor>
        void andv(SDR<arg_t, c_arg_t>& arg, Visitor visitor);
        
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
         *      the element only exists in this: this_visitor(iterator this_position)
         *      the element only exists in the arg: arg_visitor(c_arg_t::iterator arg_position)
         *      the element is in both: both_visitor(iterator this_position, c_arg_t::iterator arg_position)
         * the visitors must not invalidate the proceeding iterator (one after this_position or one after arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor>
        void orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor);

        /**
         * apply an or visitor. Perform an operation on each element in this OR in arg
         * three visitors are defined:
         *      the element only exists in this: this_visitor(iterator this_position)
         *      the element only exists in the arg: arg_visitor(c_arg_t::iterator arg_position)
         *      the element is in both: both_visitor(iterator this_position, c_arg_t::iterator arg_position)
         *
         * while applying the visitors, this function will reach a point where there doesn't exist any more elements in the arg.
         * at this point, this_back_position(SDR::iterator) is called with the curent position in this, and then this function exits.
         * similarly, if there are no more elements in this, then arg_back_position(c_arg_t::iterator) is called, followed by an exit.
         * 
         * the visitors must not invalidate the proceeding iterator (one after this_position or one after arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisBackPosition, typename ArgBackPosition>
        void orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisBackPosition this_back_position, ArgBackPosition arg_back_position);

        // Returns a copy of this which lacks any bit from arg.
        template<typename ret_t = SDR_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> rme(const SDR<arg_t, c_arg_t>& arg) const;

        // Remove inplace. Remove all bits in arg from this, then returns this.
        template<typename arg_t, typename c_arg_t>
        SDR<SDR_t, container_t>& rmi(const SDR<arg_t, c_arg_t>& arg);

        // Returns the number of elements in this that are not in arg.
        template<typename arg_t, typename c_arg_t>
        size_type rms(const SDR<arg_t, c_arg_t>& arg) const;

        /*
         * apply a rm visitor. Perform an operation on elements based on a arg.
         * visitors:
         *      the element only exists in this: this_visitor(const value_type::id_type&, value_type::data_type&)
         *      the element is in both: both_visitor(const value_type::id_type&, value_type::data_type&, arg_t::value_type::data_type&)   
         * 
         * the back_position is an iterator in this.
         * between the back_position and the end, there are no elements whose ids' also exist in the arg.
         * rather than calling these positions in this_visitor, the case can be optimized;
         * the position is made available in back_position(SDR::iterator)
         */
        // template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename BothVisitor, typename BackPosition>
        // void rmv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, BothVisitor both_visitor, BackPosition back_position);

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
    // at this point the start and end positions have been found
    // place the range into the result
    if constexpr(isVector<c_ret_t>::value) {
        sdr.v.resize(end_it - start_it);
    }
    if constexpr(isForwardList<c_ret_t>::value) {
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
void SDR<SDR_t, container_t>::andv(SDR<arg_t, c_arg_t>& arg, Visitor visitor) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    auto arg_pos = arg.v.begin();
    auto arg_end = arg.v.end();
    SDR_t this_elem;
    [[maybe_unused]] arg_t arg_elem;

    if (this_pos == this_end) return;
    while (true) {
        // get an element in this
        this_elem = *this_pos;
        // try to find the matching element in the arg
        if constexpr(isSet<c_arg_t>::value) {
            arg_pos = arg.v.lower_bound(this_elem.id);
        } else {
            arg_pos = std::lower_bound(arg_pos, arg_end, this_elem.id);
        }
        if (arg_pos == arg_end) return;
        // if the elements are equal, call the visitor
        if (*arg_pos == this_elem) {
            visitor(this_pos++, arg_pos++);
            if (arg_pos == arg_end) return;
        } else {
            ++this_pos;
        }
        // the rest of this is all of the above, except with this and arg swapped
        arg_elem = *arg_pos;
        if constexpr(usesSet) {
            this_pos = this->v.lower_bound(arg_elem.id);
        } else {
            this_pos = std::lower_bound(this_pos, this_end, arg_elem.id);
        }
        if (this_pos == this_end) return;
        if (*this_pos == arg_elem) {
            visitor(this_pos++, arg_pos++);
            if (this_pos == this_end) return;
        } else {
            ++arg_pos;
        }
    }
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::ande(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(typename container_t::iterator, typename c_arg_t::iterator)> visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.ande((typename SDR_t::data_type)arg_pos->data);
            if (data.relevant()) {
                ++r.maybe_size.size;
                ret_t elem(this_pos->id, data);
                it = r.v.insert_after(it, elem);
            }
        };
    } else {
        visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.ande((typename SDR_t::data_type)arg_pos->data);
            if (data.relevant()) {
                ret_t elem(this_pos->id, data);
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
        auto visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.ande((typename SDR_t::data_type)arg_pos->data);
            if (data.relevant()) {
                SDR_t elem(this_pos->id, data);
                *pos++ = elem;
            }
        };
        andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
        this->v.resize(pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        auto lagger = this->v.before_begin();
        auto this_visitor = [&](typename container_t::iterator) {
            // if the element only exists in this then it is removed
            this->v.erase_after(lagger);
            --this->maybe_size.size;
        };
        auto arg_visitor = [](typename c_arg_t::iterator) {};
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.ande((typename SDR_t::data_type)arg_pos->data);
            if (data.relevant()) {
                // the element is modified but not removed
                *this_pos = SDR_t(this_pos->id, data);
            } else {
                // the element is removed
                this->v.erase_after(lagger);
                --this->maybe_size.size;
                goto do_not_increment_lagger;
            }
            ++lagger;
            do_not_increment_lagger:
                (void)0;
        };
        orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    } else {
        auto this_visitor = [&](typename container_t::iterator this_pos) {
            // if the element only exists in this then it is removed
            this->v.erase(this_pos);
        };
        auto arg_visitor = [](typename c_arg_t::iterator) {};
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.ande((typename SDR_t::data_type)arg_pos->data);
            if (data.relevant()) {
                // the element is modified but not removed
                // const cast becuase std::set begin() is const
                // and we are modifying the element in a way that doesn't matter to std::set
                const_cast<SDR_t&>(*this_pos) = SDR_t(this_pos->id, data);
            } else {
                // the element is removed
                this->v.erase(this_pos);
            }
        };
        orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ands(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto data = this_pos->data.ande((typename SDR_t::data_type)arg_pos->data);
        if (data.relevant()) {
            ++r;
        }
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor>
void SDR<SDR_t, container_t>::orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor) {
    auto this_back_position = [&](iterator pos) {
        while (pos != this->cend()) {
            this_visitor(pos++);
        }
    };
    auto arg_back_position = [&](typename c_arg_t::iterator pos) {
        while (pos != arg.cend()) {
            arg_visitor(pos++);
        }
    };
    orv(arg, this_visitor, arg_visitor, both_visitor, this_back_position, arg_back_position);
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisBackPosition, typename ArgBackPosition>
void SDR<SDR_t, container_t>::orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisBackPosition this_back_position, ArgBackPosition arg_back_position) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    SDR_t this_val;
    auto arg_pos = arg.v.begin();
    auto arg_end = arg.v.end();
    arg_t arg_val;

    // helper lambdas
    // returns false if the function should exit
    auto get_this = [&]() {
        if (this_pos != this_end) {
            this_val = *this_pos;
        } else {
            arg_back_position(arg_pos);
            return false;
        }
        return true;
    };

    auto get_arg = [&]() {
        if (arg_pos != arg_end) {
            arg_val = *arg_pos;
        } else {
            this_back_position(this_pos);
            return false;
        }
        return true;
    };

    if (!get_this()) { return; }
    if (!get_arg()) { return; }

    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wmaybe-uninitialized")
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        #endif
    #endif

    while (true) {
        if (this_val.id < arg_val.id) {
            this_visitor(this_pos++);
            if (!get_this()) { return; }
        } else if (this_val.id > arg_val.id) {
            arg_visitor(arg_pos++);
            if (!get_arg()) { return; }
        } else {
            both_visitor(this_pos++, arg_pos++);
            if (!get_this()) { return; }
            if (!get_arg()) { return; }
        }
    }
    #pragma GCC diagnostic pop
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::ore(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(typename container_t::iterator)> this_visitor;
    std::function<void(typename c_arg_t::iterator)> arg_visitor;
    std::function<void(typename container_t::iterator, typename c_arg_t::iterator)> both_visitor;
    if constexpr(isForwardList<c_ret_t>::value) {
        auto it = r.v.before_begin();
        this_visitor = [&](typename container_t::iterator this_pos) {
            it = r.v.insert_after(it, ret_t(*this_pos));
            ++r.maybe_size.size;
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t(*arg_pos));
            ++r.maybe_size.size;
        };
        both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.ore((typename SDR_t::data_type)arg_pos->data);
            // there is no relevance check here, since it is assumed that elements which already exist in an SDR are relevant,
            // and that ore can only produce relevant elements from relevant elements
            ret_t elem(this_pos->id, data);
            it = r.v.insert_after(it, ret_t(this_pos->id, data));
            ++r.maybe_size.size;
        };
        const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    } else {
        this_visitor = [&](typename container_t::iterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            r.push_back(ret_t(*arg_pos));
        };
        both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.ore((typename SDR_t::data_type)arg_pos->data);
            ret_t elem(this_pos->id, data);
            r.push_back(elem);
        };
        const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::ori(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        SDR r = ore(arg);
        this->v = std::move(r.v);
    } else if constexpr(usesForwardList) {
        auto lagger = this->v.before_begin();
        auto this_visitor = [&](typename container_t::iterator) {
            ++lagger;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            this->v.insert_after(lagger, SDR_t(*arg_pos));
            ++this->maybe_size.size;
        };
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            this_pos->data = this_pos->data.ore((typename SDR_t::data_type)arg_pos->data);
            ++lagger;
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    } else {
        auto pos = this->v.begin();
        auto this_visitor = [&](typename container_t::iterator this_pos) {
            pos = this_pos;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            pos = this->v.insert(pos, SDR_t(*arg_pos));
        };
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.ore((typename SDR_t::data_type)arg_pos->data);
            const_cast<typename SDR_t::data_type&>(this_pos->data) = data;
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::ors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto this_visitor = [&](typename container_t::iterator) {
        ++r;
    };
    auto arg_visitor = [&](typename c_arg_t::iterator) {
        ++r;
    };
    auto both_visitor = [&](typename container_t::iterator, typename c_arg_t::iterator) {
        ++r;
    };
    auto this_back_handler = [&](typename container_t::iterator pos) {
        if constexpr(usesVector) {
            r += this->cend() - pos;
        } else {
            while (pos != this->cend()) {
                ++r;
                ++pos;
            }
        }
    };
    auto arg_back_handler = [&](typename c_arg_t::iterator pos){
        if constexpr(isVector<c_arg_t>::value) {
            r += arg.cend() - pos;
        } else {
            while (pos != arg.cend()) {
                ++r;
                ++pos;
            }
        }
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_handler, arg_back_handler);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::xore(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(typename container_t::iterator)> this_visitor;
    std::function<void(typename c_arg_t::iterator)> arg_visitor;
    std::function<void(typename container_t::iterator, typename c_arg_t::iterator)> both_visitor;
    // scoping weirdness requires declaring 'it' here
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        this_visitor = [&](typename container_t::iterator this_pos) {
            it = r.v.insert_after(it, ret_t(*this_pos));
            ++r.maybe_size.size;
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t(*arg_pos));
            ++r.maybe_size.size;
        };
        both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.xore((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                it = r.v.insert_after(it, ret_t(this_pos->id, data));
                ++r.maybe_size.size;
            }
        };
    } else {
        this_visitor = [&](typename container_t::iterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            r.push_back(ret_t(*arg_pos));
        };
        both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.xore((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id, data);
                r.push_back(elem);
            }
        };
    }
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::xori(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        SDR r = xore(arg);
        this->v = std::move(r.v);
    } else if constexpr(usesForwardList) {
        auto lagger = this->v.before_begin();
        auto this_visitor = [&](typename container_t::iterator) {
            ++lagger;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            lagger = this->v.insert_after(lagger, SDR_t(*arg_pos));
            ++this->maybe_size.size;
        };
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.xore((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                // not removed. just modified
                this_pos->data = this_pos->data.xore((typename SDR_t::data_type)arg_pos->data);
                ++lagger;
            } else {
                // removed
                this->v.erase_after(lagger);
                --this->maybe_size.size;
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    } else {
        auto pos = this->v.begin();
        auto this_visitor = [&](typename container_t::iterator this_pos) {
            pos = this_pos;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            pos = this->v.insert(pos, SDR_t(*arg_pos));
        };
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.xore((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                const_cast<typename SDR_t::data_type&>(this_pos->data) = data;
            } else {
                pos = this->v.erase(this_pos);
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::xors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto this_visitor = [&](typename container_t::iterator) {
        ++r;
    };
    auto arg_visitor = [&](typename c_arg_t::iterator) {
        ++r;
    };
    auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto data = this_pos->data.xore((typename SDR_t::data_type)arg_pos->data);
        if (data.rm_relevant()) {
            ++r;
        }
    };
    auto this_back_handler = [&](typename container_t::iterator pos) {
        if constexpr(usesVector) {
            r += this->cend() - pos;
        } else {
            while (pos != this->cend()) {
                ++r;
                ++pos;
            }
        }
    };
    auto arg_back_handler = [&](typename c_arg_t::iterator pos){
        if constexpr(isVector<c_arg_t>::value) {
            r += arg.cend() - pos;
        } else {
            while (pos != arg.cend()) {
                ++r;
                ++pos;
            }
        }
    };
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_handler, arg_back_handler);
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDR_t, container_t>& SDR<SDR_t, container_t>::rmi(const SDR<arg_t, c_arg_t>& arg) {
    auto arg_visitor = [&](typename c_arg_t::iterator) {};
    if constexpr(usesVector) {
        typename container_t::iterator insert_pos = this->v.begin();
        auto this_visitor = [&](typename container_t::iterator this_pos) {
            *insert_pos++ = *this_pos;
        };
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                // the element is modified but not removed
                *insert_pos++ = SDR_t(this_pos->id, data);
            } else {
                // the element is removed   
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
        this->v.resize(insert_pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        // lagger is used to remove elements
        typename container_t::iterator lagger = this->v.before_begin();
        auto this_visitor = [&](typename container_t::iterator) { ++lagger; };
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                // the element is modified but not removed
                *this_pos = SDR_t(this_pos->id, data);
            } else {
                // the element is removed
                this->v.erase_after(lagger);
                --this->maybe_size.size;
                goto do_not_increment_lagger;
            }
            ++lagger;
            do_not_increment_lagger:
                (void)0;
        };
        auto this_back_position = [](typename container_t::iterator) {};
        auto arg_back_position = [](typename c_arg_t::iterator) {};
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_position, arg_back_position);
    } else {
        auto this_visitor = [&](typename container_t::iterator) {};
        auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                // the element is modified but not removed
                // const_cast is required since std::set::begin() returns a const iter
                // this is fine since the element is not being modified in a way that the std::set cares
                const_cast<SDR_t&>(*this_pos) = SDR_t(this_pos->id, data);
            } else {
                // the element is removed
                this->v.erase(this_pos);
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDR_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDR_t, container_t>::rme(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(typename container_t::iterator)> this_visitor;
    std::function<void(typename c_arg_t::iterator)> arg_visitor;
    std::function<void(typename container_t::iterator, typename c_arg_t::iterator)> both_visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        this_visitor = [&](typename container_t::iterator this_pos) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator) {};
        both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                ++r.maybe_size.size;
                it = r.v.insert_after(it, ret_t(this_pos->id, data));
            }
        };
    } else {
        this_visitor = [&](typename container_t::iterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator) {};
        both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = (typename ret_t::data_type)this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
            if (data.rm_relevant()) {
                r.push_back(ret_t(this_pos->id, data));
            }
        };
    }
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    if constexpr(isVector<c_ret_t>::value) {
        r.v.shrink_to_fit();
    }
    return r;
}

template<typename SDR_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDR_t, container_t>::size_type SDR<SDR_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto this_visitor = [&](typename container_t::iterator) {
        ++r;
    };
    auto arg_visitor = [&](typename c_arg_t::iterator) {};
    auto both_visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto data = this_pos->data.rme((typename SDR_t::data_type)arg_pos->data);
        if (data.rm_relevant()) {
            ++r;
        }
    };
    auto this_back_handler = [&](typename container_t::iterator pos) {
        if constexpr(usesVector) {
            r += this->cend() - pos;
        } else {
            while (pos != this->cend()) {
                ++r;
                ++pos;
            }
        }
    };
    auto arg_back_handler = [](typename c_arg_t::iterator){};
    const_cast<SDR<SDR_t, container_t>&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_handler, arg_back_handler);
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