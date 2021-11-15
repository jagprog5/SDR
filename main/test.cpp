#define BOOST_TEST_MODULE sparse_distribued_representation_test_module
#include <boost/test/included/unit_test.hpp>
#include "SparseDistributedRepresentation.hpp"

BOOST_AUTO_TEST_SUITE( test_sdr )

BOOST_AUTO_TEST_CASE( test_andop_simple ) {
    BOOST_CHECK_EQUAL((SDR<>{1, 2, 3} & SDR<>{2, 3, 4}), (SDR<>{1, 2, 3}));
    BOOST_CHECK_EQUAL((SDR<>{1, 2, 3} && SDR<>{2, 3, 4}), 2);
}

// TODO add more tests

BOOST_AUTO_TEST_SUITE_END()


// BOOST_AUTO_TEST_CASE(MyTestCase)
// {
//     // To simplify this example test, let's suppose we'll test 'float'.
//     // Some test are stupid, but all should pass.
//     float x = 9.5f;

//     BOOST_CHECK(x == 0.0f);
//     BOOST_CHECK_EQUAL((int)x, 9);
//     BOOST_CHECK_CLOSE(x, 9.5f, 0.0001f); // Checks differ no more then 0.0001%
// }