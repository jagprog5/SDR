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

template<bool is_forward, typename T>
struct lesser_or_greater;

template<typename T>
struct lesser_or_greater<true, T> : std::less<T> {};

template<typename T>
struct lesser_or_greater<false, T> : std::greater<T> {};

// the below template is specific to stl containers only.
// in an effort to keep things generic, the presence or lack of certain members is checked for instead

// template<typename T>
// struct isForwardList : std::false_type {};

// template<typename T, typename A>
// struct isForwardList<std::forward_list<T, A>> : std::true_type {};

template<typename T, typename = void>
struct isForwardList : std::true_type {};

template<typename T>
struct isForwardList<T, decltype((void)T().size(), void())> : std::false_type {};

template<typename T>
struct isVector : std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<typename T::iterator>::iterator_category> {};

template<typename T, typename = void>
struct isSet : std::false_type {};

template<typename T>
struct isSet<T, decltype((void)T().lower_bound(typename T::value_type()), void())> : std::true_type {};

} // namespace