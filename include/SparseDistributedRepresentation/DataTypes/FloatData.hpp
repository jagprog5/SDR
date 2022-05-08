#pragma once

namespace SparseDistributedRepresentation {

struct FloatData {
    constexpr FloatData() : value(0) {}
    constexpr FloatData(float value) : value(value) {}

    float value;

    constexpr bool relevant() const {
        return true;
    }

    constexpr bool rm_relevant() const {       
        return relevant();
    }

    // for compatability with other data types
    template<typename T>
    explicit constexpr operator T() const { return T(); }

    constexpr FloatData ande(const FloatData& o) const {
        return FloatData(this->value * o.value);
    }

    constexpr FloatData ore(const FloatData& o) const {
        return FloatData(this->value + o.value);
    }

    // xor doesn't make sense in this context

    constexpr FloatData rme(const FloatData& o) const {
        return FloatData(this->value - o.value);
    }

};

std::ostream& operator<<(std::ostream& os, const FloatData& o) {
    os << o.value;
    return os;
}

} // namespace