#pragma once
#include <array>

/**
 * An ArrayAdaptor provides a vector-like interface so an array can be used in an SDR.
 * This is needed for statically allocated SDRs.
 * 
 * Ensure that the size if sufficient. e.g. or elements can produce an output with a size only up to (inclusively) the sum of the arguments' sizes
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

    public:
        using value_type = SDRElem_t;
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

        // NOLINTNEXTLINE(bugprone-unhandled-self-assignment) self assignment is no-op
        ArrayAdaptor& operator=(const ArrayAdaptor& o) noexcept {
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

        ArrayAdaptor& operator=(ArrayAdaptor&& o) noexcept {
            auto o_pos = std::make_move_iterator(o.begin());
            auto o_end = std::make_move_iterator(o.end());
            auto this_pos = begin();
            while (o_pos != o_end) {
                *this_pos++ = *o_pos++;
            }
            end_ = this_pos;
            return *this;
        }

        ArrayAdaptor(ArrayAdaptor&& o) noexcept {
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

        template<typename E>
        void push_back(E&& val) {
            assert(size() < N);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *end_++ = val;
        }

        void erase(iterator pos) {
            assert(!empty());
            while (++pos != end_) {
                *(pos - 1) = std::move(*pos);
            }
            --end_;
            drop_content(end_);
        }

        void resize(size_t size) {
            assert(size <= this->size() && "resize can only shrink an ArrayAdaptor");
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
