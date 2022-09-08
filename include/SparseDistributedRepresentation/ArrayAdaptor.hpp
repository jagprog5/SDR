#pragma once
#include <array>

namespace sparse_distributed_representation {

/**
 * An ArrayAdaptor provides a vector-like interface so an array can be used in an SDR.
 * This is needed for statically allocated SDRs.
 * 
 * It's like a boost::container::static_vector
 * 
 * Ensure that the capacity is always sufficient.
 * e.g. or elements can produce an output with a size only up to (inclusively) the sum of the arguments' sizes
 * 
 * Note that only the neccessary functions are implemented here (for use in SDR).
 */
template<typename SDRElem_t, std::size_t N>
class ArrayAdaptor {
    private:
        std::array<SDRElem_t, N> arr_;
        SDRElem_t* end_;

    void drop_content(SDRElem_t* pos) {
        // moves from the element, then drops the contents
        [[maybe_unused]] SDRElem_t r = std::move(*pos);
    }

    template<typename Iterator>
    void replace(Iterator begin, Iterator end) {
        SDRElem_t* previous_end = end_;
        construct(begin, end); // sets end_
        if (end_ < previous_end) {
            do {
                drop_content(--previous_end);
            } while (previous_end != end_);
        }
    }

    // self assignment / move is ok. However, it is the responsability of SDRElem_t to check for self assignment / move at the element level.
    template<typename Iterator>
    void construct(Iterator begin, Iterator end) {
        auto this_pos = &arr_[0];
        while (begin != end) {
            *this_pos++ = *begin++;
        }
        end_ = this_pos;
    }

    public:
        using value_type = SDRElem_t;
        static constexpr std::size_t capacity = N;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = SDRElem_t&;
        using const_reference = const SDRElem_t&;
        using pointer = SDRElem_t*;
        using const_pointer = const SDRElem_t*;
        using iterator = typename std::array<SDRElem_t, N>::iterator;
        using const_iterator = typename std::array<SDRElem_t, N>::const_iterator;
        using reverse_iterator = typename std::array<SDRElem_t, N>::reverse_iterator;
        using const_reverse_iterator = typename std::array<SDRElem_t, N>::const_reverse_iterator;

        ArrayAdaptor() : arr_(), end_(&arr_[0]) {}

        template<typename Iterator>
        ArrayAdaptor(Iterator begin, Iterator end) {
            construct(begin, end);
        }

        // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
        ArrayAdaptor& operator=(const ArrayAdaptor& o) {
            replace(o.cbegin(), o.cend());
            return *this;
        }

        ArrayAdaptor(const ArrayAdaptor& o) {
            construct(o.cbegin(), o.cend());
        }

        ArrayAdaptor& operator=(ArrayAdaptor&& o) noexcept {
            replace(std::make_move_iterator(o.begin()), std::make_move_iterator(o.end()));
            return *this;
        }

        ArrayAdaptor(ArrayAdaptor&& o) noexcept {
            construct(std::make_move_iterator(o.begin()), std::make_move_iterator(o.end()));
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

        reference operator[](size_type pos) { return arr_[pos]; }
        const_reference operator[](size_type pos) const { return arr_[pos]; }

        const_reference front() const {
            assert(!empty());
            return arr_[0];
        }

        reference front() {
            assert(!empty());
            return arr_[0];
        }

        template<typename E>
        void push_back(E&& val) {
            assert(size() < N);
            *end_++ = std::forward<E>(val);
        }

        void pop_back() {
            assert(!empty());
            drop_content(--end_);
        }

        iterator erase(const_iterator arg) {
            assert(!empty());
            iterator pos = &const_cast<SDRElem_t&>(*arg);
            auto ret = pos;
            while (++pos != end_) {
                *(pos - 1) = std::move(*pos);
            }
            --end_;
            drop_content(end_);
            return ret;
        }

        void resize(size_t size) {
            assert(size <= this->size() && "resize can only shrink an ArrayAdaptor");
            SDRElem_t* new_end = &arr_[0] + size;
            SDRElem_t* pos = new_end;
            while (pos != end()) {
                drop_content(pos++);
            }
            end_ = new_end;
        }

        void clear() {
            SDRElem_t* pos = begin();
            while (pos != end()) {
                drop_content(pos++);
            }
            end_ = begin();
        }

        void shrink_to_fit() {}
};

} // namespace