#define BOOST_TEST_MODULE sparse_distribued_representation_test_module
#include <boost/test/included/unit_test.hpp>
#include "SparseDistributedRepresentation.hpp"

using namespace SparseDistributedRepresentation;

BOOST_AUTO_TEST_SUITE(sdr)

BOOST_AUTO_TEST_CASE(andop) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} & SDR<>{2, 3, 4}), (SDR<>{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} && SDR<>{2, 3, 4}), 2);
}

BOOST_AUTO_TEST_CASE(andop_range) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 5, 20}.andb(2, 7)), (SDR<>{2, 3, 5}));
}

BOOST_AUTO_TEST_CASE(append) {
  SDR<> a{1, 2, 3};
  SDR<> b{4, 5, 6};
  a.append(b);
  BOOST_REQUIRE_EQUAL(a, (SDR<>{1, 2, 3, 4, 5, 6}));
}

BOOST_AUTO_TEST_CASE(sample_portion) {
  SDR<> a{1, 2, 3};
  a *= 0.5;
  BOOST_REQUIRE(a.size() <= 3);
  auto it = a.cbegin();

  while (it != a.cend()) {
    BOOST_REQUIRE(*it > 0 && *it < 4);
    ++it;
  }
}

BOOST_AUTO_TEST_CASE(andop_inplace) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} &= SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{1, 2, 99}));
}

BOOST_AUTO_TEST_CASE(orop) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} | SDR<>{2, 3, 4}), (SDR<>{1, 2, 3, 4}));
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} || SDR<>{2, 3, 4}), 4);
}

BOOST_AUTO_TEST_CASE(orop_inplace) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} |= SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{0, 1, 2, 3, 99, 100}));
}

BOOST_AUTO_TEST_CASE(xorop) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} ^ SDR<>{2, 3, 4}), (SDR<>{1, 4}));
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} / SDR<>{2, 3, 4}), 2);
}

BOOST_AUTO_TEST_CASE(xorop_inplace) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} ^= SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{0, 3, 100}));
}

BOOST_AUTO_TEST_CASE(rm) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} - SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{3}));
}

BOOST_AUTO_TEST_CASE(test_encode) {
  BOOST_REQUIRE_EQUAL(SDR<>(0, 3, 100), (SDR<>{0, 1, 2}));
  BOOST_REQUIRE_EQUAL(SDR<>(0.5, 3, 100), (SDR<>{49, 50, 51}));
  BOOST_REQUIRE_EQUAL(SDR<>(1, 3, 100), (SDR<>{97, 98, 99}));
}

BOOST_AUTO_TEST_CASE(test_encode_periodic) {
  std::mt19937 random_number(time(NULL) * getpid());
  static constexpr int DENSE_LENGTH = 100;
  static constexpr int SPARSE_LENGTH = 3;
  float rand_input_0 = (double)(random_number() % 10) / 10;
  int rand_period = random_number() % 9 + 1;
  float rand_input_1 = rand_input_0 + rand_period * (random_number() % 7);

  // there's a case where the input is directly between two representations.
  float progress = rand_input_1 / rand_period * DENSE_LENGTH;
  progress -= (int)progress;
  if (std::abs(progress - 0.5) < 0.0001) {
    rand_input_0 += 0.0001;
    rand_input_1 += 0.0001;
  }

  SDR<> a(rand_input_0, rand_period, SPARSE_LENGTH, DENSE_LENGTH);
  SDR<> b(rand_input_1, rand_period, SPARSE_LENGTH, DENSE_LENGTH);
  BOOST_REQUIRE_EQUAL(a, b);
}

BOOST_AUTO_TEST_CASE(test_comparison) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_LE((SDR<>{1, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_GE((SDR<>{1, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_NE((SDR<>{0, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_LT((SDR<>{0, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_GT((SDR<>{4}), (SDR<>{0, 1, 2}));
}

BOOST_AUTO_TEST_CASE(test_ret_type) {
  {
    SDR<> a {1, 2, 3};
    SDR<> b {2, 3, 4};
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_and = a.andb<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_and, a.andb(b));
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_or = a.orb<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_or, a.orb(b));
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_xor = a.xorb<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_xor, a.xorb(b));
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_rm = a.rmb<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_rm, a.rmb(b));
  }
  {
    SDR<> a {1, 2, 3};
    SDR<> b {2, 3, 4};
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_and = a.andb<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_and, a.andb(b));
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_or = a.orb<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_or, a.orb(b));
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_xor = a.xorb<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_xor, a.xorb(b));
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_rm = a.rmb<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_rm, a.rmb(b));
  }
}

BOOST_AUTO_TEST_SUITE_END()
