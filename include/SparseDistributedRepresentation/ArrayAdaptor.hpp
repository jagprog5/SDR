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
        size_t size_ = 0; // number of elements filled

    public:
        using value_type = SDR_t;
        using size_type = std::size_t;
        // using difference_type = std::ptrdiff_t;
        // using reference = SDR_t&;
        // using const_reference = const SDR_t&;
        // using pointer = SDR_t*;
        // using const_pointer = const SDR_t*;
        using iterator = typename std::array<SDR_t, N>::iterator;
        using const_iterator = typename std::array<SDR_t, N>::const_iterator;
        using reverse_iterator = typename std::array<SDR_t, N>::reverse_iterator;
        using const_reverse_iterator = typename std::array<SDR_t, N>::const_reverse_iterator;

        size_t size() { return size_; }
        bool empty() { return size_ == 0; }
        
        iterator begin() { return &arr_[0]; }
        iterator end() { return &arr_[size_]; }
        
        const_iterator cbegin() const { return &arr_[0]; }
        const_iterator cend() const { return &arr_[size_]; }

        reverse_iterator rbegin() { return std::reverse_iterator(end()); }
        reverse_iterator rend() { return std::reverse_iterator(begin()); }

        const_reverse_iterator crbegin() const { return std::reverse_iterator(cend()); }

        const_reverse_iterator crend() const { return std::reverse_iterator(cbegin()); }

        void push_back(const value_type& val) {
            #ifndef NDEBUG
            assert(size_ < N);
            #endif
            arr_[size_++] = val;
        }
};