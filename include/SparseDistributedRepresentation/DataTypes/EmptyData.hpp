#pragma once

namespace sparse_distributed_representation {

class EmptyData {
    // this is the data_type for a default SDRElem
    // it disables the data functionality

    public:
        constexpr EmptyData() : unused() {}

        constexpr bool relevant() const {
            // if any element is combined with this one, then it is in the result
            return true;
        }

        constexpr bool rm_relevant() const {
            // if any element is removed from this one, then it is not in the result
            return false;
        }

        template<typename T>
        constexpr EmptyData ande(const T&) const { return EmptyData(); }

        template<typename T>
        constexpr EmptyData& andi(const T&) { return *this; }

        template<typename T>
        constexpr EmptyData ore(const T&) const { return EmptyData(); }

        template<typename T>
        constexpr EmptyData& ori(const T&) { return *this; }

        template<typename T>
        constexpr EmptyData xore(const T&) const { return EmptyData(); }

        template<typename T>
        constexpr EmptyData& xori(const T&) { return *this; }

        template<typename T>
        constexpr EmptyData rme(const T&) const { return EmptyData(); }

        template<typename T>
        constexpr EmptyData& rmi(const T&) { return *this; }

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
            #if __has_warning("-Wunused-private-field")
                #pragma GCC diagnostic ignored "-Wunused-private-field"
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