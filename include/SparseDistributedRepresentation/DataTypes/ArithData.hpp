#pragma once

#include <functional>
#include "SparseDistributedRepresentation/Templates.hpp"

namespace sparse_distributed_representation {

template<typename arith_t = float>
class ArithData {
    public:
        constexpr ArithData() : value_(0) {}
        constexpr ArithData(float value) : value_(value) {}

        constexpr float value() const { return value_; }
        constexpr void value(float value) { value_ = value; }

        constexpr bool relevant() const {
            return true;
        }

        constexpr bool rm_relevant() const {       
            return relevant();
        }

        // for compatibility with other data types
        template<typename T>
        explicit constexpr operator T() const { return T(value); }

        constexpr ArithData ande(const ArithData& o) const {
            return ArithData(this->value() * o.value());
        }

        constexpr ArithData ore(const ArithData& o) const {
            return ArithData(this->value() + o.value());
        }

        // xor doesn't make sense in this context
        // instead we have divide
        constexpr auto operator/(const ArithData& o) const { return value() / o.value(); }

        constexpr ArithData& operator/=(const ArithData& o) {
            value_ /= o.value();
            return *this;
        }

        constexpr ArithData rme(const ArithData& o) const {
            return ArithData(value() - o.value());
        }

        template<typename T>
        constexpr bool operator==(const T& o) const {
            return value() == ((ArithData)o).value();
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

    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.before_begin();
        visitor_this = [&](typename container_t::iterator this_pos) {
            // pass through
            it = r.insert_after(it, SDRElem<ret_id_t, ArithData<ret_arith_t>>(*this_pos));
        };

        visitor_both = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            // no relevance check since ArithData is always relevant
            SDRElem<ret_id_t, ArithData<ret_arith_t>> elem(this_pos->id(), this_pos->data() / arg_pos->data());
            it = r.insert_after(it, elem);
        };

    } else {
        visitor_this = [&](typename container_t::iterator this_pos) {
            SDRElem<ret_id_t, ArithData<ret_arith_t>> elem(*this_pos);
            r.push_back(elem);
        };

        visitor_both = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            SDRElem<ret_id_t, ArithData<ret_arith_t>> elem(this_pos->id(), this_pos->data() / arg_pos->data());
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
        this_pos->data() /= arg_pos->data();
    };

    a.andv(const_cast<SDR<SDRElem<arg_id_t, ArithData<arg_arith_t>>, c_arg_t>&>(b), visitor);
    return a;
}

} // namespace