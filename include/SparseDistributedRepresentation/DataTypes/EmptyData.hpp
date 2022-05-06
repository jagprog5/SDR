#pragma once

namespace SparseDistributedRepresentation {

struct EmptyData {
    // this is the data_type for a default SDR_t which does not use the data functionality

    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wzero-length-array")
            #pragma GCC diagnostic ignored "-Wzero-length-array"
        #endif
    #endif
    // if this member wasn't here, the struct size would be 1, instead of 0
    char unused[0];
    #pragma GCC diagnostic pop

    constexpr bool relevant() {
        // if any element is combined with this one, then it is in the result
        return true;
    }

    constexpr bool rm_relevant() {       
        // if any element is removed from this one, then it is not in the result 
        return false;
    }

    template<typename T>
    explicit constexpr operator T() const { return T(); }

    template<typename arg_t>
    constexpr EmptyData andb([[maybe_unused]] const arg_t& o) const {
        return EmptyData();
    }

    template<typename arg_t>
    constexpr EmptyData orb([[maybe_unused]] const arg_t& o) const {
        return EmptyData();
    }

    template<typename arg_t>
    constexpr EmptyData xorb([[maybe_unused]] const arg_t& o) const {
        return EmptyData();
    }

    template<typename arg_t>
    constexpr EmptyData rmb([[maybe_unused]] const arg_t& o) const {
        return EmptyData();
    }
};

} // namespace