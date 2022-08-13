#pragma once

#include <type_traits>

namespace sparse_distributed_representation {

// ========== helper templates ===============

// if the underlying container does not have a size, then declare a size member
template<typename container_t, typename = void>
struct MaybeSize {
    typename container_t::size_type size;
};

template<typename container_t>
struct MaybeSize<container_t, decltype((void)container_t().size(), void())> {};

// the below template is specific to stl containers only.
// in an effort to keep things generic, the presence or lack of certain members is checked for instead

// template<typename T>
// struct isForwardList : std::false_type {};

// template<typename T, typename A>
// struct isForwardList<std::forward_list<T, A>> : std::true_type {};

template<typename T, typename = void>
struct flistLike : std::true_type {};

template<typename T>
struct flistLike<T, decltype((void)T().size(), void())> : std::false_type {};

template<typename T>
struct vectorLike : std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<typename T::iterator>::iterator_category> {};

template<typename T, typename = void>
struct setLike : std::false_type {};

template<typename T>
struct setLike<T, decltype((void)T().lower_bound(typename T::value_type()), void())> : std::true_type {};

template<typename SDRElem_t, std::size_t N>
class ArrayAdaptor;

template<typename T>
struct isArrayAdaptor : std::false_type {};

template<typename T, size_t N>
struct isArrayAdaptor<ArrayAdaptor<T, N>> : std::true_type {};

template<typename T, typename = void>
struct setComparatorCheck : std::false_type {};

template<typename T>
struct setComparatorCheck<T, decltype((void)T().lower_bound(typename T::value_type::id_type()), void())> : std::true_type {};

} // namespace