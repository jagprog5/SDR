#pragma once

namespace sparse_distributed_representation {

// this is the data_type for an SDRElem which stores an element from 0 to 1.
class UnitData {
    public:
        constexpr UnitData() : value_(1) {}
        constexpr UnitData(float value) : value_(value) {
            assert(value_ >= 0 && value_ <= 1);
        }

        constexpr float value() const { return value_; }
        constexpr void value(float value) { value_ = value; }

        constexpr bool relevant() const {
            return value() >= 0.1;
        }

        constexpr bool rm_relevant() const {
            return relevant();
        }

        // for compatibility with other data types
        template<typename T>
        explicit constexpr operator T() const { return T(value); }
    
        UnitData ande(const UnitData& o) const {
            return UnitData(value() * o.value());
        }

        constexpr UnitData ore(const UnitData& o) const {
            return UnitData(value() > o.value() ? value() : o.value());
        }

        constexpr UnitData xore(const UnitData& o) const {
            return UnitData(std::abs(value() - o.value()));
        }

        constexpr UnitData rme(const UnitData& o) const {
            return UnitData(value() * (1 - o.value()));
        }

        template<typename T>
        constexpr bool operator==(const T& o) const {
            return value() == (UnitData(o)).value();
        }

    private:
        float value_;
};

inline std::ostream& operator<<(std::ostream& os, const UnitData& o) {
    auto val = o.value();
    if (val > 1 || val < 0) {
        os << "!!!";
    } else {
        if (val == 1) {
            os << "1.0";
        } else {
            // in range [0,1)
            os << '.';
            val *= 10;
            os << (int)val;
            // NOLINTNEXTLINE
            val -= (int)val;
            val *= 10;
            os << (int)val;
        }
    }
    return os;
}

} // namespace