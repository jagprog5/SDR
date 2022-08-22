#pragma once

namespace sparse_distributed_representation {

#pragma GCC diagnostic push
#if defined(__has_warning)
    #if __has_warning("-Wzero-length-array")
        #pragma GCC diagnostic ignored "-Wzero-length-array"
    #endif
    #if __has_warning("-Wunused-private-field")
        #pragma GCC diagnostic ignored "-Wunused-private-field"
    #endif
    #if __has_warning("-Warray-bounds")
        #pragma GCC diagnostic ignored "-Warray-bounds"
    #endif
#endif

class EmptyData {
    // this is the data_type for a default SDRElem
    // it disables the data functionality

    public:
        constexpr EmptyData() : unused() {}

        template<typename T>
        // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
        constexpr explicit EmptyData(T&&) : unused() {}

        constexpr bool relevant() const {
            // if any element is combined with this one, then it is in the result
            return true;
        }

        constexpr bool rm_relevant() const {
            // if any element is removed from this one, then it is not in the result
            return false;
        }

        // the SDElem_t::data_type classes are made to have a similar interface to SDR.
        // (note that both EmptyData and SDR have ande, xors, etc.)
        // that way an SDR can be used as a SDElem_t::data_type, for the creation of n dimensional arrays
        // (SDR containing SDRs containing SDRs... ad infinitum)

        template<typename ret_t = EmptyData, typename T>
        constexpr EmptyData ande(const T&) const { return ret_t(); }

        template<typename T>
        constexpr EmptyData& andi(const T&) { return *this; }

        template<typename T>
        constexpr bool ands(const T&) const { return relevant(); }

        template<typename ret_t = EmptyData, typename T>
        constexpr EmptyData ore(const T&) const { return ret_t(); }

        template<typename T>
        constexpr EmptyData& ori(const T&) { return *this; }

        // ors isn't actually used, keeping it anyway for consistency
        // (SDR::ors doesn't check for data relevance because it doesn't need to)
        template<typename T>
        constexpr bool ors(const T&) const { return relevant(); }

        template<typename ret_t = EmptyData, typename T>
        constexpr EmptyData xore(const T&) const { return ret_t(); }

        template<typename T>
        constexpr EmptyData& xori(const T&) { return *this; }

        template<typename T>
        constexpr bool xors(const T&) const { return rm_relevant(); }

        template<typename ret_t = EmptyData, typename T>
        constexpr EmptyData rme(const T&) const { return EmptyData(); }

        template<typename T>
        constexpr EmptyData& rmi(const T&) { return *this; }

        template<typename T>
        constexpr bool rms(const T&) const { return rm_relevant(); }

        constexpr bool operator==(const EmptyData&) const { return true; }
        constexpr bool operator!=(const EmptyData&) const { return false; }

    private:
        // if this member wasn't here, the EmptyData size would be 1, instead of 0.
        // just cpp being quirky I guess
        // NOLINTNEXTLINE(clang-diagnostic-unused-private-field)
        char unused[0];
};

#pragma GCC diagnostic pop

inline std::ostream& operator<<(std::ostream& os, const EmptyData&) {
    os << "EMPTY";
    return os;
}

} // namespace
