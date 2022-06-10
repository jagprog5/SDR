#pragma once

namespace sparse_distributed_representation {

struct UnitData {
    // this is the data_type for an SDR_t which stores an element from 0 to 1.

    constexpr UnitData() : value(1) {}
    constexpr UnitData(float value) : value(value) {
        assert(value >= 0 && value <= 1);
    }

    float value;

    constexpr bool relevant() const {
        return value >= 0.1;
    }

    constexpr bool rm_relevant() const {       
        return relevant();
    }

    // for compatibility with other data types
    template<typename T>
    explicit constexpr operator T() const { return T(value); }

    constexpr UnitData ande(const UnitData& o) const {
        return UnitData(this->value * o.value);
    }

    constexpr UnitData ore(const UnitData& o) const {
        return UnitData(this->value > o.value ? this->value : o.value);
    }

    constexpr UnitData xore(const UnitData& o) const {
        return UnitData(std::abs(this->value - o.value));
    }

    constexpr UnitData rme(const UnitData& o) const {
        return UnitData(this->value * (1 - o.value));
    }

    template<typename T>
    constexpr bool operator==(const T& o) const {
        return value == (UnitData(o)).value;
    }

};

std::ostream& operator<<(std::ostream& os, const UnitData& o) {
    auto val = o.value;
    if (val > 1 || val < 0) {
        os << "!!!";
    } else {
        os << '.';
        val *= 10;
        os << (int)val;
        val -= (int)val;
        val *= 10;
        os << (int)val;
    }
    return os;
}

} // namespace