#pragma once

#include "SparseDistributedRepresentation/Templates.hpp"
#include <iostream>

namespace sparse_distributed_representation {

class EmptyData;

// this is the data_type for an SDRElem which stores an element from 0 to 1.
class UnitData {
    public:
        constexpr UnitData(EmptyData) : UnitData() {}
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

        template<typename ret_t = UnitData, typename T>
        constexpr ret_t ande(const T& o) const {
            UnitData r(*this);
            r.andi(o); // reusing andi
            return ret_t(r);
        }
        
        template<typename T>
        constexpr UnitData& andi(const T& o) {
            // EmptyData lacks a value() function
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ *= o.value();
            }
            return *this;
        }

        template<typename T>
        constexpr bool ands(const T& o) const {
            return ande(o).relevant();
        }

        template<typename ret_t = UnitData, typename T>
        constexpr ret_t ore(const T& o) const {
            UnitData r(*this);
            r.ori(o); // reusing ori
            return ret_t(r);
        }

        template<typename T>
        constexpr UnitData& ori(const T& o) {
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ = value_ > o.value() ? value_ : o.value();
            }
            return *this;
        }

        template<typename T>
        constexpr bool ors(const T& o) const {
            return ore(o).relevant();
        }

        template<typename ret_t = UnitData, typename T>
        constexpr ret_t xore(const T& o) const {
            UnitData r(*this);
            r.xori(o); // reusing xori
            return ret_t(r);
        }

        template<typename T>
        constexpr UnitData& xori(const T& o) {
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ = std::abs(value_ - o.value());
            }
            return *this;
        }

        template<typename T>
        constexpr bool xors(const T& o) const {
            return xore(o).rm_relevant();
        }

        template<typename ret_t = UnitData, typename T>
        constexpr ret_t rme(const T& o) const {
            UnitData r(*this);
            r.rmi(o); // reusing rmi
            return ret_t(r);
        }

        template<typename T>
        constexpr UnitData& rmi(const T& o) {
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ = value_ * (1 - o.value());
            }
            return *this;
        }

        template<typename T>
        constexpr bool rms(const T& o) const {
            return rme(o).rm_relevant();
        }

        constexpr bool operator==(const UnitData& o) const {
            return value() == o.value();
        }

        constexpr bool operator!=(const UnitData& o) const {
            return value() != o.value();
        }

        constexpr bool operator==(const EmptyData&) const {
            return true;
        }

        constexpr bool operator!=(const EmptyData&) const {
            return false;
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