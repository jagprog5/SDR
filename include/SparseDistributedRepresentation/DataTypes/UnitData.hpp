#pragma once

#include "SparseDistributedRepresentation/DataTypes/UnitData.hpp"

namespace SparseDistributedRepresentation {

struct UnitData {
    constexpr UnitData() : value(1) {}
    constexpr UnitData(float value) : value(value) {}
    float value;

    template <typename T, typename = void>
    struct has_value : std::false_type {};

    template <typename T>
    struct has_value<T, decltype((void)T::value, void())> : std::true_type {};

    constexpr bool relevant() {
        return value > 0.1;
    }

    constexpr bool rm_relevant() {       
        return relevant();
    }

    template<typename T>
    explicit constexpr operator T() const { return T(); }

    template<typename arg_t>
    constexpr UnitData andb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return UnitData(this->value * o.value);
        } else {
            return UnitData(this->value);
        }
    }

    template<typename arg_t>
    constexpr UnitData orb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return UnitData(this->value > o.value ? this->value : o.value);
        } else {
            return UnitData(1);
        }
    }

    template<typename arg_t>
    constexpr UnitData xorb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return UnitData(std::abs(this->value - o.value));
        } else {
            return UnitData(0);
        }
    }

    template<typename arg_t>
    constexpr UnitData rmb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return UnitData(this->value * (1 - o.value));
        } else {
            return UnitData(0);
        }
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