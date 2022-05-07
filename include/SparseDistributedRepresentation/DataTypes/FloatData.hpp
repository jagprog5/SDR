#pragma once

namespace SparseDistributedRepresentation {

struct FloatData {
    constexpr FloatData() : value(0) {}
    constexpr FloatData(float value) : value(value) {}

    float value;

    constexpr bool relevant() {
        return true;
    }

    constexpr bool rm_relevant() {       
        return relevant();
    }

    template<typename T>
    explicit constexpr operator T() const { return T(); }

    constexpr FloatData andb(const FloatData& o) const {
        return FloatData(this->value * o.value);
    }

    constexpr FloatData orb(const FloatData& o) const {
        return FloatData(this->value + o.value);
    }

    // xor doesn't make sense in this context

    constexpr FloatData rmb(const FloatData& o) const {
        return FloatData(this->value - o.value);
    }

};

std::ostream& operator<<(std::ostream& os, const FloatData& o) {
    os << o.value;
    return os;
}

} // namespace