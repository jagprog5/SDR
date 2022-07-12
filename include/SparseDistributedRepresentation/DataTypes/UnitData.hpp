#pragma once

#include "SparseDistributedRepresentation/Templates.hpp"

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

        template<typename T>
        constexpr UnitData ande(const T& o) const {
            UnitData r(*this);
            r.andi(o); // reusing andi
            return r;
        }
        
        template<typename T>
        constexpr UnitData& andi(const T& o) {
            // EmptyData lacks a value() function
            if constexpr(hasValue<T>::value) {
                value_ *= o.value();
            }
            return *this;
        }

        template<typename T>
        constexpr UnitData ore(const T& o) const {
            UnitData r(*this);
            r.ori(o); // reusing ori
            return r;
        }

        template<typename T>
        constexpr UnitData& ori(const T& o) {
            if constexpr(hasValue<T>::value) {
                value_ = value_ > o.value() ? value_ : o.value();
            }
            return *this;
        }

        template<typename T>
        constexpr UnitData xore(const T& o) const {
            UnitData r(*this);
            r.xori(o); // reusing xori
            return r;
        }

        template<typename T>
        constexpr UnitData& xori(const T& o) {
            if constexpr(hasValue<T>::value) {
                value_ = std::abs(value_ - o.value());
            }
            return *this;
        }

        template<typename T>
        constexpr UnitData rme(const T& o) const {
            UnitData r(*this);
            r.rmi(o); // reusing rmi
            return r;
        }

        template<typename T>
        constexpr UnitData& rmi(const T& o) {
            if constexpr(hasValue<T>::value) {
                value_ = value_ * (1 - o.value());
            }
            return *this;
        }

        constexpr bool operator==(const UnitData& o) const {
            return value() == o.value();
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