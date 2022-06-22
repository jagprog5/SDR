#pragma once

namespace sparse_distributed_representation {

class EmptyData {
    public:
        // this is the data_type for a default SDR_elem_t which does not use the data functionality

        constexpr EmptyData() : unused() {}

        // for compatibility with other data types
        template<typename T>
        constexpr EmptyData(T) : unused() {}

        constexpr bool relevant() const {
            // if any element is combined with this one, then it is in the result
            return true;
        }

        constexpr bool rm_relevant() const {
            // if any element is removed from this one, then it is not in the result
            return false;
        }

        // for compatibility with other data types
        template<typename T>
        explicit constexpr operator T() const { return T(); }

        constexpr EmptyData ande(const EmptyData&) const {
            return EmptyData();
        }

        constexpr EmptyData ore(const EmptyData&) const {
            return EmptyData();
        }

        constexpr EmptyData xore(const EmptyData&) const {
            return EmptyData();
        }

        constexpr EmptyData rme(const EmptyData&) const {
            return EmptyData();
        }

        template<typename T>
        constexpr bool operator==(const T&) const {
            return true;
        }

    private:
        #pragma GCC diagnostic push
        #if defined(__has_warning)
            #if __has_warning("-Wzero-length-array")
                #pragma GCC diagnostic ignored "-Wzero-length-array"
            #endif
        #endif
        // if this member wasn't here, the struct size would be 1, instead of 0.
        // just cpp being quirky I guess
        // NOLINTNEXTLINE(clang-diagnostic-unused-private-field)
        char unused[0];
        #pragma GCC diagnostic pop
};

inline std::ostream& operator<<(std::ostream& os, const EmptyData&) {
    os << "EMPTY";
    return os;
}

} // namespace