#pragma once

#include <type_traits>
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
        using id_type = id_t;
        using data_type = data_t;

        template<typename dat>
        SDRElem(id_t id, dat&& data) : id_(id), data_(std::forward<dat>(data)) {}
        explicit SDRElem(id_t id) : id_(id), data_() {}
        SDRElem() : id_(), data_() {}

        SDRElem(const SDRElem& o) : id_(o.id()), data_(o.data()) {}

        template<typename o_id_t, typename o_data_t>
        explicit SDRElem(const SDRElem<o_id_t, o_data_t>& o) : id_(o.id()), data_(o.data()) {}

        SDRElem& operator=(const SDRElem& o) {
            const_cast<id_t&>(id_) = o.id();
            data_ = o.data();
            return *this;
        }

        SDRElem(SDRElem&& o) noexcept : id_(std::move(o.id())), data_(std::move(o.data())) {}

        SDRElem& operator=(SDRElem&& o) noexcept {
            const_cast<id_t&>(id_) = o.id();
            data_ = std::move(o.data());
            return *this;
        }

        const id_t& id() const { return id_; }
        data_t& data() { return data_; }
        const data_t& data() const { return data_; }
        void data(data_t data) { data_ = data; }

        template<typename id_t_inner, 
                typename data_t_inner>
        friend std::ostream& operator<<(std::ostream&,
            const SDRElem<id_t_inner, data_t_inner>&);

        template<typename id_other, typename data_other>
        bool operator<(const SDRElem<id_other, data_other>& o) const {
            return id() < o.id();
        }

        template<typename id_other, typename data_other>
        bool operator==(const SDRElem<id_other, data_other>& o) const {
            // equality of SDRElem disregards the data equality
            // this make sense since the elements should be ordered by id
            return id() == o.id();
        }

        template<typename id_other, typename data_other>
        bool operator>(const SDRElem<id_other, data_other>& o) const {
            return id() > o.id();
        }

        bool operator<(id_t o) const { return id() < o; }
        bool operator==(id_t o) const { return id() == o; }
        bool operator>(id_t o) const { return id() > o; }
    
    private:
        const id_t id_;
        data_t data_;
};

template<typename id_t,
         typename data_t>
std::ostream& operator<<(std::ostream& os, const SDRElem<id_t, data_t>& o) {
  os << o.id();
  // NOLINTNEXTLINE
  if constexpr(sizeof(data_t) > 0) {
      os << "(" << o.data() << ")";
  }
  return os;
}

} // namespace sparse_distributed_representation

