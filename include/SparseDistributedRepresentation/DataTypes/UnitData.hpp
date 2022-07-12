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
    
        constexpr UnitData ande(const UnitData& o) const {
            UnitData r(*this);
            r.andi(o); // reusing andi
            return r;
        }
        
        constexpr UnitData& andi(const UnitData& o) {
            value_ *= o.value();
            return *this;
        }

        constexpr UnitData ore(const UnitData& o) const {
            UnitData r(*this);
            r.ori(o); // reusing ori
            return r;
        }

        constexpr UnitData& ori(const UnitData& o) {
            value_ = value_ > o.value() ? value_ : o.value();
            return *this;
        }

        constexpr UnitData xore(const UnitData& o) const {
            UnitData r(*this);
            r.xori(o); // reusing xori
            return r;
        }

        constexpr UnitData& xori(const UnitData& o) {
            value_ = std::abs(value_ - o.value());
            return *this;
        }

        constexpr UnitData rme(const UnitData& o) const {
            UnitData r(*this);
            r.rmi(o); // reusing rmi
            return r;
        }

        constexpr UnitData& rmi(const UnitData& o) {
            value_ = value_ * (1 - o.value());
            return *this;
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