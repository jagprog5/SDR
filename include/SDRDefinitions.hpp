#pragma once

#include <type_traits>
#include <forward_list>
#include <vector>
#include <set>
#include <iostream>

namespace SparseDistributedRepresentation {

// if the underlying container does not have a size, then declare a size member, and inherit true_type
template <typename container_t, typename = void>
struct MaybeSize {
    typename container_t::size_type size;
};

template <typename container_t>
struct MaybeSize<container_t, decltype((void)container_t().size(), void())> {
};

// the below template is specific to stl containers only.
// in an effort to keep things generic, the presence or lack of certain members is checked for instead

// template <typename T>
// struct isForwardList : std::false_type {};

// template <typename T, typename A>
// struct isForwardList<std::forward_list<T, A>> : std::true_type {};

template <typename T, typename = void>
struct isForwardList : std::true_type {};

template <typename T>
struct isForwardList<T, decltype((void)T().size(), void())> : std::false_type {};

template <typename T>
struct isVector : std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<typename T::iterator>::iterator_category> {};

struct SDRFloatData;

struct EmptyStruct {
    // this is the data_type for a default SDR_t which does not use the data functionality

    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wzero-length-array")
            #pragma GCC diagnostic ignored "-Wzero-length-array"
        #endif
    #endif
    // if this member wasn't here, the struct size would be 1, instead of 0
    char unused[0];
    #pragma GCC diagnostic pop

    constexpr bool relevant() {
        // if any element is combined with this one, then it is in the result
        return true;
    }

    constexpr bool rm_relevant() {       
        // if any element is removed from this one, then it is not in the result 
        return false;
    }

    explicit constexpr operator SDRFloatData() const;

    template<typename arg_t>
    constexpr EmptyStruct andb([[maybe_unused]] const arg_t& o) const {
        return EmptyStruct();
    }

    template<typename arg_t>
    constexpr EmptyStruct orb([[maybe_unused]] const arg_t& o) const {
        return EmptyStruct();
    }

    template<typename arg_t>
    constexpr EmptyStruct xorb([[maybe_unused]] const arg_t& o) const {
        return EmptyStruct();
    }

    template<typename arg_t>
    constexpr EmptyStruct rmb([[maybe_unused]] const arg_t& o) const {
        return EmptyStruct();
    }
};

struct SDRFloatData {
    constexpr SDRFloatData() : value(1) {}
    constexpr SDRFloatData(float value) : value(value) {}
    float value;

    template <typename T, typename = void>
    struct has_value : std::false_type {};

    template <typename T>
    struct has_value<T, decltype((void)T::value, void())> : std::true_type {};

    constexpr bool relevant() {
        return value > 0.1;
    }

    constexpr bool rm_relevant() {       
        return relevant();
    }

    explicit constexpr operator EmptyStruct() const { return EmptyStruct(); }

    template<typename arg_t>
    constexpr SDRFloatData andb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return SDRFloatData(this->value * o.value);
        } else {
            return SDRFloatData(this->value);
        }
    }

    template<typename arg_t>
    constexpr SDRFloatData orb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return SDRFloatData(this->value > o.value ? this->value : o.value);
        } else {
            return SDRFloatData(1);
        }
    }

    template<typename arg_t>
    constexpr SDRFloatData xorb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return SDRFloatData(std::abs(this->value - o.value));
        } else {
            return SDRFloatData(0);
        }
    }

    template<typename arg_t>
    constexpr SDRFloatData rmb(const arg_t& o) const {
        if constexpr(has_value<arg_t>::value) {
            return SDRFloatData(this->value * (1 - o.value));
        } else {
            return SDRFloatData(0);
        }
    }

};

constexpr EmptyStruct::operator SDRFloatData() const {
    return SDRFloatData(1);
}

[[maybe_unused]] std::ostream& operator<<(std::ostream& os, const SDRFloatData& o) {
    auto val = o.value;
    if (val > 1 || val < 0) {
        os << "!!!";
    } else {
        os << '.';
        val *= 10;
        os << (int)val;
        val -= (int)val;
        val *= 10;
        os << (int)val;
    }
    return os;
}

template<typename id_t = int,
         typename data_t = EmptyStruct>
struct SDR_t {
    /*
    An SDR element type has an id, and (optionally) some data.
    When ops are computed, the relevant elements (identified by a matching id)
    and their data are combined in the result.
    */

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

    constexpr SDR_t(id_t id, data_t data) : id(id), data(data) {}
    constexpr SDR_t(id_t id) : id(id), data() {}
    constexpr SDR_t() : id(), data() {}
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

