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
        ArithData(EmptyData) : ArithData() {}
        ArithData() : value_(0) {}
        ArithData(float value) : value_(value) {}

        float value() const { return value_; }
        void value(float value) { value_ = value; }

        bool relevant() const {
            return true;
        }

        bool rm_relevant() const {       
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
        bool ands(const T&) const {
            // shortened from:
            // return ande(o).relevant();
            return relevant();
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
        bool ors(const T&) const {
            return relevant();
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
        bool rms(const T&) const {
            return rm_relevant();
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
        float value_;
};

template<typename arith_t>
inline std::ostream& operator<<(std::ostream& os, const ArithData<arith_t>& o) {
    os << o.value();
    return os;
}

// ======= fancyness to give ArithData SDRs the division op ===============

template<typename, typename>
class SDR;

template<typename, typename>
class SDRElem;

template<typename ret_id_t, typename c_ret_t, typename ret_arith_t, typename id_t, typename container_t, typename arith_t, typename arg_id_t, typename c_arg_t, typename arg_arith_t>
SDR<SDRElem<ret_id_t, ArithData<ret_arith_t>>, c_ret_t> divide(const SDR<SDRElem<id_t, ArithData<arith_t>>, container_t>& a, const SDR<SDRElem<arg_id_t, ArithData<arg_arith_t>>, c_arg_t>& b) {
    SDR<SDRElem<ret_id_t, ArithData<ret_arith_t>>, c_ret_t> r;
    std::function<void(typename container_t::iterator)> visitor_this;
    auto visitor_query = [](typename c_arg_t::iterator){};
    std::function<void(typename container_t::iterator, typename c_arg_t::iterator)> visitor_both;
    [[maybe_unused]] typename c_ret_t::const_iterator it;

    if constexpr(flistLike<c_ret_t>::value) {
        it = r.before_begin();
        visitor_this = [&](typename container_t::iterator this_pos) {
            // pass through
            it = r.insert_after(it, SDRElem<ret_id_t, ArithData<ret_arith_t>>(*this_pos));
        };

        visitor_both = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            // no relevance check since ArithData is always relevant
            SDRElem<ret_id_t, ArithData<ret_arith_t>> elem(this_pos->id(), this_pos->data().value() / arg_pos->data().value());
            it = r.insert_after(it, elem);
        };

    } else {
        visitor_this = [&](typename container_t::iterator this_pos) {
            SDRElem<ret_id_t, ArithData<ret_arith_t>> elem(*this_pos);
            r.push_back(elem);
        };

        visitor_both = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            SDRElem<ret_id_t, ArithData<ret_arith_t>> elem(this_pos->id(), this_pos->data().value() / arg_pos->data().value());
            r.push_back(elem);
        };
    }
    const_cast<SDR<SDRElem<id_t, ArithData<arith_t>>, container_t>&>(a).orv(const_cast<SDR<SDRElem<arg_id_t, ArithData<arg_arith_t>>, c_arg_t>&>(b), visitor_this, visitor_query, visitor_both);
    return r;
}

template<typename id_t, typename container_t, typename arith_t, typename arg_id_t, typename c_arg_t, typename arg_arith_t>
SDR<SDRElem<id_t, ArithData<arith_t>>, container_t> operator/(const SDR<SDRElem<id_t, ArithData<arith_t>>, container_t>& a, const SDR<SDRElem<arg_id_t, ArithData<arg_arith_t>>, c_arg_t>& b) {
    // assumes that the returned container and id type are the same as the first arg's container and id type
    return divide<id_t, container_t, arith_t>(a, b);
}

template<typename id_t, typename container_t, typename arith_t, typename arg_id_t, typename c_arg_t, typename arg_arith_t>
SDR<SDRElem<id_t, ArithData<arith_t>>, container_t>& operator/=(SDR<SDRElem<id_t, ArithData<arith_t>>, container_t>& a, const SDR<SDRElem<arg_id_t, ArithData<arg_arith_t>>, c_arg_t>& b) {
    auto visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
        this_pos->data().value(this_pos->data().value() / arg_pos->data().value());
    };

    a.andv(const_cast<SDR<SDRElem<arg_id_t, ArithData<arg_arith_t>>, c_arg_t>&>(b), visitor);
    return a;
}

} // namespace
