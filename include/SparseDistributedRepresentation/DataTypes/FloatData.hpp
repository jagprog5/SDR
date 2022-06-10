#pragma once

#include <functional>
#include "SparseDistributedRepresentation/Templates.hpp"

namespace sparse_distributed_representation {

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

    // for compatibility with other data types
    template<typename T>
    explicit constexpr operator T() const { return T(value); }

    constexpr FloatData ande(const FloatData& o) const {
        return FloatData(this->value * o.value);
    }

    constexpr FloatData ore(const FloatData& o) const {
        return FloatData(this->value + o.value);
    }

    // xor doesn't make sense in this context
    // instead we have divide
    auto operator/(const FloatData& o) const { return value / o.value; }
    auto operator/=(const FloatData& o) { return value /= o.value; }

    constexpr FloatData rme(const FloatData& o) const {
        return FloatData(this->value - o.value);
    }

    template<typename T>
    constexpr bool operator==(const T& o) const {
        return value == ((FloatData)o).value;
    }

};

std::ostream& operator<<(std::ostream& os, const FloatData& o) {
    os << o.value;
    return os;
}

// ======= fancyness to give FloatData SDRs the division op ===============

template<typename SDR_t, typename container_t>
class SDR;

template<typename id_t, typename data_t>
struct SDR_t;

template<typename ret_id_t, typename c_ret_t, typename id_t, typename container_t, typename arg_id_t, typename c_arg_t>
SDR<SDR_t<ret_id_t, FloatData>, c_ret_t> divide(const SDR<SDR_t<id_t, FloatData>, container_t>& a, const SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>& b) {
    SDR<SDR_t<ret_id_t, FloatData>, c_ret_t> r;
    std::function<void(const id_t&, FloatData&)> visitor_this;
    std::function<void(const id_t&, FloatData&, FloatData&)> visitor_both;
    [[maybe_unused]] typename c_ret_t::const_iterator it;

    if constexpr(isForwardList<c_ret_t>::value) {
        it = r.before_begin();
        visitor_this = [&](const id_t& this_id, FloatData& this_data) {
            // pass through
            it = r.insert_after(it, SDR_t<ret_id_t, FloatData>(this_id, this_data));
        };

        visitor_both = [&](const id_t& this_id, FloatData& this_data, FloatData& arg_data) {
            // no relevance check since FloatData is always relevant
            SDR_t<ret_id_t, FloatData> elem(this_id, this_data / arg_data);
            it = r.insert_after(it, elem);
        };

    } else {
        visitor_this = [&](const id_t& this_id, FloatData& this_data) {
            SDR_t<ret_id_t, FloatData> elem(this_id, this_data);
            r.push_back(elem);
        };

        visitor_both = [&](const id_t& this_id, FloatData& this_data, FloatData& arg_data) {
            SDR_t<ret_id_t, FloatData> elem(this_id, this_data / arg_data);
            r.push_back(elem);
        };
    }
    const_cast<SDR<SDR_t<id_t, FloatData>, container_t>&>(a).rmv(const_cast<SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>&>(b), visitor_this, visitor_both);
    return r;
}

template<typename id_t, typename container_t, typename arg_id_t, typename c_arg_t>
SDR<SDR_t<id_t, FloatData>, container_t> operator/(const SDR<SDR_t<id_t, FloatData>, container_t>& a, const SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>& b) {
    // assumes that the returned container and id type are the same as the first arg's container and id type
    return divide<id_t, container_t>(a, b);
}

template<typename id_t, typename container_t, typename arg_id_t, typename c_arg_t>
SDR<SDR_t<id_t, FloatData>, container_t>& operator/=(SDR<SDR_t<id_t, FloatData>, container_t>& a, const SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>& b) {
    auto visitor = [&](const id_t&, FloatData& this_data, FloatData& arg_data) {
        this_data /= arg_data;
    };

    a.andv(const_cast<SDR<SDR_t<arg_id_t, FloatData>, c_arg_t>&>(b), visitor);
    return a;
}

} // namespace