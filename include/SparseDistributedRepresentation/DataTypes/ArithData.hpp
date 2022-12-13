#pragma once

#include "SparseDistributedRepresentation/Templates.hpp"
#include <functional>
#include <iostream>

namespace sparse_distributed_representation {

class EmptyData;

template<typename arith_t = float>
class ArithData {
    public:
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        explicit ArithData(EmptyData) : ArithData() {}
        constexpr ArithData() : value_(0) {}
        constexpr ArithData(arith_t value) : value_(value) {}

        arith_t value() const { return value_; }
        void value(arith_t value) { value_ = value; }

        constexpr bool relevant() const {
            return true;
        }

        constexpr bool rm_relevant() const {       
            return relevant();
        }

        template<typename ret_t = ArithData, typename T>
        ret_t ande(const T& o) const {
            ArithData r(*this);
            r.andi(o); // reusing andi
            return ret_t(r);
        }
        
        template<typename T>
        ArithData& andi(const T& o) {
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ *= o.value();
            }
            return *this;
        }

        template<typename T>
        bool ands(const T& o) const {
            return ande(o).relevant();
        }

        template<typename ret_t = ArithData, typename T>
        ret_t ore(const T& o) const {
            ArithData r(*this);
            r.ori(o); // reusing ori
            return ret_t(r);
        }
        
        template<typename T>
        ArithData& ori(const T& o) {
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ += o.value();
            }
            return *this;
        }

        template<typename T>
        bool ors(const T& o) const {
            return ore(o).relevant();
        }

        template<typename ret_t = ArithData, typename T>
        ret_t rme(const T& o) const {
            ArithData r(*this);
            r.rmi(o); // reusing rmi
            return ret_t(r);
        }
        
        template<typename T>
        ArithData& rmi(const T& o) {
            if constexpr(!std::is_base_of_v<EmptyData, T>) {
                value_ -= o.value();
            }
            return *this;
        }

        template<typename T>
        bool rms(const T& o) const {
            return rme(o).relevant();
        }

        bool operator==(const ArithData& o) const {
            return value() == o.value();
        }

        bool operator!=(const ArithData& o) const {
            return value() != o.value();
        }

        bool operator==(const EmptyData&) const {
            return true;
        }

        bool operator!=(const EmptyData&) const {
            return false;
        }

    private:
        arith_t value_;
};

template<typename T, typename = void>
struct has_xor : std::false_type {};

template<typename T>
struct has_xor<T, decltype((void)T().xore(T()), void())> : std::true_type {};

template<typename arith_t>
inline std::ostream& operator<<(std::ostream& os, const ArithData<arith_t>& o) {
    os << o.value();
    return os;
}

} // namespace
