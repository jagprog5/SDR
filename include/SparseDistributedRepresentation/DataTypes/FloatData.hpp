#pragma once

#include <functional>
#include "SparseDistributedRepresentation/Templates.hpp"

namespace sparse_distributed_representation {

class FloatData {
    public:
        constexpr FloatData() : value_(0) {}
        constexpr FloatData(float value) : value_(value) {}

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

        constexpr FloatData ande(const FloatData& o) const {
            return FloatData(this->value() * o.value());
        }

        constexpr FloatData ore(const FloatData& o) const {
            return FloatData(this->value() + o.value());
        }

        // xor doesn't make sense in this context
        // instead we have divide
        constexpr auto operator/(const FloatData& o) const { return value() / o.value(); }

        constexpr FloatData& operator/=(const FloatData& o) {
            value_ /= o.value();
            return *this;
        }

        constexpr FloatData rme(const FloatData& o) const {
            return FloatData(value() - o.value());
        }

        template<typename T>
        constexpr bool operator==(const T& o) const {
            return value() == ((FloatData)o).value();
        }

    private:
        float value_;
};

inline std::ostream& operator<<(std::ostream& os, const FloatData& o) {
    os << o.value();
    return os;
}

// ======= fancyness to give FloatData SDRs the division op ===============

template<typename SDR_t, typename container_t>
class SDR;

template<typename id_t, typename data_t>
class SDR_t;

template<typename ret_id_t, typename c_ret_t, typename id_t, typename container_t, typename arg_id_t, typename c_arg_t>
SDR<SDR_t<ret_id_t, FloatData>, c_ret_t> divide(const SDR<SDR_t<id_t, FloatData>, container_t>& a, const SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>& b) {
    SDR<SDR_t<ret_id_t, FloatData>, c_ret_t> r;
    std::function<void(typename container_t::iterator)> visitor_this;
    auto visitor_query = [](typename c_arg_t::iterator){};
    std::function<void(typename container_t::iterator, typename c_arg_t::iterator)> visitor_both;
    [[maybe_unused]] typename c_ret_t::const_iterator it;

    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.before_begin();
        visitor_this = [&](typename container_t::iterator this_pos) {
            // pass through
            it = r.insert_after(it, SDR_t<ret_id_t, FloatData>(*this_pos));
        };

        visitor_both = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            // no relevance check since FloatData is always relevant
            SDR_t<ret_id_t, FloatData> elem(this_pos->id(), this_pos->data() / arg_pos->data());
            it = r.insert_after(it, elem);
        };

    } else {
        visitor_this = [&](typename container_t::iterator this_pos) {
            SDR_t<ret_id_t, FloatData> elem(*this_pos);
            r.push_back(elem);
        };

        visitor_both = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
            SDR_t<ret_id_t, FloatData> elem(this_pos->id(), this_pos->data() / arg_pos->data());
            r.push_back(elem);
        };
    }
    const_cast<SDR<SDR_t<id_t, FloatData>, container_t>&>(a).orv(const_cast<SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>&>(b), visitor_this, visitor_query, visitor_both);
    return r;
}

template<typename id_t, typename container_t, typename arg_id_t, typename c_arg_t>
SDR<SDR_t<id_t, FloatData>, container_t> operator/(const SDR<SDR_t<id_t, FloatData>, container_t>& a, const SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>& b) {
    // assumes that the returned container and id type are the same as the first arg's container and id type
    return divide<id_t, container_t>(a, b);
}

template<typename id_t, typename container_t, typename arg_id_t, typename c_arg_t>
SDR<SDR_t<id_t, FloatData>, container_t>& operator/=(SDR<SDR_t<id_t, FloatData>, container_t>& a, const SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>& b) {
    auto visitor = [&](typename container_t::iterator this_pos, typename c_arg_t::iterator arg_pos) {
        this_pos->data() /= arg_pos->data();
    };

    a.andv(const_cast<SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>&>(b), visitor);
    return a;
}

} // namespace