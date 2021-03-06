#pragma once

#include <type_traits>
#include <forward_list>
#include <vector>
#include <set>
#include <iostream>

#include "SparseDistributedRepresentation/DataTypes/EmptyData.hpp"

namespace sparse_distributed_representation {

/*
An SDR element type has an id, and (optionally) some data.
When ops are computed, the relevant elements (identified by a matching id)
and their data are combined in the result.
*/
template<typename id_t = int,
         typename data_t = EmptyData>
class SDRElem {
    public:
        static_assert(std::is_integral<id_t>::value);
        using id_type = id_t;
        using data_type = data_t;

        constexpr SDRElem(id_t id, data_t data) : id_(id), data_(data) {}
        constexpr SDRElem(id_t id) : id_(id), data_() {}
        constexpr SDRElem() : id_(), data_() {}
        constexpr SDRElem(const SDRElem& o) : id_(o.id()), data_(o.data()) {}

        template<typename o_id_t, typename o_data_t>
        constexpr SDRElem(const SDRElem<o_id_t, o_data_t>& o) : id_(o.id()), data_(o.data()) {}

        constexpr SDRElem& operator=(const SDRElem& o) {
            const_cast<id_t&>(id_) = o.id();
            data_ = o.data();
            return *this;
        }

        constexpr SDRElem(SDRElem&& o) noexcept : id_(std::move(o.id())), data_(std::move(o.data())) {}

        constexpr SDRElem& operator=(SDRElem&& o) noexcept {
            const_cast<id_t&>(id_) = std::move(const_cast<id_t&>(o.id()));
            data_ = std::move(o.data());
            return *this;
        }

        constexpr const id_t& id() const { return id_; }
        constexpr data_t& data() { return data_; }
        constexpr const data_t& data() const { return data_; }
        constexpr void date(data_t data) { data_ = data; }

        template<typename id_t_inner, 
                typename data_t_inner>
        friend std::ostream& operator<<(std::ostream&,
            const SDRElem<id_t_inner, data_t_inner>&);

        template<typename id_other, typename data_other>
        constexpr bool operator<(const SDRElem<id_other, data_other>& o) const {
            return id() < o.id();
        }

        template<typename id_other, typename data_other>
        constexpr bool operator==(const SDRElem<id_other, data_other>& o) const {
            return id() == o.id();
        }

        template<typename id_other, typename data_other>
        constexpr bool operator>(const SDRElem<id_other, data_other>& o) const {
            return id() > o.id();
        }

        constexpr bool operator<(const id_t& o) const { return id() < o; }
        constexpr bool operator==(const id_t& o) const { return id() == o; }
        constexpr bool operator>(const id_t& o) const { return id() > o; }
    
    private:
        const id_t id_;
        data_t data_;
};

template<typename id_t,
         typename data_t>
std::ostream& operator<<(std::ostream& os, const SDRElem<id_t, data_t>& o) {
  os << o.id();
  // NOLINTNEXTLINE(bugprone-sizeof-expression)
  if constexpr(sizeof(data_t) > 0) {
      os << "(" << o.data() << ")";
  }
  return os;
}

} // namespace sparse_distributed_representation

