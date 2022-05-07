#pragma once

namespace SparseDistributedRepresentation {

struct UnitData {
    constexpr UnitData() : value(1) {}
    constexpr UnitData(float value) : value(value) {
        assert(value >= 0 && value <= 1);
    }

    float value;

    constexpr bool relevant() {
        return value > 0.1;
    }

    constexpr bool rm_relevant() {       
        return relevant();
    }

    template<typename T>
    explicit constexpr operator T() const { return T(); }

    constexpr UnitData andb(const UnitData& o) const {
        return UnitData(this->value * o.value);
    }

    constexpr UnitData orb(const UnitData& o) const {
        return UnitData(this->value > o.value ? this->value : o.value);
    }

    constexpr UnitData xorb(const UnitData& o) const {
        return UnitData(std::abs(this->value - o.value));
    }

    constexpr UnitData rmb(const UnitData& o) const {
        return UnitData(this->value * (1 - o.value));
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