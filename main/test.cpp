#define BOOST_TEST_MODULE sparse_distribued_representation_test_module
#include <boost/test/included/unit_test.hpp>
#include "SparseDistributedRepresentation.hpp"

const int rand_sdr_range = 300;

SDR<> get_random_sdr() {
    SDR<> ret;
    for (int i = 0; i < 100; ++i) {
        ret += SDR<>::get_random_number() % rand_sdr_range;
    }
    return ret;
}

BOOST_AUTO_TEST_SUITE( sdr )

BOOST_AUTO_TEST_CASE( contains ) {
    SDR<> a = get_random_sdr();
    for (int i = -100; i < rand_sdr_range + 100; ++i) {
        auto val = SDR<>::get_random_number();
        bool expected = std::find(a.cbegin(), a.cend(), val) != a.cend();
        bool tested = a & val;
        BOOST_REQUIRE_EQUAL(expected, tested);
    }
}

BOOST_AUTO_TEST_CASE( andop_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} & SDR<>{2, 3, 4}), (SDR<>{2, 3}));
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} && SDR<>{2, 3, 4}), 2);
}

BOOST_AUTO_TEST_CASE( andop_generic ) {
    SDR<> a = get_random_sdr();
    SDR<> b = get_random_sdr();
    SDR<> result = a & b;
    SDR<>::const_iterator it;
    // for every element in a, if it is also in b, then it must be in the result
    for(it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        BOOST_REQUIRE_EQUAL(a_elem_in_b, result & a_elem);
    }
    // for every element in b, if it is also in a, then it must be in the result
    for(it = b.cbegin(); it != b.cend(); ++it) {
        auto b_elem = *it;
        bool b_elem_in_a = std::find(a.cbegin(), a.cend(), b_elem) != a.cend();
        BOOST_REQUIRE_EQUAL(b_elem_in_a, result & b_elem);
    }
    // the result can't contain any elements not in a or not in b
    for(it = result.cbegin(); it != result.cend(); ++it) {
        auto r_elem = *it;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        BOOST_REQUIRE(in_a || in_b);
    }
}

BOOST_AUTO_TEST_CASE( andop_range_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 5, 20}.andb(2, 7)), (SDR<>{2, 3, 5}));
}

BOOST_AUTO_TEST_CASE( andop_positions_simple ) {
    BOOST_REQUIRE((SDR<>{1, 2, 3, 99}.andp(SDR<>{2, 99})) == (std::vector<SDR<>::size_type>{1, 3}));
}

BOOST_AUTO_TEST_CASE( andop_inplace_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} &= SDR<>{0, 1, 2, 99, 100}), (SDR<>{1, 2, 99}));
}

BOOST_AUTO_TEST_CASE( orop_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} | SDR<>{2, 3, 4}), (SDR<>{1, 2, 3, 4}));
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} || SDR<>{2, 3, 4}), 4);
}

BOOST_AUTO_TEST_CASE( orop_generic ) {
    SDR<> a = get_random_sdr();
    SDR<> b = get_random_sdr();
    SDR<> result = a | b;
    SDR<>::const_iterator it;
    // every element in a must be in result
    for(it = a.cbegin(); it != a.cend(); ++it) {
        BOOST_REQUIRE(std::find(result.cbegin(), result.cend(), *it) != result.cend());
    }
    // every element in b must be in result
    for(it = b.cbegin(); it != b.cend(); ++it) {
        BOOST_REQUIRE(std::find(result.cbegin(), result.cend(), *it) != result.cend());
    }
    // the result can't contain any elements not in a or not in b
    for(it = result.cbegin(); it != result.cend(); ++it) {
        auto r_elem = *it;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        BOOST_REQUIRE(in_a || in_b);
    }
}

BOOST_AUTO_TEST_CASE( orop_inplace_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} |= SDR<>{0, 1, 2, 99, 100}), (SDR<>{0, 1, 2, 3, 99, 100}));
}

BOOST_AUTO_TEST_CASE( xorop_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} ^ SDR<>{2, 3, 4}), (SDR<>{1, 4}));
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} <= SDR<>{2, 3, 4}), 2);
}

BOOST_AUTO_TEST_CASE( xorop_generic ) {
    SDR<> a = get_random_sdr();
    SDR<> b = get_random_sdr();
    SDR<> result = a ^ b;
    SDR<>::const_iterator it;
    // for every element in a, if it is not in b, then it must be in the result
    for(it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        BOOST_REQUIRE_EQUAL(!a_elem_in_b, result & a_elem);
    }
    // for every element in b, if it is not in a, then it must be in the result
    for(it = b.cbegin(); it != b.cend(); ++it) {
        auto b_elem = *it;
        bool b_elem_in_a = std::find(a.cbegin(), a.cend(), b_elem) != a.cend();
        BOOST_REQUIRE_EQUAL(!b_elem_in_a, result & b_elem);
    }
    // the result can't contain any elements not in a or not in b
    for(it = result.cbegin(); it != result.cend(); ++it) {
        auto r_elem = *it;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        BOOST_REQUIRE(in_a || in_b);
    }
}

BOOST_AUTO_TEST_CASE( xorop_inplace_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} ^= SDR<>{0, 1, 2, 99, 100}), (SDR<>{0, 3, 100}));
}

BOOST_AUTO_TEST_CASE( rm_simple ) {
    BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} - SDR<>{0, 1, 2, 99, 100}), (SDR<>{3}));
}

BOOST_AUTO_TEST_CASE( rm_generic ) {
    SDR<> a = get_random_sdr();
    SDR<> b = get_random_sdr();
    SDR<> result = a - b;
    SDR<>::const_iterator it;
    // for all elements in a, if it is not in b, then it must be in the result
    int i = 0;
    for(it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        BOOST_REQUIRE_EQUAL(!a_elem_in_b, result & a_elem);
        if (!a_elem_in_b) i++;
    }
    // the result can contain no other elements
    BOOST_REQUIRE_EQUAL(i, result.size());
}

BOOST_AUTO_TEST_CASE( separate_simple ) {
    SDR<> a{1, 2, 3, 4};
    SDR<> b{3, 4, 5, 6};
    SDR<>::separate(a, b);
    BOOST_REQUIRE_EQUAL(a, (SDR<>{1, 2}));
    BOOST_REQUIRE_EQUAL(b, (SDR<>{5, 6}));
}

BOOST_AUTO_TEST_CASE( separate_generic ) {
    SDR<> a = get_random_sdr();
    SDR<> a_before(a);
    SDR<> b = get_random_sdr();
    SDR<> b_before(b);
    SDR<>::separate(a, b);
    BOOST_REQUIRE_EQUAL(a, a_before - b_before);
    BOOST_REQUIRE_EQUAL(b, b_before - a_before);
}

BOOST_AUTO_TEST_CASE( empty_operands ) {
    BOOST_REQUIRE_EQUAL(SDR<>() & SDR<>(), SDR<>());
    BOOST_REQUIRE_EQUAL((SDR<>{1}) & SDR<>(), SDR<>());
    BOOST_REQUIRE_EQUAL(SDR<>() & (SDR<>{1}), SDR<>());
    BOOST_REQUIRE_EQUAL(SDR<>() &= (SDR<>{1}), SDR<>());
    BOOST_REQUIRE_EQUAL((SDR<>{1}) &= SDR<>(), SDR<>());

    BOOST_REQUIRE_EQUAL(SDR<>() | SDR<>(), SDR<>());
    BOOST_REQUIRE_EQUAL((SDR<>{1}) | SDR<>(), (SDR<>{1}));
    BOOST_REQUIRE_EQUAL(SDR<>() |= (SDR<>{1}), (SDR<>{1}));

    BOOST_REQUIRE_EQUAL(SDR<>() - (SDR<>{1}), SDR<>());
    BOOST_REQUIRE_EQUAL((SDR<>{1}) - SDR<>(), (SDR<>{1}));
}

BOOST_AUTO_TEST_SUITE_END()
