#pragma once

#include <assert.h>
#include <initializer_list>
#include <cmath>
#include <algorithm>
#include <functional>

#include "SparseDistributedRepresentation/Templates.hpp"
#include "SparseDistributedRepresentation/SDRElem.hpp" 
#include "SparseDistributedRepresentation/MatrixUtils.hpp"

namespace sparse_distributed_representation {

enum MatrixFormat { ROW_MAJOR, COLUMN_MAJOR };
enum MatrixFormatSameness { SAME_MAJOR, DIFFERENT_MAJOR };

/**
 * An SDR or sparse nd array.
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
        using pointer = typename container_t::pointer;
        using const_pointer = typename container_t::const_pointer;
        using reference = typename container_t::reference;
        using const_reference = typename container_t::const_reference;

        static constexpr bool usesVectorLike = vectorLike<container_t>::value;
        static constexpr bool usesFlistLike = flistLike<container_t>::value;
        static constexpr bool usesSetLike = setLike<container_t>::value;

        static_assert(!std::is_fundamental<SDRElem_t>::value, "Instead of SDR<fundamental_type_here>, use SDR<SDRElem_t<fundamental_type_here>>");
        static_assert(!usesSetLike || setComparatorCheck<container_t>::value, "Bad comparator for container! instead of std::less<T>, use std::less<>");

        // default ctor
        SDR() {
            if constexpr(usesFlistLike)
                this->maybe_size.size = 0;
        }

        // copy ctor
        SDR(const SDR& sdr): v(sdr.v) {
            if constexpr(usesFlistLike)
                this->maybe_size.size = sdr.maybe_size.size;
        }

        // copy assignment ctor
        SDR& operator=(const SDR& sdr) {
            this->v = sdr.v;
            if constexpr(usesFlistLike)
                this->maybe_size.size = sdr.maybe_size.size;
            return *this;
        }

        // move ctor
        SDR(SDR&& sdr) noexcept : v(std::move(sdr.v)) {
            if constexpr(usesFlistLike)
                this->maybe_size.size = sdr.maybe_size.size;
        }

        // move assignment ctor
        SDR& operator=(SDR&& sdr) noexcept {
            this->v = std::move(sdr.v);
            if constexpr(usesFlistLike)
                this->maybe_size.size = sdr.maybe_size.size;
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
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> ore(SDR<arg_t, c_arg_t>&& arg) const;

        /**
         * or inplace. Insert elements into this, or combine elements if they are already in this.
         * 
         * @return Ref to this.
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& ori(const SDR<arg_t, c_arg_t>& arg);
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& ori(SDR<arg_t, c_arg_t>&& arg);

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
        template<typename ret_t = SDRElem_t, typename c_ret_t = container_t, typename arg_t, typename c_arg_t>
        SDR<ret_t, c_ret_t> xore(SDR<arg_t, c_arg_t>&& arg) const;

        /**
         * xor inplace. computes this xor arg, and places the result in this.
         * 
         * @return Ref to this.
         */
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& xori(const SDR<arg_t, c_arg_t>& arg);
        template<typename arg_t, typename c_arg_t>
        SDR<SDRElem_t, container_t>& xori(SDR<arg_t, c_arg_t>&& arg);

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
         * the visitors must not invalidate proceeding iterators (one after this_position or one after arg_position)
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
         * at this point, this_only_position(iterator) is called with the curent position in this, and then this function exits.
         * similarly, if there are no more elements in this, then arg_only_position(c_arg_t::iterator) is called, followed by an exit.
         * 
         * the visitors must not invalidate proceeding iterators (one after this_position or one after arg_position)
         */
        template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisOnlyPosition, typename ArgOnlyPosition>
        void orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisOnlyPosition this_only_position, ArgOnlyPosition arg_only_position);

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
        auto begin() const { return v.cbegin(); } // const is intentional
        auto end() const { return v.cend(); }
        auto empty() const { return v.empty(); }

        auto size() const {
            if constexpr(usesFlistLike) {
                return maybe_size.size;
            } else {
                return v.size();
            }
        }

        void clear() noexcept { v.clear(); }

        template<typename T = container_t>
        void shrink_to_fit() { v.shrink_to_fit(); }

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
        const_iterator erase(const_iterator position) { return v.erase(position); };

        // calls erase on the underlying container
        template<typename T = container_t>
        const_iterator erase(const_iterator first, const_iterator last) { return v.erase(first, last); };

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
        const_iterator cbefore_begin() const { return v.cbefore_begin(); }

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
        const_iterator lower_bound(const value_type& val) const { return v.lower_bound(val); }

        // calls upper_bound on the set underlying container
        template<typename T = container_t>
        const_iterator upper_bound(const value_type& val) const { return v.upper_bound(val); }

        template<typename SDRElem_t_inner, typename container_t_inner>
        friend std::ostream& operator<<(std::ostream& os, const SDR<SDRElem_t_inner, container_t_inner>& sdr);

        template<typename other>
        auto operator&(other&& o) { return ande(std::forward<other>(o)); } // non-const version for non-const ande(id_type)
        template<typename other>
        auto operator&(other&& o) const { return ande(std::forward<other>(o)); }
        template<typename other>
        auto operator&&(other&& o) const { return ands(std::forward<other>(o)); }
        template<typename other>
        auto operator&=(other&& o) { return andi(std::forward<other>(o)); }
        template<typename other>
        auto operator*(other&& o) const { return ande(std::forward<other>(o)); }
        template<typename other>
        auto operator*=(other&& o) { return andi(std::forward<other>(o)); }
        template<typename other>
        auto operator|(other&& o) const { return ore(std::forward<other>(o)); }
        template<typename other>
        auto operator||(other&& o) const { return ors(std::forward<other>(o)); }
        template<typename other>
        auto operator|=(other&& o) { return ori(std::forward<other>(o)); }
        template<typename other>
        auto operator^(other&& o) const { return xore(std::forward<other>(o)); }
        template<typename other>
        auto operator^=(other&& o) { return xori(std::forward<other>(o)); }
        template<typename other>
        auto operator+(other&& o) const { return ore(std::forward<other>(o)); }
        template<typename other>
        auto operator+=(other&& o) { return ori(std::forward<other>(o)); }
        template<typename other>
        auto operator-(other&& o) const { return rme(std::forward<other>(o)); }
        template<typename other>
        auto operator-=(other&& o) { return rmi(std::forward<other>(o)); }
        template<typename other>
        auto operator<<(other&& o) const { return SDR(*this).shift(std::forward<other>(o)); }
        template<typename other>
        auto operator>>(other&& o) const { return SDR(*this).shift(std::forward<other>(-o)); }
        template<typename other>
        auto operator<<=(other&& o) { return shift(std::forward<other>(o)); }
        template<typename other>
        auto operator>>=(other&& o) { return shift(std::forward<other>(-o)); }
        
        template<typename arg_t, typename c_arg_t>
        auto operator==(const SDR<arg_t, c_arg_t>& other) const {
            auto this_pos = this->cbegin();
            auto this_end = this->cend();
            auto other_pos = other.cbegin();
            auto other_end = other.cend();
            while (true) {
                if (this_pos == this_end) return other_pos == other_end;
                if (other_pos == other_end) return false;
                const auto& this_elem = *this_pos++;
                const auto& other_elem = *other_pos++;
                if (this_elem.id() != other_elem.id() || this_elem.data() != other_elem.data()) return false;
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

        template<MatrixFormat format = ROW_MAJOR,
                 typename arg_t,
                 typename ret_t = arg_t,
                 typename c_arg_t,
                 typename c_ret_t = c_arg_t,
                 typename PriorityQueueContainer_t = std::vector<other_major_view_objs::row_information<SDR<SDRElem_t, container_t>>>>
        SDR<ret_t, c_ret_t> matrix_vector_mul(const SDR<arg_t, c_arg_t>& arg) const;

        template<typename ret_t = SDRElem_t,
                 typename c_ret_t = container_t,
                 typename PriorityQueueContainer_t = std::vector<other_major_view_objs::row_information<SDR<SDRElem_t, container_t>>>>
        SDR<ret_t, c_ret_t> matrix_transpose() const;

        template<typename T = SDRElem_t>
        typename T::data_type::value_type::data_type matrix_trace() const;

        /**
         * Multiply two matrices together.
         * 
         * The arg must have the same / different format as this matrix, as specified by targ format.
         * 
         * The returned matrix has the same column / row major-ness as this matrix.
         */
        template<MatrixFormatSameness format = SAME_MAJOR,
                 typename arg_t,
                 typename ret_t = arg_t,
                 typename c_arg_t,
                 typename c_ret_t = c_arg_t,
                 typename PriorityQueueContainer_t = std::vector<other_major_view_objs::row_information<SDR<arg_t, c_arg_t>>>>
        SDR<ret_t, c_ret_t> matrix_matrix_mul(const SDR<arg_t, c_arg_t>& arg) const;

        // relevance is needed for interface compatability between SDRs and SDRElem::data_type
        bool relevant() const { return !empty(); }
        bool rm_relevant() const { return relevant(); }
        
        // for testing / debug purposes. ensure that the elements are in ascending order and with no duplicates.
        bool is_ascending() const;
    private:
        container_t v;


        MaybeSize<container_t> maybe_size;

        // used in the output stream op
        static constexpr bool print_type = false;

        template<typename friend_SDRElem_t, typename friend_container_t>
        friend class SDR;

        template<typename friend_SDRElem_t, typename friend_container_t, typename QueryIterator>
        friend SDR<friend_SDRElem_t, friend_container_t>& ori(SDR<friend_SDRElem_t, friend_container_t>& me, QueryIterator arg_begin, QueryIterator arg_end);

        template<typename friend_SDRElem_t, typename friend_container_t, typename QueryIterator>
        friend SDR<friend_SDRElem_t, friend_container_t>& xori(SDR<friend_SDRElem_t, friend_container_t>& me, QueryIterator arg_begin, QueryIterator arg_end);
};

template<typename SDRElem_t, typename container_t>
bool SDR<SDRElem_t, container_t>::is_ascending() const {
    if constexpr(!usesSetLike) {
        #ifndef NDEBUG
            if (v.empty()) return true;
            auto pos = v.cbegin();
            auto end = v.cend();
            typename SDRElem_t::id_type prev_id = pos++->id();
            while (pos != end) {
                typename SDRElem_t::id_type id = pos++->id();
                if (prev_id >= id) {
                    return false;
                }
                prev_id = id;
            }   
        #endif
        return true;
    } else {
        return true;
    }
}

template<typename SDRElem_t, typename container_t>
template<typename Iterator>
SDR<SDRElem_t, container_t>::SDR(Iterator begin, Iterator end) {
    if constexpr(usesFlistLike) {
        this->maybe_size.size = 0;
        auto insert_it = this->v.before_begin();
        while (begin != end) {
            insert_it = this->v.insert_after(insert_it, SDRElem_t(*begin++));
            ++this->maybe_size.size;
        }
    } else if constexpr(usesVectorLike && std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>::value) {
        auto count = end - begin;
        this->v.resize(count);
        for (decltype(count) i = 0; i < count; ++i) {
            this->v[i] = SDRElem_t(*begin++); 
        }
    } else {
        while (begin != end) {
            if constexpr(usesVectorLike) {
                v.push_back(SDRElem_t(*begin++));
            } else {
                v.insert(v.end(), SDRElem_t(*begin++));
            }
        }
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T>
SDR<SDRElem_t, container_t>::SDR(std::initializer_list<T> list) {
    if constexpr(usesFlistLike) {
        auto pos = list.begin();
        auto end = list.end();
        this->maybe_size.size = 0;
        auto insert_it = this->v.before_begin();
        while (pos != end) {
            const T& elem = *pos++;
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
                // allow for the id only. e.g. SDR{1, 2, 3}. Implicitly assumes EmptyData
                relevant = true;
            } else {
                relevant = elem.data().relevant();
            }
            if (relevant) {
                if constexpr(usesVectorLike) {
                    this->v.push_back(SDRElem_t(elem));
                } else {
                    this->v.insert(this->v.end(), SDRElem_t(elem));
                }
            }
        }
    }

    assert(is_ascending() && "Elements must be in ascending order and with no duplicates.");
}

template<typename SDRElem_t, typename container_t>
SDR<SDRElem_t, container_t>::SDR(float input, float period, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length && period >= 0 && input >= 0);
    if constexpr(usesVectorLike) v.resize(size);

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

        if constexpr(usesFlistLike) {
            auto insert_it = v.before_begin();
            for (size_type i = 0; i < wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDRElem_t(i));
            }
            for (size_type i = 0; i < non_wrapped_elements; ++i) {
                insert_it = v.insert_after(insert_it, SDRElem_t(start_index + i));
            }
        } else if constexpr(usesVectorLike) {
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
        if constexpr(usesFlistLike) {
            auto insert_it = v.before_begin();
            for (size_type i = 0; i < size; ++i) {
                insert_it = v.insert_after(insert_it, SDRElem_t(start_index + i));
            }
        } else if constexpr(usesVectorLike) {
            for (size_type i = 0; i < size; ++i) {
                v[i] = SDRElem_t(start_index + i);
            }
        } else {
            for (size_type i = 0; i < size; ++i) {
                v.insert(v.end(), SDRElem_t(start_index + i));
            }
        }
    }
    if constexpr(usesFlistLike) {
        maybe_size.size = size;
    }
}

template<typename SDRElem_t, typename container_t>
SDR<SDRElem_t, container_t>::SDR(float input, size_type size, size_type underlying_array_length) {
    assert(size <= underlying_array_length);
    assert(input >= 0);
    if constexpr(usesVectorLike) v.resize(size);
    size_type start_index = std::round((underlying_array_length - size) * input);
    if constexpr(usesFlistLike) {
        auto insert_it = v.before_begin();
        for (size_type i = 0; i < size; ++i) {
            insert_it = v.insert_after(insert_it, SDRElem_t(start_index + i));
        }
    } else if constexpr(usesVectorLike) {
        for (size_type i = 0; i < size; ++i) {
            v[i] = SDRElem_t(start_index + i);
        }
    } else {
        for (size_type i = 0; i < size; ++i) {
            v.insert(v.end(), SDRElem_t(start_index + i));
        }
    }
    if constexpr(usesFlistLike) {
        maybe_size.size = size;
    }
}

template<typename SDRElem_t, typename container_t>
template<typename RandomGenerator>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::sample(float amount, RandomGenerator& g) {
    assert(amount >= 0 && amount <= 1);
    auto check_val = amount * (float)g.max();
    if constexpr(usesFlistLike) {
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
    } else if constexpr(usesVectorLike) {
        auto to_offset = v.begin();
        auto from_offset = v.cbegin();
        auto end = v.end();
        while (from_offset != end) {
            if (g() < check_val) {
                *to_offset++ = std::move(*from_offset);
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
    if constexpr(usesSetLike) {
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
    if constexpr(usesSetLike) {
        start_it = v.lower_bound(start_inclusive);
    } else {
        start_it = std::lower_bound(v.cbegin(), v.cend(), start_inclusive);
    }
    typename container_t::const_iterator end_it;
    if constexpr(usesFlistLike) {
        end_it = start_it;
        while (true) {
            if (end_it == cend() || end_it->id() >= stop_exclusive) {
                break;
            }
            ++end_it;
            ++sdr.maybe_size.size;
        }
    } else if constexpr(usesSetLike) {
        end_it = v.lower_bound(stop_exclusive);
    } else {
        end_it = std::lower_bound(start_it, v.cend(), stop_exclusive);
    }
    // at this point the start and end positions have been found
    // place the range into the result
    if constexpr(vectorLike<c_ret_t>::value) {
        sdr.v.resize(end_it - start_it);
    }
    if constexpr(flistLike<c_ret_t>::value) {
        auto insert_it = sdr.v.before_begin();
        for (auto it = start_it; it != end_it; ++it) {
            insert_it = sdr.v.insert_after(insert_it, ret_t(*it));
        }
    } else if constexpr(vectorLike<c_ret_t>::value) {
        auto insert_it = sdr.v.begin();
        for (auto it = start_it; it != end_it; ++it) {
            *insert_it++ = ret_t(*it);
        }
    }  else {
        for (auto it = start_it; it != end_it; ++it) {
            sdr.v.insert(sdr.v.end(), ret_t(*it));
        }
    } 
    return sdr; // nrvo
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t>
// NOLINTNEXTLINE
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::ands(arg_t start_inclusive, arg_t stop_exclusive) const {
    if constexpr(usesSetLike) {
        auto pos = this->v.lower_bound(start_inclusive);
        size_type count = 0;
        while (pos != v.cend()) {
            if (pos++->id() >= stop_exclusive) {
                break;
            }
            ++count;
        }
        return count;
    } else if constexpr(usesVectorLike) {
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
    typename SDRElem_t::id_type this_elem;
    typename arg_t::id_type arg_elem;

    if (this_pos == this_end) return;
    while (true) {
        // get an element in this
        this_elem = this_pos->id();
        // try to find the matching element in the arg
        if constexpr(setLike<c_arg_t>::value) {
            arg_pos = arg.v.lower_bound(this_elem);
        } else {
            arg_pos = std::lower_bound(arg_pos, arg_end, this_elem);
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
        arg_elem = arg_pos->id();
        if constexpr(usesSetLike) {
            this_pos = this->v.lower_bound(arg_elem);
        } else {
            this_pos = std::lower_bound(this_pos, this_end, arg_elem);
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

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::ande(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    std::function<void(iterator, typename c_arg_t::iterator)> visitor;
    [[maybe_unused]] typename c_ret_t::iterator it;
    if constexpr(flistLike<c_ret_t>::value) {
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
    if constexpr(usesVectorLike) {
        auto pos = this->v.begin();
        auto visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            typename SDRElem_t::data_type& data = this_pos->data().andi(arg_pos->data());
            if (data.relevant()) {
                SDRElem_t elem(this_pos->id(), std::move(data));
                *pos++ = std::move(elem);
            }
        };
        andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
        this->v.resize(pos - this->v.begin());
    } else if constexpr(usesFlistLike) {
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

// iterator form. detached from class itself
// (the reason there are separate (iterator vs object) overloads is to avoid code duplication between const lval and rval overloads of or operator functions)
template<typename ThisIterator, typename QueryIterator, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisOnlyPosition, typename ArgOnlyPosition>
void orv(ThisIterator this_pos, ThisIterator this_end, QueryIterator arg_pos, QueryIterator arg_end, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisOnlyPosition this_only_position, ArgOnlyPosition arg_only_position) {
    typename std::remove_cv<typename std::remove_reference<decltype(this_pos->id())>::type>::type this_val;
    typename std::remove_cv<typename std::remove_reference<decltype(arg_pos->id())>::type>::type arg_val;

    // helper lambdas
    // returns false if the function should exit
    auto get_this = [&]() {
        if (this_pos != this_end) {
            this_val = this_pos->id();
        } else {
            arg_only_position(arg_pos);
            return false;
        }
        return true;
    };

    auto get_arg = [&]() {
        if (arg_pos != arg_end) {
            arg_val = arg_pos->id();
        } else {
            this_only_position(this_pos);
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
        if (this_val < arg_val) {
            this_visitor(this_pos++);
            if (!get_this()) { return; }
        } else if (this_val > arg_val) {
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

// iterator form. a less specific overload
template<typename ThisIterator, typename QueryIterator, typename ThisVisitor, typename QueryVisitor, typename BothVisitor>
void orv(ThisIterator this_pos, ThisIterator this_end, QueryIterator arg_pos, QueryIterator arg_end, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor) {
    auto this_only_position = [&](ThisIterator pos) {
        while (pos != this_end) {
            this_visitor(pos++);
        }
    };
    auto arg_only_position = [&](QueryIterator pos) {
        while (pos != arg_end) {
            arg_visitor(pos++);
        }
    };
    sparse_distributed_representation::orv(this_pos, this_end, arg_pos, arg_end, this_visitor, arg_visitor, both_visitor, this_only_position, arg_only_position);
}

// wraps the iterator form with the object form
template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor>
void SDR<SDRElem_t, container_t>::orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor) {
    sparse_distributed_representation::orv(this->v.begin(), this->v.end(), arg.v.begin(), arg.v.end(), this_visitor, arg_visitor, both_visitor);
}

// wraps the iterator form with the object form
template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t, typename ThisVisitor, typename QueryVisitor, typename BothVisitor, typename ThisOnlyPosition, typename ArgOnlyPosition>
void SDR<SDRElem_t, container_t>::orv(SDR<arg_t, c_arg_t>& arg, ThisVisitor this_visitor, QueryVisitor arg_visitor, BothVisitor both_visitor, ThisOnlyPosition this_only_position, ArgOnlyPosition arg_only_position) {
    sparse_distributed_representation::orv(this->v.begin(), this->v.end(), arg.v.begin(), arg.v.end(), this_visitor, arg_visitor, both_visitor, this_only_position, arg_only_position);
}

// iterator form. exposed to ore for rvalue and lvalue overloads
template<typename ret_t, typename c_ret_t, typename MeIterator, typename QueryIterator>
SDR<ret_t, c_ret_t> ore(MeIterator this_begin, MeIterator this_end, QueryIterator arg_begin, QueryIterator arg_end) {
    SDR<ret_t, c_ret_t> r;
    std::function<void(MeIterator)> this_visitor;
    std::function<void(QueryIterator)> arg_visitor;
    std::function<void(MeIterator, QueryIterator)> both_visitor;
    if constexpr(flistLike<c_ret_t>::value) {
        auto it = r.before_begin();
        this_visitor = [&](MeIterator this_pos) {
            it = r.insert_after(it, ret_t(*this_pos));
        };
        arg_visitor = [&](QueryIterator arg_pos) {
            // it is assumed that creating ret_t data from arg_t data does not change the relevance
            // (meaning that there is no need to check for relevance here).
            // rational: elements which are already relevant (since it already exists in the arg SDR)
            // should not become irrelevant after changing to a different type.
            it = r.insert_after(it, ret_t(*arg_pos));
        };
        both_visitor = [&](MeIterator this_pos, QueryIterator arg_pos) {
            auto data = this_pos->data().template ore<typename ret_t::data_type>(arg_pos->data());
            // there is no relevance check here, since it is assumed that elements which already exist in an SDR are relevant,
            // and that ore can only produce relevant elements from relevant elements
            ret_t elem(this_pos->id(), std::move(data));
            it = r.insert_after(it, std::move(elem));
        };
        orv(this_begin, this_end, arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    } else {
        this_visitor = [&](MeIterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](QueryIterator arg_pos) {
            r.push_back(ret_t(*arg_pos));
        };
        both_visitor = [&](MeIterator this_pos, QueryIterator arg_pos) {
            auto data = this_pos->data().template ore<typename ret_t::data_type>(arg_pos->data());
            ret_t elem(this_pos->id(), std::move(data));
            r.push_back(std::move(elem));
        };
        orv(this_begin, this_end, arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    }
    if constexpr(vectorLike<c_ret_t>::value) r.shrink_to_fit();
    return r; // nrvo
}

// exposing const lval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::ore(const SDR<arg_t, c_arg_t>& arg) const {
    return sparse_distributed_representation::ore<ret_t, c_ret_t>(this->cbegin(), this->cend(), arg.cbegin(), arg.cend());
}

// exposing rval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::ore(SDR<arg_t, c_arg_t>&& arg) const {
    return sparse_distributed_representation::ore<ret_t, c_ret_t>(this->cbegin(), this->cend(), std::make_move_iterator(arg.v.begin()), std::make_move_iterator(arg.v.end()));
}

// iterator form, exposed to ori for const lval and rval overloads
template<typename SDRElem_t, typename container_t, typename QueryIterator>
SDR<SDRElem_t, container_t>& ori(SDR<SDRElem_t, container_t>& me, QueryIterator arg_begin, QueryIterator arg_end) {
    using const_iterator = typename SDR<SDRElem_t, container_t>::const_iterator;
    if constexpr(vectorLike<container_t>::value) {
        auto r = ore<SDRElem_t, container_t>(std::make_move_iterator(me.v.begin()), std::make_move_iterator(me.v.end()), arg_begin, arg_end);
        me = std::move(r);
    } else if constexpr(flistLike<container_t>::value) {
        auto lagger = me.before_begin();
        auto this_visitor = [&](const_iterator) {
            ++lagger;
        };
        auto arg_visitor = [&](QueryIterator arg_pos) {
            me.insert_after(lagger, SDRElem_t(*arg_pos));
            ++lagger;
        };
        auto both_visitor = [&](const_iterator this_pos, QueryIterator arg_pos) {
            const_cast<typename SDRElem_t::data_type&>(this_pos->data()).ori(arg_pos->data());
            ++lagger;
        };
        orv(me.begin(), me.end(), arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    } else {
        auto pos = me.begin();
        auto this_visitor = [&](const_iterator this_pos) {
            pos = this_pos;
        };
        auto arg_visitor = [&](QueryIterator arg_pos) {
            pos = me.insert(pos, SDRElem_t(*arg_pos));
        };
        auto both_visitor = [&](const_iterator this_pos, QueryIterator arg_pos) {
            const_cast<typename SDRElem_t::data_type&>(this_pos->data()).ori(arg_pos->data());
        };
        orv(me.begin(), me.end(), arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    }
    return me;
}

// exposing const lval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::ori(const SDR<arg_t, c_arg_t>& arg) {
    return sparse_distributed_representation::ori(*this, arg.cbegin(), arg.cend());
}

// exposing rval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::ori(SDR<arg_t, c_arg_t>&& arg) {
    return sparse_distributed_representation::ori(*this, std::make_move_iterator(arg.v.begin()), std::make_move_iterator(arg.v.end()));
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::ors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type overlap = 0;
    auto visitor = [&](iterator, typename c_arg_t::iterator) {
        // same relevance assumption mentioned in ore
        ++overlap;
    };
    const_cast<SDR&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return this->size() + arg.size() - overlap;
}

// iterator form, exposed to xore for const lval and rval overloads
template<typename ret_t, typename c_ret_t, typename MeIterator, typename QueryIterator>
SDR<ret_t, c_ret_t> xore(MeIterator this_begin, MeIterator this_end, QueryIterator arg_begin, QueryIterator arg_end) {
    SDR<ret_t, c_ret_t> r;
    std::function<void(MeIterator)> this_visitor;
    std::function<void(QueryIterator)> arg_visitor;
    std::function<void(MeIterator, QueryIterator)> both_visitor;
    // scoping weirdness requires declaring 'it' here
    // if it was instead declared inside the flistLike branch below (in the same line it is initialized),
    // then it would be no longer valid by the time the lambdas are used in orv
    // it's strange that c++ does not catch it going out of scope...
    [[maybe_unused]] typename c_ret_t::const_iterator it;
    if constexpr(flistLike<c_ret_t>::value) {
        it = r.before_begin();
        this_visitor = [&](MeIterator this_pos) {
            it = r.insert_after(it, ret_t(*this_pos));
        };
        arg_visitor = [&](QueryIterator arg_pos) {
            // it is assumed that casting arg_t data to ret_t data does not change the relevance
            it = r.insert_after(it, ret_t(*arg_pos));
        };
        both_visitor = [&](MeIterator this_pos, QueryIterator arg_pos) {
            auto data = this_pos->data().template xore<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                it = r.insert_after(it, std::move(elem));
            }
        };
    } else {
        this_visitor = [&](MeIterator this_pos) {
            r.push_back(ret_t(*this_pos));
        };
        arg_visitor = [&](QueryIterator arg_pos) {
            r.push_back(ret_t(*arg_pos));
        };
        both_visitor = [&](MeIterator this_pos, QueryIterator arg_pos) {
            auto data = this_pos->data().template xore<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                r.push_back(std::move(elem));
            }
        };
    }
    orv(this_begin, this_end, arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    if constexpr(vectorLike<c_ret_t>::value) r.shrink_to_fit();
    return r; // nrvo
}

// exposing const lval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::xore(const SDR<arg_t, c_arg_t>& arg) const {
    return sparse_distributed_representation::xore<ret_t, c_ret_t>(this->cbegin(), this->cend(), arg.cbegin(), arg.cend());
}

// exposing rval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::xore(SDR<arg_t, c_arg_t>&& arg) const {
    return sparse_distributed_representation::xore<ret_t, c_ret_t>(this->cbegin(), this->cend(), std::make_move_iterator(arg.v.begin()), std::make_move_iterator(arg.v.end()));
}

// iterator form, exposed to ori for const lval and rval overloads
template<typename SDRElem_t, typename container_t, typename QueryIterator>
SDR<SDRElem_t, container_t>& xori(SDR<SDRElem_t, container_t>& me, QueryIterator arg_begin, QueryIterator arg_end) {
    using const_iterator = typename SDR<SDRElem_t, container_t>::const_iterator;
    if constexpr(vectorLike<container_t>::value) {
        auto r = xore<SDRElem_t, container_t>(std::make_move_iterator(me.v.begin()), std::make_move_iterator(me.v.end()), arg_begin, arg_end);
        me = std::move(r);
    } else if constexpr(flistLike<container_t>::value) {
        auto lagger = me.before_begin();
        auto this_visitor = [&](const_iterator) {
            ++lagger;
        };
        auto arg_visitor = [&](QueryIterator arg_pos) {
            lagger = me.insert_after(lagger, SDRElem_t(*arg_pos));
        };
        auto both_visitor = [&](const_iterator this_pos, QueryIterator arg_pos) {
            const typename SDRElem_t::data_type& data = const_cast<typename SDRElem_t::data_type&>(this_pos->data()).xori(arg_pos->data());
            if (data.rm_relevant()) {
                // not removed. just modified (above)
                ++lagger;
            } else {
                // removed
                me.erase_after(lagger);
            }
        };
        orv(me.begin(), me.end(), arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    } else {
        auto pos = me.begin();
        auto this_visitor = [&](const_iterator this_pos) {
            pos = this_pos;
        };
        auto arg_visitor = [&](QueryIterator arg_pos) {
            pos = me.insert(pos, SDRElem_t(*arg_pos));
        };
        auto both_visitor = [&](const_iterator this_pos, QueryIterator arg_pos) {
            const typename SDRElem_t::data_type& data = const_cast<typename SDRElem_t::data_type&>(this_pos->data()).xori(arg_pos->data());
            if (!data.rm_relevant()) {
                pos = me.erase(this_pos);
            }
        };
        orv(me.begin(), me.end(), arg_begin, arg_end, this_visitor, arg_visitor, both_visitor);
    }
    return me;
}

// exposing const lval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::xori(const SDR<arg_t, c_arg_t>& arg) {
    return sparse_distributed_representation::xori(*this, arg.cbegin(), arg.cend());
}

// exposing rval overload from iterator forms
template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::xori(SDR<arg_t, c_arg_t>&& arg) {
    return sparse_distributed_representation::xori(*this, std::make_move_iterator(arg.v.begin()), std::make_move_iterator(arg.v.end()));
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::xors(const SDR<arg_t, c_arg_t>& arg) const {
    size_type remove = 0;
    auto visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        if (!this_pos->data().xors(arg_pos->data())) ++remove;
        ++remove;
    };
    const_cast<SDR&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return this->size() + arg.size() - remove;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::rmi(const SDR<arg_t, c_arg_t>& arg) {
    if constexpr(usesVectorLike) {
        // this_fill emulates an orv over this but an andv over the arg
        const_iterator this_fill = this->cbegin();
        iterator insert_pos = this->v.begin();
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            while (this_fill != this_pos) {
                *insert_pos++ = std::move(*this_fill++);
            }
            auto& data = this_pos->data().rmi(arg_pos->data());
            if (data.rm_relevant()) {
                // the element was modified above
                *insert_pos++ = SDRElem_t(this_pos->id(), std::move(data));
            } else {
                // the element is removed
            }
            ++this_fill;
        };
        this->andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), both_visitor);
        while (this_fill != this->cend()) {
            *insert_pos++ = std::move(*this_fill++);
        }
        this->v.resize(insert_pos - this->v.begin());
    } else if constexpr(usesFlistLike) {
        // lagger is used to remove elements
        iterator lagger = this->v.before_begin();
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            // get the lagger up to speed
            while (1) {
                iterator next = std::next(lagger);
                if (next == this_pos) break;
                lagger = next;
            }

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
        this->andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), both_visitor);
    } else {
        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            const auto& data = const_cast<typename SDRElem_t::data_type&>(this_pos->data()).rmi(arg_pos->data());
            if (data.rm_relevant()) {
                // the element was modified above
            } else {
                // the element is removed
                this->v.erase(this_pos);
            }
        };
        this->andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), both_visitor);
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename arg_t, typename c_arg_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::rme(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> r;
    // this_fill emulates an orv over this but an andv over the arg
    const_iterator this_fill = this->cbegin();
    std::function<void(iterator, typename c_arg_t::iterator)> both_visitor;
    [[maybe_unused]] typename c_ret_t::const_iterator it; // iterator for appending to flist result

    if constexpr(flistLike<c_ret_t>::value) {
        it = r.v.before_begin();

        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            while (this_fill != this_pos) {
                ++r.maybe_size.size;
                it = r.v.insert_after(it, ret_t(*this_fill++));
            }
            auto data = this_pos->data().template rme<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ++r.maybe_size.size;
                ret_t elem(this_pos->id(), std::move(data));
                it = r.v.insert_after(it, std::move(elem));
            }
            ++this_fill;
        };
    } else {
        both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
            while (this_fill != this_pos) {
                r.push_back(ret_t(*this_fill++));
            }
            auto data = this_pos->data().template rme<typename ret_t::data_type>(arg_pos->data());
            if (data.rm_relevant()) {
                ret_t elem(this_pos->id(), std::move(data));
                r.push_back(std::move(elem));
            }
            ++this_fill;
        };
    }
    const_cast<SDR&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), both_visitor);
    while (this_fill != this->v.cend()) {
        if constexpr(flistLike<c_ret_t>::value) {
            ++r.maybe_size.size;
            it = r.v.insert_after(it, ret_t(*this_fill++));
        } else {
            r.push_back(ret_t(*this_fill++));
        }
    }

    if constexpr(vectorLike<c_ret_t>::value) {
        r.v.shrink_to_fit();
    }
    return r;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
typename SDR<SDRElem_t, container_t>::size_type SDR<SDRElem_t, container_t>::rms(const SDR<arg_t, c_arg_t>& arg) const {
    size_type remove = 0;
    auto visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        if (!this_pos->data().rms(arg_pos->data())) ++remove;
    };
    const_cast<SDR&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), visitor);
    return this->size() - remove;
}

template<typename SDRElem_t, typename container_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::shift(int amount) {
    for (auto& elem : v) {
        #ifdef NDEBUG
        const_cast<typename SDRElem_t::id_type*>(&elem.id()) += amount;
        #else
        // NOLINTNEXTLINE
        assert(!__builtin_add_overflow(amount, elem.id(), const_cast<typename SDRElem_t::id_type*>(&elem.id())));
        #endif
    }
    return *this;
}

template<typename SDRElem_t, typename container_t>
template<typename arg_t, typename c_arg_t>
SDR<SDRElem_t, container_t>& SDR<SDRElem_t, container_t>::append(SDR<arg_t, c_arg_t>&& arg) {
    // this function doesn't work for flist sdrs. no good way of providing an efficient specialization
    assert(empty() || arg.empty() || *--v.end() < *arg.v.cbegin());
    if constexpr(usesVectorLike) {
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
    SDRElem_t elem = SDRElem_t(std::forward<E>(i));
    if (!v.empty()) {
        auto last_elem_iter = v.end();
        --last_elem_iter;
        assert(last_elem_iter->id() < elem.id());
    }
    if constexpr(usesVectorLike) {
        v.push_back(std::move(elem));
    } else {
        v.insert(v.end(), std::move(elem));
    }
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename E>
typename container_t::const_iterator SDR<SDRElem_t, container_t>::insert(const_iterator position, E&& i) {
    SDRElem_t elem = SDRElem_t(std::forward<E>(i));
    if constexpr(!usesSetLike) {
        assert(position == cend() || elem.id() < position->id());
        assert(position == cbegin() || elem.id() > std::prev(position)->id());
    }
    return v.insert(position, std::move(elem));
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
    SDRElem_t elem = SDRElem_t(std::forward<E>(i));
    assert(v.empty() || v.cbegin()->id() > elem.id());
    ++maybe_size.size;
    v.push_front(std::move(elem));
}

template<typename SDRElem_t, typename container_t>
template<typename T, typename E>
typename container_t::const_iterator SDR<SDRElem_t, container_t>::insert_after(const_iterator pos, E&& i) {
    SDRElem_t elem = SDRElem_t(std::forward<E>(i));
    assert(pos == before_begin() || elem.id() > pos->id());
    auto next = std::next(pos);
    assert(next == cend() || elem.id() < next->id());
    auto ret = v.insert_after(pos, std::move(elem));
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
    auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator arg_pos) {
        auto elem = this_pos->data().template ande<ret_t>(arg_pos->data());
        ret.ori(std::move(elem));
    };
    const_cast<SDR&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(other), both_visitor);
    return ret;
}

template<typename SDRElem_t, typename container_t>
template<MatrixFormat format, typename arg_t, typename ret_t, typename c_arg_t, typename c_ret_t, typename PriorityQueueContainer_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::matrix_vector_mul(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> ret;
    if constexpr(format == ROW_MAJOR) {
        std::function<void(iterator)> visitor;
        [[maybe_unused]] typename c_ret_t::iterator it;

        if constexpr(flistLike<c_ret_t>::value) {
            it = ret.v.before_begin();
            visitor = [&](iterator this_pos) {
                auto data = this_pos->data().template dot<typename ret_t::data_type>(arg);
                if (data.relevant()) {
                    ++ret.maybe_size.size;
                    ret_t elem(this_pos->id(), std::move(data));
                    it = ret.v.insert_after(it, std::move(elem));
                }
            };
        } else {
            visitor = [&](iterator this_pos) {
                auto data = this_pos->data().template dot<typename ret_t::data_type>(arg);
                if (data.relevant()) {
                    ret_t elem(this_pos->id(), std::move(data));
                    ret.push_back(std::move(elem));
                }
            };
        }

        const_cast<SDR&>(*this).visitor(visitor);
    } else {
        OtherMajorView<SDR<SDRElem_t, container_t>, PriorityQueueContainer_t> view;

        auto both_visitor = [&](iterator this_pos, typename c_arg_t::iterator) {
            view.add_major(*this_pos);
        };

        const_cast<SDR<SDRElem_t, container_t>&>(*this).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), both_visitor);

        BucketOutputAccumulator accumulator(ret);
        while (view) {
            auto pos = *view;

            decltype(arg.cbegin()) arg_pos;
            if constexpr(setLike<c_arg_t>::value) {
                arg_pos = arg.v.lower_bound(pos.major_id);
            } else {
                arg_pos = std::lower_bound(arg.cbegin(), arg.cend(), pos.major_id);
            }

            // arg_pos is guarenteed to contain the id, because of the andv call when initializing the view
            assert(arg_pos != arg.cend() && arg_pos->id() == pos.major_id);
            typename ret_t::id_type id = pos.element->id();
            auto data = pos.element->data().template ande<typename ret_t::data_type>(arg_pos->data());
            ret_t output_elem(id, std::move(data));
            accumulator(std::move(output_elem));
            ++view;
        }
    }
    return ret;
}

template<typename SDRElem_t, typename container_t>
template<typename ret_t, typename c_ret_t, typename PriorityQueueContainer_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::matrix_transpose() const {
    SDR<ret_t, c_ret_t> ret;
    OtherMajorView<SDR<SDRElem_t, container_t>, PriorityQueueContainer_t> view;
    for (const auto& elem : *this) {
        view.add_major(elem);
    }

    BucketOutputAppender appender(ret);
    while (view) {
        auto pos = *view;
        appender.send(pos.element->id(), std::move(typename ret_t::data_type::value_type(pos.major_id, pos.element->data())));
        ++view;
    }

    return ret;
}

template<typename SDRElem_t, typename container_t>
template<MatrixFormatSameness format, typename arg_t, typename ret_t, typename c_arg_t, typename c_ret_t, typename PriorityQueueContainer_t>
SDR<ret_t, c_ret_t> SDR<SDRElem_t, container_t>::matrix_matrix_mul(const SDR<arg_t, c_arg_t>& arg) const {
    SDR<ret_t, c_ret_t> ret;
    if constexpr(format == SAME_MAJOR) {
        // for variable naming, assume that both this and the arg are row major

        [[maybe_unused]] typename c_ret_t::const_iterator output_row_insertion;
        if constexpr(flistLike<c_ret_t>::value) {
            output_row_insertion = ret.before_begin();
        } 

        for (const auto& row : *this) {
            typename SDRElem_t::data_type::const_iterator this_row_retrival = row.data().cbegin();

            OtherMajorView<SDR<arg_t, c_arg_t>, PriorityQueueContainer_t> view;
            auto both_visitor = [&](typename SDRElem_t::data_type::iterator, typename c_arg_t::iterator arg_pos) {
                view.add_major(*arg_pos);
            };
            const_cast<typename SDRElem_t::data_type&>(row.data()).andv(const_cast<SDR<arg_t, c_arg_t>&>(arg), both_visitor);

            typename ret_t::data_type output_data;

            { // scope for accumulator dtor
                BucketOutputAccumulator accumulator(output_data);

                while (view) {
                    auto pos = *view;

                    auto column_id = pos.element->id();
                    if (this_row_retrival == row.data().cend() || this_row_retrival->id() > pos.major_id) {
                        this_row_retrival = row.data().cbegin();
                    }
                    if constexpr(setLike<typename SDRElem_t::data_type::container_type>::value) {
                        this_row_retrival = row.data().v.lower_bound(pos.major_id);
                    } else {
                        this_row_retrival = std::lower_bound(this_row_retrival, row.data().cend(), pos.major_id);
                    }

                    assert(this_row_retrival != row.data().cend() && this_row_retrival->id() == pos.major_id);
                    auto data = this_row_retrival->data().template ande<typename ret_t::data_type::value_type::data_type>(pos.element->data());
                    typename ret_t::data_type::value_type output_elem(column_id, std::move(data));
                    accumulator(std::move(output_elem));
                    ++view;
                }
            } // scope

            if (output_data.relevant()) {
                ret_t output(row.id(), std::move(output_data));
                if constexpr(flistLike<c_ret_t>::value) {
                    output_row_insertion = ret.v.insert_after(output_row_insertion, std::move(output));
                } else {
                    ret.push_back(std::move(output));
                }
            }
        }
    } else {
        [[maybe_unused]] typename c_ret_t::iterator it;
        if constexpr(flistLike<c_ret_t>::value) {
            it = ret.v.before_begin();
        }
        for (const auto& row : *this) {
            typename ret_t::data_type row_data;
            [[maybe_unused]] typename ret_t::data_type::const_iterator inner_it;
            if constexpr(flistLike<typename ret_t::data_type::container_type>::value) {
                inner_it = row_data.before_begin();
            }
            for (const auto& column : arg) {
                auto data = row.data().template dot<typename ret_t::data_type::value_type::data_type>(column.data());
                typename ret_t::data_type::value_type elem(column.id(), std::move(data));
                if constexpr(flistLike<typename ret_t::data_type::container_type>::value) {
                    ++row_data.maybe_size.size;
                    inner_it = row_data.v.insert_after(inner_it, std::move(elem));
                } else {
                    row_data.push_back(std::move(elem));
                }
            }
            ret_t output_row(row.id(), std::move(row_data));
            if constexpr(flistLike<c_ret_t>::value) {
                ++ret.maybe_size.size;
                it = ret.v.insert_after(it, std::move(output_row));
            } else {
                ret.push_back(std::move(output_row));
            }
        }
    }
    return ret;
}

template<typename SDRElem_t, typename container_t>
template<typename T>
typename T::data_type::value_type::data_type SDR<SDRElem_t, container_t>::matrix_trace() const {
    typename T::data_type::value_type::data_type ret;
    for (const auto& row : *this) {
        auto row_num = row.id();
        auto elems = row.data();
        if (const auto* ptr = elems.ande(row_num)) {
            ret.value(ret.value() + ptr->value());
        }
    }
    return ret;
}

} // namespace sparse_distributed_representation
