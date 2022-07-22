#pragma once

#include <assert.h>
#include <initializer_list>
#include <cmath>
#include <algorithm>
#include <functional>
#include <queue>

#include "SparseDistributedRepresentation/Templates.hpp"
#include "SparseDistributedRepresentation/SDRElem.hpp" 
#include "SparseDistributedRepresentation/ArrayAdaptor.hpp" // used in transpose

namespace sparse_distributed_representation {

/**
 * An SDR or sparse vector.
 * 
 * Inspired from ideas explained in this series:
 * https://youtu.be/ZDgCdWTuIzc
 * Numenta: SDR Capacity & Comparison (Episode 2)
 * 
 * @tparam SDRElem_t The elements, which have an id, and optionally data associated with the id.
 * @tparam container_t The underlying container for the SDRElem_t elements.
 */
template<typename SDRElem_t = SDRElem<>, typename container_t = std::vector<SDRElem_t>>
class SDR {
    public:
        using value_type = SDRElem_t;
        using container_type = container_t;
        using size_type = typename container_t::size_type;
        using iterator = typename container_t::iterator;
        using const_iterator = typename container_t::const_iterator;

        static constexpr bool usesVector = isVector<container_t>::value;
        static constexpr bool usesForwardList = isForwardList<container_t>::value;
        static constexpr bool usesSet = isSet<container_t>::value;

        static_assert(!std::is_fundamental<SDRElem_t>::value, "Instead of SDR<fundamental_type_here>, use SDR<SDRElem_t<fundamental_type_here>>");
        static_assert(!usesSet || setComparatorCheck<container_t>::value, "Bad comparator for std::set! instead of std::less<T>, use std::less<>");

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
        SDR(SDR&& sdr) noexcept : v(std::move(sdr.v)) {
            if constexpr(usesForwardList)
                this->maybe_size.size = sdr.maybe_size.size;
        }

        // move assignment ctor
        SDR& operator=(SDR&& sdr) noexcept {
            this->v = std::move(sdr.v);
            if constexpr(usesForwardList)
                this->maybe_size.size = std::move(sdr.maybe_size.size);
            return *this;
        }

        /**
         * Iterator ctor.
         * 
         * If the iterators point to SDRElem_t elements then the elements are NOT checked for relevance before insertion.
         * 
         * @param begin Iterator to the first element to insert.
         * @param end Iterator to one past the last element to insert.
         */
        template<typename Iterator>
        SDR(Iterator begin, Iterator end);

        /**
         * Initializer list ctor.
         * 
         * @param list If the list has SDRElem_t elements, then each element's data is checked for relevance before insertion.
         */
        template<typename T>
        SDR(std::initializer_list<T> list);

        /**
         * Encode a float as an SDR.
         * 
         * @param input the float to encode. Should be from 0 to 1 inclusively. Must be non-negative. 
         * @param size the size of the instantiated SDR result. 
         * @param underlying_array_length the size of the corresponding dense representation. 
         */
        SDR(float input, size_type size, size_type underlying_array_length);

        /**
         * Encode a periodic float as an SDR.
         * 
         * @param input the float to encode. Must be non-negative.
         * @param period encodes the input such that it wraps back to 0 as it approaches a multiple of the period. Must be non-negative.
         * @param size the size of the instantiated SDR result.
         * @param underlying_array_length the size of the corresponding dense representation.
         */
        SDR(float input, float period, size_type size, size_type underlying_array_length);

        /**
         * sample. Each element has a chance of being removed.
         * 
         * @param amount 0 always clears the sdr, and 1 nearly always leaves it unchanged.
         * @param g The random generator. e.g. a std::mt19937 instance.
         * @return Ref to this.
         */
        template<typename RandomGenerator>
        SDR& sample(float amount, RandomGenerator& g);

        /**
         * apply a visitor on all elements.
         * 
         * @param visitor A functor that can be called as visitor(iterator)
         */
        template<typename Visitor>
        void visitor(Visitor visitor);

        /**
         * and element. used for checking for the existence of an element.
         * 
         * @param val The element's id.
         * @return A pointer to the element's data, or null if the id is not in this.
         */
        const typename SDRElem_t::data_type* ande(typename SDRElem_t::id_type val) const;
        typename SDRElem_t::data_type* ande(typename SDRElem_t::id_type val);

        /**
         * and elements. query the state of many elements.
         * 
         * @return An SDR that contains elements in common between this and the arg (combined accordingly if the element is in both).
         */
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> ande(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * and elements. query for a range of elements based on a start and stop id
         * 
         * @return As SDR that contains all elements in the specified range in this.
         */
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t>
        SDR<ret_t, c_ret_t> ande(arg_t start_inclusive, arg_t stop_exclusive) const;

        /**
         * and inplace. Computes arg AND this, and place the result in this.
         * 
         * @return Ref to this.
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& andi(const SDR<arg_t, c_arg_t>& arg);

        /**
         * and size
         * 
         * @return returns the number of elements in common between this and arg.
         */
        template<typename arg_t, typename c_arg_t>
        size_type ands(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * and size.
         * 
         * @return the number of elements in this from start to stop.
         */
        template<typename arg_t>
        size_type ands(arg_t start_inclusive, arg_t stop_exclusive) const;
        
        /**
         * apply an and visitor. Perform an operation on each element in this AND in the arg.
         * each selected element pair is called in the visitor as visitor(iterator this_position, c_arg_t::iterator arg_position)
         * the visitor should not invalidate proceeding iterators (after this_position or arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename Visitor>
        void andv(SDR<arg_t, c_arg_t>& arg, Visitor visitor);
        
        /**
         * or elements.
         * 
         * @return The combined elements in this and the arg.
         */
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> ore(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * or inplace. Insert elements into this, or combine elements if they are already in this.
         * 
         * TODO add const lval and rval overload, iters?
         * 
         * @return Ref to this.
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& ori(const SDR<arg_t, c_arg_t>& arg);

        /**
         * or inplace. Insert a single element into this (generic specialization).
         * 
         * If an element already exists in the insertion position, then its data is combined with arg's data.
         * 
         * @arg combine How is the arg combined with a pre-existing element.
         * @arg start_point Attempt to insert the arg at this position first, then try proceeding positions.
         * @return The insertion position.
         */
        template<typename T = container_t, typename Combine, typename id_t, typename data_t>
        typename std::enable_if<!isForwardList<T>::value, typename T::const_iterator>::type ori(SDRElem<id_t, data_t>&& arg, Combine combine, const_iterator start_point);

        template<typename T = container_t>
        typename std::enable_if<!isForwardList<T>::value, void>::type ori(SDRElem_t&& arg);

        /**
         * or inplace. Insert a single element into this (forward_list specialization).
         * 
         * If an element already exists in the insertion position, then its data is combined with arg's data.
         * 
         * @arg combine How is the arg combined with a pre-existing element.
         * @arg start_point Attempt to insert the arg after this position, then try proceeding positions.
         * @return The insertion position.
         */
        template<typename T = container_t, typename Combine, typename id_t, typename data_t>
        typename std::enable_if<isForwardList<T>::value, typename T::const_iterator>::type ori(SDRElem<id_t, data_t>&& arg, Combine combine, const_iterator start_point);

        template<typename T = container_t>
        typename std::enable_if<isForwardList<T>::value, void>::type ori(SDRElem_t&& arg);

        /**
         * or size
         * 
         * @return the number of elements in this or arg.
         */
        template<typename arg_t, typename c_arg_t>
        size_type ors(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * xor elements
         * 
         * @return An SDR that contains elements only in this or only in the arg (or combined accordingly if the element is in both).
         */
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> xore(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * xor inplace. computes this xor arg, and places the result in this.
         * 
         * @return Ref to this.
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& xori(const SDR<arg_t, c_arg_t>& arg);

        /**
         * xor size, aka hamming distance

         * @return returns the number of elements in this xor arg 
         */
        template<typename arg_t, typename c_arg_t>
        size_type xors(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * apply an or visitor. Perform an operation on each element in this OR in arg
         * three visitors are defined:
         *      the element is only in this: this_visitor(iterator this_position)
         *      the element is only in the arg: arg_visitor(c_arg_t::iterator arg_position)
         *      the element is in both: both_visitor(iterator this_position, c_arg_t::iterator arg_position)
         * the visitors must not invalidate proceeding iterators (one this_position or one arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor>
        void orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor);

        /**
         * apply an or visitor. Perform an operation on each element in this OR in arg
         * three visitors are defined:
         *      the element is only in this: this_visitor(iterator this_position)
         *      the element is only in the arg: arg_visitor(c_arg_t::iterator arg_position)
         *      the element is in both: both_visitor(iterator this_position, c_arg_t::iterator arg_position)
         *
         * while applying the visitors, this function will reach a point where there doesn't exist any more elements in the arg.
         * at this point, this_back_position(iterator) is called with the curent position in this, and then this function exits.
         * similarly, if there are no more elements in this, then arg_back_position(c_arg_t::iterator) is called, followed by an exit.
         * 
         * the visitors must not invalidate proceeding iterators (one this_position or one arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisBackPosition, typename ArgBackPosition>
        void orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisBackPosition this_back_position, ArgBackPosition arg_back_position);

        /**
         * remove elements.
         * 
         * @return A copy of this with each element in the arg removed from it.
         */
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> rme(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * remove inplace. Remove all elements in arg from this.
         * 
         * @return Ref to this.
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& rmi(const SDR<arg_t, c_arg_t>& arg);

        /**
         * remove size.
         * 
         * @return Returns the number of elements in this that are not in arg. 
         */
        template<typename arg_t, typename c_arg_t>
        size_type rms(const SDR<arg_t, c_arg_t>& arg) const;

        /**
         * Shift the elements in this.
         * 
         * @param amount increments each element's id.
         * @return Ref to this.
         */
        SDR<SDRElem_t, container_t>& shift(int amount);

        /**
         * concatenate an SDR to an SDR
         * 
         * @param arg Every element in arg must be greater than every element in this.
         * @return Ref to this
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& append(SDR<arg_t, c_arg_t>&& arg);

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

        void clear() noexcept { v.clear(); }

        template<typename T = container_t>
        void reserve(size_type n) { v.reserve(n); }

        template<typename T = container_t>
        typename T::const_reverse_iterator crbegin() const { return v.crbegin(); }

        template<typename T = container_t>
        typename T::const_reverse_iterator crend() const { return v.crend(); }

        // calls push_back or insert (at the end) on the underlying container
        // assert checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        void push_back(E&& i);

        // calls insert on the underlying container
        // assert checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        const_iterator insert(const_iterator position, E&& i);

        // calls erase on the underlying container
        template<typename T = container_t>
        const_iterator erase(const_iterator position) { v.erase(position); };

        // calls erase on the underlying container
        template<typename T = container_t>
        const_iterator erase(const_iterator first, const_iterator last) { v.erase(first, last); };

        // calls pop_front on the forward_list underlying container
        // assertion checks not empty
        template<typename T = container_t>
        void pop_front();

        // calls push_front on the forward_list underlying container
        // assertion checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        void push_front(E&& i);

        template<typename T = container_t>
        const_iterator before_begin() { return v.before_begin(); }
        template<typename T = container_t>
        const_iterator cbefore_begin() { return v.cbefore_begin(); }

        // calls insert_after on the forward_list underlying container
        // assertion checks that the inserted element is in order and with no duplicates
        template<typename T = container_t, typename E>
        const_iterator insert_after(const_iterator pos, E&& i);

        // calls erase_after on the forward_list underlying container
        template<typename T = container_t>
        const_iterator erase_after(const_iterator pos) {
            this->maybe_size.size -= 1;
            return v.erase_after(pos);
        }

        // calls erase_after on the forward_list underlying container
        template<typename T = container_t>
        const_iterator erase_after(const_iterator first, const_iterator last) {
            while (first != last) {
                this->maybe_size.size -= 1;
                v.erase_after(first++);
            }
            return last;
        }

        // calls lower_bound on the set underlying container
        template<typename T = container_t>
        const_iterator lower_bound(const value_type& val) const {
            return v.lower_bound(val);
        }

        template<typename SDRElem_t_inner, typename container_t_inner>
        friend std::ostream& operator<<(std::ostream& os, const SDR<SDRElem_t_inner, container_t_inner>& sdr);

        template<typename other>
        auto operator&(other&& o) { return ande(o); } // non-const version for ande(id_type)
        template<typename other>
        auto operator&(other&& o) const { return ande(o); }
        template<typename other>
        auto operator&&(other&& o) const { return ands(o); }
        template<typename other>
        auto operator&=(other&& o) { return andi(o); }
        template<typename other>
        auto operator*(other&& o) const { return ande(o); }
        template<typename other>
        auto operator*=(other&& o) { return andi(o); }
        template<typename other>
        auto operator|(other&& o) const { return ore(o); }
        template<typename other>
        auto operator||(other&& o) const { return ors(o); }
        template<typename other>
        auto operator|=(other&& o) { return ori(o); }
        template<typename other>
        auto operator^(other&& o) const { return xore(o); }
        template<typename other>
        auto operator^=(other&& o) { return xori(o); }
        template<typename other>
        auto operator+(other&& o) const { return ore(o); }
        template<typename other>
        auto operator+=(other&& o) { return ori(o); }
        template<typename other>
        auto operator-(other&& o) const { return rme(o); }
        template<typename other>
        auto operator-=(other&& o) { return rmi(o); }
        template<typename other>
        auto operator<<(other&& o) const { return SDR(*this).shift(o); }
        template<typename other>
        auto operator>>(other&& o) const { return SDR(*this).shift(-o); }
        template<typename other>
        auto operator<<=(other&& o) { return shift(o); }
        template<typename other>
        auto operator>>=(other&& o) { return shift(-o); }
        
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
                if (this_elem.id() != other_elem.id()) return false;
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

        // dot product
        template<typename ret_t = typename SDRElem_t::data_type, typename arg_t, typename c_arg_t>
        ret_t dot(const SDR<arg_t, c_arg_t>& other) const;

        enum MatrixFormat { ROW_MAJOR, COLUMN_MAJOR };

        /**
         * matrix-vector multiplication
         * 
         * @param this a row-major matrix (an SDR, which contains each row as an SDR, which contains the row elements).
         * @param arg a column vector input.
         * @return a row vector result.
         */
        template<MatrixFormat format = ROW_MAJOR, typename arg_t, typename ret_t = arg_t, typename c_arg_t, typename c_ret_t = c_arg_t>
        SDR<ret_t, c_ret_t> matrix_vector_mul(const SDR<arg_t, c_arg_t>& arg) const;

        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t>
        SDR<ret_t, c_ret_t> matrix_transpose() const;

        // SDR& matrix_transpose_inplace();

        template<typename T = SDRElem_t>
        typename T::data_type::value_type::data_type matrix_trace() const;

        /**
         * matrix-matrix multiplication
         * 
         * this, the arg, and the output are row-major matrices
         */
        // template<typename arg_t, typename ret_t = arg_t, typename c_arg_t, typename c_ret_t = c_arg_t>
        // SDR<ret_t, c_ret_t> matrix_matrix_mul(const SDR<arg_t, c_arg_t>& arg) const;
        
    private:
        container_t v;

        void assert_ascending();

        MaybeSize<container_t> maybe_size;

        // used in the output stream op
        static constexpr bool print_type = false;

        template<typename friend_SDRElem_t, typename friend_container_t>
        friend class SDR;

        // relevance is needed for interface compatability between SDRs and SDRElem::data_type
        bool relevant() const { return !empty(); }
        bool rm_relevant() const { return relevant(); }
};

template<typename SDRElem_t, typename container_t>
void SDR<SDRElem_t, container_t>::assert_ascending() {
    if constexpr(!usesSet) {
        #ifndef NDEBUG
            if (v.empty()) return;
            auto pos = v.begin();
            auto end = v.end();
            SDRElem_t prev_elem = *pos++;
            while (pos != end) {
                SDRElem_t elem = *pos++;
                if (prev_elem.id() >= elem.id()) {
                    assert(!"Elements must be in ascending order and with no duplicates.");
                }
                prev_elem = elem;
            }   
        #endif
    }
}

template<typename SDRElem_t, typename container_t>
template<typename Iterator>
SDR<SDRElem_t, container_t>::SDR(Iterator begin, Iterator end) {
    if constexpr(usesForwardList) {
        this->maybe_size.size = 0;
        auto insert_it = this->v.before_begin();
        while (begin != end) {
            insert_it = this->v.insert_after(insert_it, SDRElem_t(*begin++)); // ctor here since the iters might point to id_type elements
            ++this->maybe_size.size;
        }
    } else if constexpr(usesVector && std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>::value) {
        auto count = end - begin;
        this->v.resize(count);
        for (decltype(count) i = 0; i < count; ++i) {
            this->v[i] = SDRElem_t(*begin++); 
        }
    } else {
        while (begin != end) {
            if constexpr(usesVector) {
                v.push_back(*begin++);
            } else {
                v.insert(v.end(), SDRElem_t(*begin++));
            }
        }
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T>
SDR<SDRElem_t, container_t>::SDR(std::initializer_list<T> list) {
    if constexpr(usesForwardList) {
        auto pos = list.begin();
        auto end = list.end();
        this->maybe_size.size = 0;
        auto insert_it = this->v.before_begin();
        while (pos != end) {
            T elem = *pos++;
            bool relevant;
            if constexpr(std::is_same<T, typename SDRElem_t::id_type>::value) {
                relevant = true;
            } else {
                relevant = elem.data().relevant();
            }
            if (relevant) {
                insert_it = this->v.insert_after(insert_it, SDRElem_t(elem));  // ctor here since the iters might point to id_type elements
                ++this->maybe_size.size;
            }
        }
    } else {
        for (const auto& elem : list) {
            bool relevant;
            if constexpr(std::is_same<T, typename SDRElem_t::id_type>::value) {
                relevant = true;
            } else {
                relevant = elem.data().relevant();
            }
            if (relevant) {
                if constexpr(usesVector) {
                    this->v.push_back(SDRElem_t(elem));
                } else {
                    this->v.insert(this->v.end(), SDRElem_t(elem));
                }
            }
        }
    }

    assert_ascending();
}

template<typename SDRElem_t, typename container_t>
SDR<SDRElem_t, container_t>::SDR(float input, float period, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length && period >= 0 && input >= 0);
    if constexpr(usesVector) v.resize(size);

    float progress = input / period;
    // NOLINTNEXTLINE
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
                insert_it = v.insert_after(insert_it, SDRElem_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDRElem_t(start_index + i));
            }
        } else if constexpr(usesVector) {
            for (size_type i = 0; i < wrapped_elements; ++i) {
                v[i] = SDRElem_t(i);
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                v[i + wrapped_elements] = SDRElem_t(start_index + i);
            }
        } else {
            for (size_type i = 0; i < wrapped_elements; ++i) {
                v.insert(v.end(), SDRElem_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                v.insert(v.end(), SDRElem_t(start_index + i));
            }
        }
    } else {
        // no elements are wrapped from the end
        if constexpr(usesForwardList) {
            auto insert_it = v.before_begin();
            for (size_type i = 0; i < size; ++i) {
                insert_it = v.insert_after(insert_it, SDRElem_t(start_index + i));
            }
        } else if constexpr(usesVector) {
            for (size_type i = 0; i < size; ++i) {
                v[i] = SDRElem_t(start_index + i);
            }
        } else {
            for (size_type i = 0; i < size; ++i) {
                v.insert(v.end(), SDRElem_t(start_index + i));
            }
        }
    }
    if constexpr(usesForwardList) {
        maybe_size.size = size;
    }
}

template<typename SDRElem_t, typename container_t>
SDR<SDRElem_t, container_t>::SDR(float input, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length);
    assert(input >= 0);
    if constexpr(usesVector) v.resize(size);
    size_type start_index = std::round((underlying_array_length - size) * input);
    if constexpr(usesForwardList) {
        auto insert_it = v.before_begin();
        for (size_type i = 0; i < size; ++i) {
            insert_it = v.insert_after(insert_it, SDRElem_t(start_index + i));
        }
    } else if constexpr(usesVector) {
        for (size_type i = 0; i < size; ++i) {
            v[i] = SDRElem_t(start_index + i);
        }
    } else {
        for (size_type i = 0; i < size; ++i) {
            v.insert(v.end(), SDRElem_t(start_index + i));
        }
    }
    if constexpr(usesForwardList) {
        maybe_size.size = size;
    }
}

template<typename SDRElem_t, typename container_t>
template<typename RandomGenerator>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::sample(float amount, RandomGenerator& g) {
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

template<typename SDRElem_t, typename container_t>
const typename SDRElem_t::data_type* SDR<SDRElem_t, container_t>::ande(typename SDRElem_t::id_type val) const {
    decltype(std::lower_bound(v.cbegin(), v.cend(), val)) pos;
    if constexpr(usesSet) {
        pos = v.lower_bound(val);
    } else {
        pos = std::lower_bound(v.cbegin(), v.cend(), val);
    }
    if (pos == v.cend() || pos->id() != val) {
        return NULL;
    } else {
        return &pos->data();
    }
}

template<typename SDRElem_t, typename container_t>
typename SDRElem_t::data_type* SDR<SDRElem_t, container_t>::ande(typename SDRElem_t::id_type val) {
    // reuse above
    return const_cast<typename SDRElem_t::data_type*>(const_cast<const SDR<SDRElem_t, container_t>&>(*this).ande(val));
}

template<typename SDRElem_t, typename container_t>
template<typename Visitor>
void SDR<SDRElem_t, container_t>::visitor(Visitor visitor) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    while (this_pos != this_end) {
        visitor(this_pos++);
    }
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::ande(arg_t start_inclusive, arg_t stop_exclusive) const {
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
            if (end_it == cend() || end_it->id() >= stop_exclusive) {
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

template<typename SDRElem_t, typename container_t>
template<typename arg_t>
// NOLINTNEXTLINE
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::ands(arg_t start_inclusive, arg_t stop_exclusive) const {
    if constexpr(usesSet) {
        auto pos = this->v.lower_bound(start_inclusive);
        size_type count = 0;
        while (pos != v.cend()) {
            if (pos++->id() >= stop_exclusive) {
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
        while (pos != cend() && pos++->id() < stop_exclusive) {
            ++count;
        }
        return count;
    }
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename Visitor>
void SDR<SDRElem_t, container_t>::andv(SDR<arg_t, c_arg_t>& arg, Visitor visitor) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    auto arg_pos = arg.v.begin();
    auto arg_end = arg.v.end();
    const SDRElem_t* this_elem;
    [[maybe_unused]] const arg_t* arg_elem;

    if (this_pos == this_end) return;
    while (true) {
        // get an element in this
        this_elem = &*this_pos;
        // try to find the matching element in the arg
        if constexpr(isSet<c_arg_t>::value) {
            arg_pos = arg.v.lower_bound(this_elem->id());
        } else {
            arg_pos = std::lower_bound(arg_pos, arg_end, this_elem->id());
        }
        if (arg_pos == arg_end) return;
        // if the elements are equal, call the visitor
        if (*arg_pos == *this_elem) {
            visitor(this_pos++, arg_pos++);
            if (arg_pos == arg_end) return;
        } else {
            ++this_pos;
        }
        // the rest of this is all of the above, except with this and arg swapped
        arg_elem = &*arg_pos;
        if constexpr(usesSet) {
            this_pos = this->v.lower_bound(arg_elem->id());
        } else {
            this_pos = std::lower_bound(this_pos, this_end, arg_elem->id());
        }
        if (this_pos == this_end) return;
        if (*this_pos == *arg_elem) {
            visitor(this_pos++, arg_pos++);
            if (this_pos == this_end) return;
        } else {
            ++arg_pos;
        }
    }
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::ande(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(iterator, typename c_arg_t::iterator)> visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template ande<typename ret_t::data_type>(arg_pos->data());
            if (data.relevant()) {
                ++r.maybe_size.size;
                ret_t elem(this_pos->id(), std::move(data));
                it = r.v.insert_after(it, std::move(elem));
            }
        };
    } else {
        visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template ande<typename ret_t::data_type>(arg_pos->data());
            if (data.relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                r.push_back(std::move(elem));
            }
        };
    }
    const_cast<SDR<SDRElem_t, container_t>&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return r; // nrvo 
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::andi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        auto pos = this->v.begin();
        auto visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const typename SDRElem_t::data_type& data = this_pos->data().andi(arg_pos->data());
            if (data.relevant()) {
                SDRElem_t elem(this_pos->id(), std::move(data));
                *pos++ = std::move(elem);
            }
        };
        andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
        this->v.resize(pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        auto lagger = this->v.before_begin();
        auto this_visitor = [&](iterator) {
            // if the element only exists in this then it is removed
            this->v.erase_after(lagger);
            --this->maybe_size.size;
        };
        auto arg_visitor = [](typename c_arg_t::iterator) {};
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const typename SDRElem_t::data_type& data = this_pos->data().andi(arg_pos->data());
            if (!data.relevant()) {
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
        auto this_visitor = [&](iterator this_pos) {
            // if the element only exists in this then it is removed
            this->v.erase(this_pos);
        };
        auto arg_visitor = [](typename c_arg_t::iterator) {};
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const typename SDRElem_t::data_type& data = const_cast<typename SDRElem_t::data_type&>(this_pos->data()).andi(arg_pos->data());
            if (!data.relevant()) {
                // the element is removed
                this->v.erase(this_pos);
            }
        };
        orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::ands(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto count = this_pos->data().ands(arg_pos->data());
        // if the SDRElem_t::data_type is an SDR, then the "size" is literally the SDR size, and an empty SDR is not relevant
        // if the SDRElem_t::data_type is a normal data type (UnitData, etc.) then the "size" is just an alias for relevance
        if (count) ++r;
    };
    const_cast<SDR&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return r; // nrvo 
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor>
void SDR<SDRElem_t, container_t>::orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor) {
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

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisBackPosition, typename ArgBackPosition>
void SDR<SDRElem_t, container_t>::orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisBackPosition this_back_position, ArgBackPosition arg_back_position) {
    auto this_pos = this->v.begin();
    auto this_end = this->v.end();
    SDRElem_t this_val;
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
        if (this_val.id() < arg_val.id()) {
            this_visitor(this_pos++);
            if (!get_this()) { return; }
        } else if (this_val.id() > arg_val.id()) {
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

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::ore(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(iterator)> this_visitor;
    std::function<void(typename c_arg_t::iterator)> arg_visitor;
    std::function<void(iterator, typename c_arg_t::iterator)> both_visitor;
    if constexpr(isForwardList<c_ret_t>::value) {
        auto it = r.v.before_begin();
        this_visitor = [&](iterator this_pos) {
            it = r.v.insert_after(it, ret_t(*this_pos));
            ++r.maybe_size.size;
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            // it is assumed that creating ret_t data from arg_t data does not change the relevance
            // (meaning that there is no need to check for relevance here).
            // rational: elements which are already relevant (since it already exists in the arg SDR)
            // should not become irrelevant after changing to a different type.
            it = r.v.insert_after(it, ret_t(*arg_pos));
            ++r.maybe_size.size;
        };
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template ore<typename ret_t::data_type>(arg_pos->data());
            // there is no relevance check here, since it is assumed that elements which already exist in an SDR are relevant,
            // and that ore can only produce relevant elements from relevant elements
            ret_t elem(this_pos->id(), std::move(data));
            it = r.v.insert_after(it, std::move(elem));
            ++r.maybe_size.size;
        };
        const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    } else {
        this_visitor = [&](iterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            r.push_back(ret_t(*arg_pos));
        };
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template ore<typename ret_t::data_type>(arg_pos->data());
            ret_t elem(this_pos->id(), std::move(data));
            r.push_back(std::move(elem));
        };
        const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::ori(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        SDR r = ore(arg);
        this->v = std::move(r.v);
    } else if constexpr(usesForwardList) {
        auto lagger = this->v.before_begin();
        auto this_visitor = [&](iterator) {
            ++lagger;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            this->v.insert_after(lagger, SDRElem_t(*arg_pos));
            ++this->maybe_size.size;
        };
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            this_pos->data().ori(arg_pos->data());
            ++lagger;
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    } else {
        auto pos = this->v.begin();
        auto this_visitor = [&](iterator this_pos) {
            pos = this_pos;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            pos = this->v.insert(pos, SDRElem_t(*arg_pos));
        };
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const_cast<typename SDRElem_t::data_type&>(this_pos->data()).ori(arg_pos->data());
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

// auto id = arg.id();
// auto pos = v.lower_bound(id); // TODO use a std::find here  
// if (pos == v.cend() || pos->id() != id) {
//     // the element does not yet exist. add it
//     if constexpr(!std::is_same_v<SDRElem<id_t, data_t>, SDRElem_t>) {
//         // in matrix_transpose, we might be allocating as an array. convert it if necessary
//         SDRElem_t correct(arg.id());
//         auto arg_pos = std::make_move_iterator(arg.data().begin());
//         auto arg_end = std::make_move_iterator(arg.data().end());
//         while (arg_pos != arg_end) {
//             correct.data().push_back(*arg_pos++);
//         }
//         pos = v.insert(pos, std::move(correct));
//     } else {
//         pos = v.insert(pos, std::move(arg));
//     }
// } else {
//     // no relevance check since ori shouldn't make something not relevant
//     combine(pos, std::move(arg));
// }

template<typename SDRElem_t, typename container_t>
template<typename T, typename Combine, typename id_t, typename data_t>
typename std::enable_if<!isForwardList<T>::value, typename T::const_iterator>::type SDR<SDRElem_t, container_t>::ori(SDRElem<id_t, data_t>&& arg, Combine combine, const_iterator pos) {
    auto id = arg.id();
    if constexpr(usesSet) {
        if (pos == v.cend() || pos->id() != id) {
            pos = this->v.insert(pos, std::move(arg));
        } else {
            const_cast<typename SDRElem_t::data_type&>(pos->data()).ori(arg.data());
        }
        return pos;
    } else {
        pos = std::lower_bound(pos, v.cend(), id);
        if (pos == v.cend() || pos->id() != id) {
            // the element does not yet exist. add it
            if constexpr(!std::is_same_v<SDRElem<id_t, data_t>, SDRElem_t>) {
                // in matrix_transpose, we might be allocating as an array. convert it if necessary
                SDRElem_t correct(arg.id());
                auto arg_pos = std::make_move_iterator(arg.data().begin());
                auto arg_end = std::make_move_iterator(arg.data().end());
                while (arg_pos != arg_end) {
                    correct.data().push_back(*arg_pos++);
                }
                pos = v.insert(pos, std::move(correct));
            } else {
                pos = v.insert(pos, std::move(arg));
            }
        } else {
            combine(pos, std::move(arg));
        }
        return pos;
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T>
typename std::enable_if<!isForwardList<T>::value, void>::type SDR<SDRElem_t, container_t>::ori(SDRElem_t&& arg) {
    if constexpr(usesSet) {
        auto pos = v.lower_bound(arg.id());
        if (pos == v.cend() || pos->id() != arg.id()) {
            this->v.insert(pos, std::move(arg));
        } else {
            const_cast<typename SDRElem_t::data_type&>(pos->data()).ori(arg.data());
        }
    } else {
        // forward to the other overload
        auto combine = [](const_iterator existing_element, SDRElem_t&& arg) {
            const_cast<typename SDRElem_t::data_type&>(existing_element->data()).ori(arg.data());
        };
        this->ori(std::move(arg), combine, this->cbegin());
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename Combine, typename id_t, typename data_t>
typename std::enable_if<isForwardList<T>::value, typename T::const_iterator>::type SDR<SDRElem_t, container_t>::ori(SDRElem<id_t, data_t>&& arg, Combine combine, const_iterator start_point) {
    auto id = arg.id();
    const_iterator lagger = start_point;
    const_iterator pos = std::next(lagger);
    while (pos != v.end()) {
        if (pos->id() >= id) break;
        ++pos;
        ++lagger;
    }
    if (pos == v.cend() || pos->id() != id) {
        ++maybe_size.size;
        if constexpr(!std::is_same_v<SDRElem<id_t, data_t>, SDRElem_t>) {
            // in matrix_transpose, we might be allocating as an array. convert it if necessary
            SDRElem_t correct(arg.id());
            auto arg_pos = std::make_move_iterator(arg.data().begin());
            auto arg_end = std::make_move_iterator(arg.data().end());
            while (arg_pos != arg_end) {
                correct.data().push_back(*arg_pos++);
            }
            return v.insert_after(lagger, std::move(correct));
        } else {
            return v.insert_after(lagger, std::move(arg));
        }
    } else {
        combine(pos, std::move(arg));
        return pos;
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T>
typename std::enable_if<isForwardList<T>::value, void>::type SDR<SDRElem_t, container_t>::ori(SDRElem_t&& arg) {
    auto combine = [](const_iterator existing_element, SDRElem_t&& arg) {
        const_cast<typename SDRElem_t::data_type&>(existing_element->data()).ori(arg.data());
    };
    this->ori(std::move(arg), combine, this->v.cbefore_begin());
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::ors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto this_visitor = [&](iterator) {
        ++r;
    };
    auto arg_visitor = [&](typename c_arg_t::iterator) {
        ++r;
    };
    auto both_visitor = [&](iterator, typename c_arg_t::iterator) {
        ++r;
    };
    auto this_back_handler = [&](iterator pos) {
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
    const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_handler, arg_back_handler);
    return r;
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::xore(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(iterator)> this_visitor;
    std::function<void(typename c_arg_t::iterator)> arg_visitor;
    std::function<void(iterator, typename c_arg_t::iterator)> both_visitor;
    // scoping weirdness requires declaring 'it' here
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        this_visitor = [&](iterator this_pos) {
            it = r.v.insert_after(it, ret_t(*this_pos));
            ++r.maybe_size.size;
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.v.insert_after(it, ret_t(*arg_pos));
            ++r.maybe_size.size;
        };
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template xore<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                it = r.v.insert_after(it, std::move(elem));
                ++r.maybe_size.size;
            }
        };
    } else {
        this_visitor = [&](iterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            r.push_back(ret_t(*arg_pos));
        };
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template xore<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                r.push_back(std::move(elem));
            }
        };
    }
    const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    if constexpr(isVector<c_ret_t>::value) r.v.shrink_to_fit();
    return r; // nrvo 
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::xori(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVector) {
        SDR r = xore(arg);
        this->v = std::move(r.v);
    } else if constexpr(usesForwardList) {
        auto lagger = this->v.before_begin();
        auto this_visitor = [&](iterator) {
            ++lagger;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            lagger = this->v.insert_after(lagger, SDRElem_t(*arg_pos));
            ++this->maybe_size.size;
        };
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const typename SDRElem_t::data_type& data = this_pos->data().xori(arg_pos->data());
            if (data.rm_relevant()) {
                // not removed. just modified (above)
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
        auto this_visitor = [&](iterator this_pos) {
            pos = this_pos;
        };
        auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
            pos = this->v.insert(pos, SDRElem_t(*arg_pos));
        };
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const typename SDRElem_t::data_type& data = const_cast<typename SDRElem_t::data_type&>(this_pos->data()).xori(arg_pos->data());
            if (!data.rm_relevant()) {
                pos = this->v.erase(this_pos);
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::xors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto this_visitor = [&](iterator) {
        ++r;
    };
    auto arg_visitor = [&](typename c_arg_t::iterator) {
        ++r;
    };
    auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto count = this_pos->data().xors(arg_pos->data());
        if (count) ++r;
    };
    auto this_back_handler = [&](iterator pos) {
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
    const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_handler, arg_back_handler);
    return r;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::rmi(const SDR<arg_t, c_arg_t>& arg) {
    auto arg_visitor = [&](typename c_arg_t::iterator) {};
    if constexpr(usesVector) {
        iterator insert_pos = this->v.begin();
        auto this_visitor = [&](iterator this_pos) {
            *insert_pos++ = *this_pos;
        };
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const auto& data = this_pos->data().rmi(arg_pos->data());
            if (data.rm_relevant()) {
                // the element was modified above
                *insert_pos++ = SDRElem_t(this_pos->id(), std::move(data));
            } else {
                // the element is removed   
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
        this->v.resize(insert_pos - this->v.begin());
    } else if constexpr(usesForwardList) {
        // lagger is used to remove elements
        iterator lagger = this->v.before_begin();
        auto this_visitor = [&](iterator) { ++lagger; };
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const auto& data = this_pos->data().rmi(arg_pos->data());
            if (data.rm_relevant()) {
                // the element was modified above
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
        auto this_back_position = [](iterator) {};
        auto arg_back_position = [](typename c_arg_t::iterator) {};
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_position, arg_back_position);
    } else {
        auto this_visitor = [&](iterator) {};
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const auto& data = const_cast<typename SDRElem_t::data_type&>(this_pos->data()).rmi(arg_pos->data());
            if (data.rm_relevant()) {
                // the element was modified above
            } else {
                // the element is removed
                this->v.erase(this_pos);
            }
        };
        this->orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::rme(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(iterator)> this_visitor;
    std::function<void(typename c_arg_t::iterator)> arg_visitor;
    std::function<void(iterator, typename c_arg_t::iterator)> both_visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.v.before_begin();
        this_visitor = [&](iterator this_pos) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator) {};
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template rme<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ++r.maybe_size.size;
                ret_t elem(this_pos->id(), std::move(data));
                it = r.v.insert_after(it, std::move(elem));
            }
        };
    } else {
        this_visitor = [&](iterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](typename c_arg_t::iterator) {};
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            auto data = this_pos->data().template rme<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                r.push_back(std::move(elem));
            }
        };
    }
    const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor);
    if constexpr(isVector<c_ret_t>::value) {
        r.v.shrink_to_fit();
    }
    return r;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type r = 0;
    auto this_visitor = [&](iterator) {
        ++r;
    };
    auto arg_visitor = [&](typename c_arg_t::iterator) {};
    auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto count = this_pos->data().rms(arg_pos->data());
        if (count) ++r;
    };
    auto this_back_handler = [&](iterator pos) {
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
    const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(arg), this_visitor, arg_visitor, both_visitor, this_back_handler, arg_back_handler);
    return r; // nrvo
}

template<typename SDRElem_t, typename container_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::shift(int amount) {
    for (auto& elem : v) {
        #ifdef NDEBUG
        const_cast<typename SDRElem_t::id_type&>(elem.id()) += amount;
        #else
        assert(!__builtin_add_overflow(amount, elem.id(), const_cast<typename SDRElem_t::id_type*>(&elem.id())));
        #endif
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::append(SDR<arg_t, c_arg_t>&& arg) {
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

        auto pos = arg.begin();
        auto end = arg.end();
        while (pos != end) {
            it = v.insert_after(it, std::move(*pos++));
            ++this->maybe_size.size;
        }
    } else if constexpr(usesVector) {
        auto old_size = this->v.size();
        this->v.resize(this->v.size() + arg.v.size());
        auto insert_it = this->v.begin() + old_size;
        for (auto it = arg.v.begin(); it != arg.v.end(); ++it) {
            *insert_it++ = std::move(*it);
        }
    } else {
        auto pos = arg.begin();
        auto end = arg.end();
        while (pos != end) {
            push_back(std::move(*pos++));
        }
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename E>
void SDR<SDRElem_t, container_t>::push_back(E&& i) {
    SDRElem_t elem = SDRElem_t(i);
    assert(v.empty() || v.crbegin()->id() < elem.id());
    if constexpr(usesVector) {
        v.push_back(i);
    } else {
        v.insert(v.end(), i);
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename E>
typename container_t::const_iterator SDR<SDRElem_t, container_t>::insert(const_iterator position, E&& i) {
    SDRElem_t elem = SDRElem_t(i);
    assert(position == cend() || elem.id() < position->id());
    assert(position == cbegin() || elem.id() > std::prev(position)->id());
    return v.insert(position, i);
}

template<typename SDRElem_t, typename container_t>
template<typename T>
void SDR<SDRElem_t, container_t>::pop_front() {
    assert(!v.empty());
    v.pop_front();
    --maybe_size.size;
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename E>
void SDR<SDRElem_t, container_t>::push_front(E&& i)  {
    SDRElem_t elem = SDRElem_t(i);
    assert(v.empty() || v.cbegin()->id() > elem.id());
    ++maybe_size.size;
    v.push_front(i);
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename E>
typename container_t::const_iterator SDR<SDRElem_t, container_t>::insert_after(const_iterator pos, E&& i) {
    SDRElem_t elem = SDRElem_t(i);
    assert(pos == before_begin() || elem.id() > pos->id());
    auto next = std::next(pos);
    assert(next == cend() || elem.id() < next->id());
    auto ret = v.insert_after(pos, i);
    ++maybe_size.size;
    return ret;
}

template<typename SDRElem_t, typename container_t>
std::ostream& operator<<(std::ostream& os, const SDR<SDRElem_t, container_t>& sdr) {
    os << '[';
    for (auto it = sdr.cbegin(), end = sdr.cend(); it != end; ++it) { 
        const auto& i = *it;
        os << i;
        if (std::next(it) != end) os << ",";
    }
    os << ']';
    return os;
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename arg_t, typename c_arg_t>
ret_t SDR<SDRElem_t, container_t>::dot(const SDR<arg_t, c_arg_t>& other) const {
    ret_t ret;
    auto this_visitor = [&](iterator this_pos) {
        auto elem = this_pos->data().template ande<ret_t>(typename arg_t::data_type());
        ret.ori(std::move(elem));
    };

    auto arg_visitor = [&](typename c_arg_t::iterator arg_pos) {
        auto elem = typename SDRElem_t::data_type().template ande<ret_t>(arg_pos->data());
        ret.ori(std::move(elem));
    };

    auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto elem = this_pos->data().template ande<ret_t>(arg_pos->data());
        ret.ori(std::move(elem));
    };

    const_cast<SDR&>(*this).orv(const_cast<SDR<arg_t, c_arg_t>&>(other), this_visitor, arg_visitor, both_visitor);
    return ret;
}

template<typename SDRElem_t, typename container_t>
template<typename SDR<SDRElem_t, container_t>::MatrixFormat format, typename arg_t, typename ret_t, typename c_arg_t, typename c_ret_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::matrix_vector_mul(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> ret;
    if constexpr(format == ROW_MAJOR) {
        auto this_visitor = [&](iterator this_pos) {
            auto data = this_pos->data().template dot<typename ret_t::data_type>(arg);
            if (data.relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                ret.push_back(std::move(elem));
            }
        };
        const_cast<SDR&>(*this).visitor(this_visitor);
    } else {
        // points to the current iteration position
        struct column_information {
            // pos must always be valid
            typename SDRElem_t::data_type::const_iterator pos;
            typename SDRElem_t::data_type::const_iterator end;
            bool operator>(const column_information& other) const {
                return pos->id() < other.pos->id();
            }
        };

        auto get_heap = []() {
            // use the same-ish data structure as whoever made this matrix
            // (they have more context)
            if constexpr(usesVector) {
                return std::priority_queue<column_information, std::vector<column_information>, std::greater<column_information>>();
            } else {
                return std::priority_queue<column_information,  std::deque<column_information>,  std::greater<column_information>>();
            }
        };

        auto column_infos = get_heap();        

        for (const auto& col : *this) {
            // should never happen since an SDR that is empty is irrelevant, and should have been ommitted
            assert(!col.data().empty());
            column_infos.push(column_information{col.data().cbegin(), col.data().cend()});
        }

        auto arg_pos = arg.cbegin();
        auto arg_end = arg.cend();

        // helper functions for brevity, sanity, clarity

        bool accumulated_output_valid = false;
        ret_t accumulated_output; // init with garbage id, guarded by valid bool
        auto send_to_output = [&accumulated_output_valid, &accumulated_output](ret_t&& out) -> void {
            if (out.id() != accumulated_output.id()) {
                if (accumulated_output_valid) {
                    ret.push_back(std::move(accumulated_output));
                    // no pre-conditions on moved-from object
                    accumulated_output = out;

                }
            } else {
                // this is fine even if the garbage id happens to be the same as the output id
            }
        };

        // TODOODODODO

        while (!column_infos.empty()) {
            // the top of the heap points to the lowest row number
            column_information elem = column_infos.top();

            if (arg_pos != arg_end) {
                // both elems are available for combination
                // but are the id's the same?
                if (arg_pos->id() == elem.pos->id()) {
                    // the ids are the same. combine the elements
                    auto data = elem.pos->data().template ande<ret_t>(arg_pos++->data());
                    // if the output_elem is valid, send it off, before freshly 
                    
                } else if (arg_pos.id() > elem.pos->id()) {
                    goto only_this;
                } else {
                    
                }
            } else {
                only_this:
                // only this is available
                auto data = elem.pos->data().template ande<ret_t>(arg_pos++->data());
                output_elem.data().ori(data);
            }

            ++elem.pos; // increment to the next element in that row
            column_infos.pop();
            if (elem.pos != elem.end) {
                column_infos.push(elem);
            }
        }

        while (arg_pos != arg_end) {
            // only the arg is available
        }
    }
    return ret;
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::matrix_transpose() const {
    SDR<ret_t, c_ret_t> ret;

    // the way I'm doing this is wack af, however:
    // - it melds well with the existing api
    // - its efficient (maybe some unnecessary stack allocation? hopefully ommitted during compilation)

    using InsertionTempRowType = SDRElem<typename ret_t::id_type, SDR<typename ret_t::data_type::value_type, ArrayAdaptor<typename ret_t::data_type::value_type, 1>>>;

    auto row_append = [](decltype(ret.cbegin()) existing_element, InsertionTempRowType&& arg) {
        const_cast<typename ret_t::data_type&>(existing_element->data()).push_back(std::move(*arg.data().begin()));
    };

    for (const auto& this_row : *this) {
        auto insert_column = this_row.id();
        [[maybe_unused]] decltype(ret.begin()) row_insertion_hint;
        if constexpr(usesForwardList) {
            row_insertion_hint = ret.before_begin();
        } else if constexpr(usesSet) {
            // unused
        } else {
            row_insertion_hint = ret.begin();
        }
        for (const auto& this_element : this_row.data()) {
            auto insert_row = this_element.id();
            typename ret_t::data_type::value_type ret_element(insert_column, this_element.data()); // copy
            // use an array here to allocate on the stack for this temporary SDR
            SDR<typename ret_t::data_type::value_type, ArrayAdaptor<typename ret_t::data_type::value_type, 1>> ret_row;
            ret_row.push_back(std::move(ret_element));
            InsertionTempRowType val(insert_row, std::move(ret_row));
            if constexpr(usesSet) {
                ret.ori(std::move(val), row_append);
            } else {
                row_insertion_hint = ret.ori(std::move(val), row_append, row_insertion_hint);
                if constexpr(!usesForwardList) {
                    ++row_insertion_hint;
                }
            }
        }
    }
    return ret;
}

// template<typename SDRElem_t, typename container_t>
// SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::matrix_transpose_inplace() {
    
//     return *this;
// }

template<typename SDRElem_t, typename container_t>
template<typename T>
typename T::data_type::value_type::data_type SDR<SDRElem_t, container_t>::matrix_trace() const {
    typename T::data_type::value_type::data_type ret;
    for (const auto& row : *this) {
        auto row_num = row.id();
        auto elems = row.data();
        if (auto ptr = elems.ande(row_num)) {
            ret.value(ret.value() + ptr->value());
        }
    }
    return ret;
}

} // namespace sparse_distributed_representation
