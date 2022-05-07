#pragma once

#include <type_traits>
#include <forward_list>
#include <vector>
#include <set>
#include <iostream>

#include "SparseDistributedRepresentation/DataTypes/EmptyData.hpp"

namespace SparseDistributedRepresentation {

/*
An SDR element type has an id, and (optionally) some data.
When ops are computed, the relevant elements (identified by a matching id)
and their data are combined in the result.
*/
template<typename id_t = int,
         typename data_t = EmptyData>
struct SDR_t {
    static_assert(std::is_integral<id_t>::value);

    constexpr SDR_t(id_t id, data_t data) : id(id), data(data) {}
    constexpr SDR_t(id_t id) : id(id), data() {}
    constexpr SDR_t() : id(), data() {}

    using id_type = id_t;
    using data_type = data_t;

    id_t id;
    data_t data;

    template<typename id_t_inner, 
             typename data_t_inner>
    friend std::ostream& operator<<(std::ostream&,
        const SDR_t<id_t_inner, data_t_inner>&);

    template<typename id_other, typename data_other>
    constexpr bool operator<(const SDR_t<id_other, data_other>& o) const {
        return id < o.id;
    }

    template<typename id_other, typename data_other>
    constexpr bool operator==(const SDR_t<id_other, data_other>& o) const {
        return id == o.id;
    }

    constexpr bool operator<(const id_t& o) const { return id < o; }
    constexpr bool operator==(const id_t& o) const { return id == o; }
    constexpr bool operator>(const id_t& o) const { return id > o; }

    constexpr operator id_t() const { return id; }

    constexpr SDR_t& operator+=(int o) {
        id += o;
        return *this;
    }
};

template<typename id_t,
         typename data_t>
std::ostream& operator<<(std::ostream& os, const SDR_t<id_t, data_t>& o) {
  os << o.id;
  if constexpr(sizeof(data_t) > 0) {
      os << "(" << o.data << ")";
  }
  return os;
}

} // namespace SparseDistributedRepresentation
