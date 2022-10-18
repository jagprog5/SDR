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
struct flist_like : std::true_type {};

template<typename T>
struct flist_like<T, decltype((void)T().size(), void())> : std::false_type {};

template<typename T>
struct vector_like : std::is_base_of<std::random_access_iterator_tag, typename std::iterator_traits<typename T::iterator>::iterator_category> {};

template<typename T, typename = void>
struct set_like : std::false_type {};

template<typename T>
struct set_like<T, decltype((void)T().lower_bound(typename T::value_type()), void())> : std::true_type {};

template<typename T, typename = void>
struct set_comparator_check : std::false_type {};

template<typename T>
struct set_comparator_check<T, decltype((void)T().lower_bound(typename T::value_type::id_type()), void())> : std::true_type {};

template<typename old_t, typename new_t, typename T>
struct ___replace_type_in_params;

// if the subject is the old type, replace it with the new type, else leave it unchanged.
// applies recursively to all template arguments
// e.g. replace_type<int, float, std::vector<int>> -> std::vector<float>
// note that the allocator is changed as well
template<typename old_t, typename new_t, typename subject_t>
using replace_type = typename std::conditional<std::is_same_v<subject_t, old_t>, new_t,
    typename ___replace_type_in_params<old_t, new_t, subject_t>::value>::type;

template<typename old_t, typename new_t, typename T>
struct ___replace_type_in_params {
    // this case happens when the type is not a template
    using value = T;
};

template<typename old_t,
    typename new_t,
    template <typename...> class T, typename arg_element>
struct ___replace_type_in_params<old_t, new_t, T<arg_element>> {
    // base case. just consumed last template argument
    using value = T<replace_type<old_t, new_t, arg_element>>;
};

template<typename old_t,
    typename new_t,
    template <typename...> class T, typename arg_element, typename... arg_p>
struct ___replace_type_in_params<old_t, new_t, T<arg_element, arg_p...>> {
    using value = T<replace_type<old_t, new_t, arg_element>, ___replace_type_in_params<old_t, new_t, T<arg_p...>>>;
};

// declares a container, but it instead stores new_item_t
template<typename container_t, typename new_item_t>
using replace_value_type = replace_type<typename container_t::value_type, new_item_t, container_t>;

} // namespace
