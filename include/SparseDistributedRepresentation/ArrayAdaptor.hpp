#pragma once
#include <array>

/**
 * An ArrayAdaptor provides a vector-like interface so an array can be used in an SDR.
 * This is needed for statically allocated SDRs.
 */
template<typename SDR_t, std::size_t N>
class ArrayAdaptor {
    private:
        std::array<SDR_t, N> arr_;
        SDR_t* end_;

    public:
        using value_type = SDR_t;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = SDR_t&;
        using const_reference = const SDR_t&;
        using pointer = SDR_t*;
        using const_pointer = const SDR_t*;
        using iterator = typename std::array<SDR_t, N>::iterator;
        using const_iterator = typename std::array<SDR_t, N>::const_iterator;
        using reverse_iterator = typename std::array<SDR_t, N>::reverse_iterator;
        using const_reverse_iterator = typename std::array<SDR_t, N>::const_reverse_iterator;

        ArrayAdaptor() : arr_(), end_(&arr_[0]) {}

        ArrayAdaptor& operator=(const ArrayAdaptor& o) {
            auto o_pos = o.cbegin();
            auto o_end = o.cend();
            auto this_pos = begin();
            while (o_pos != o_end) {
                *this_pos++ = *o_pos++;
            }
            end_ = this_pos;
            return *this;
        }

        ArrayAdaptor(const ArrayAdaptor& o) {
            *this = o;
        }

        ArrayAdaptor& operator=(ArrayAdaptor&& o) {
            auto o_pos = std::make_move_iterator(o.begin());
            auto o_end = std::make_move_iterator(o.end());
            auto this_pos = begin();
            while (o_pos != o_end) {
                *this_pos++ = *o_pos++;
            }
            end_ = this_pos;
            return *this;
        }

        ArrayAdaptor(ArrayAdaptor&& o) {
            *this = o;
        }

        size_t size() const { return end_ - &arr_[0]; }
        bool empty() const { return end_ == &arr_[0]; }
        
        iterator begin() { return &arr_[0]; }
        iterator end() { return end_; }
        
        const_iterator cbegin() const { return &arr_[0]; }
        const_iterator cend() const { return end_; }

        reverse_iterator rbegin() { return std::reverse_iterator(end()); }
        reverse_iterator rend() { return std::reverse_iterator(begin()); }

        const_reverse_iterator crbegin() const { return std::reverse_iterator(cend()); }

        const_reverse_iterator crend() const { return std::reverse_iterator(cbegin()); }

        void push_back(const value_type& val) {
            #ifndef NDEBUG
                assert((size_t)(end_ - &arr_[0]) < N);
            #endif
            *end_++ = val;
        }

        void erase(iterator pos) {
            #ifndef NDEBUG
                assert(!empty());
            #endif
            while (++pos != end_) {
                *(pos - 1) = *pos;
            }
            --end_;
        }

        void resize(size_t size) {
            #ifndef NDEBUG
                assert(size <= N);
            #endif
            end_ = &arr_[0] + size;
        }

        void shrink_to_fit() {}
};