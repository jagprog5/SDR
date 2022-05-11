#define BOOST_TEST_MODULE sparse_distribued_representation_test_module
#include <boost/test/included/unit_test.hpp>
#include "SparseDistributedRepresentation/SDR.hpp"
#include "SparseDistributedRepresentation/DataTypes/FloatData.hpp"
#include "SparseDistributedRepresentation/DataTypes/UnitData.hpp"
#include <random>
#include <unistd.h>

using namespace SparseDistributedRepresentation;

BOOST_AUTO_TEST_SUITE(sdr)

BOOST_AUTO_TEST_CASE(copy_assignment_ctor) {
  SDR<SDR_t<>, std::forward_list<SDR_t<>>> a{1, 2, 3};
  SDR<SDR_t<>, std::forward_list<SDR_t<>>> b;
  b = a;
  BOOST_REQUIRE_EQUAL(b, a);
  BOOST_REQUIRE_EQUAL(b.size(), a.size());
}

BOOST_AUTO_TEST_CASE(move_ctor) {
  SDR<SDR_t<>, std::forward_list<SDR_t<>>> a{1, 2, 3};
  auto b(std::move(a));
  BOOST_REQUIRE_EQUAL(b.size(), 3);
  BOOST_REQUIRE_EQUAL(b, (SDR{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(move_assign_ctor) {
  SDR<SDR_t<>, std::forward_list<SDR_t<>>> a{1, 2, 3};
  SDR<SDR_t<>, std::forward_list<SDR_t<>>> b;
  b = std::move(a);
  BOOST_REQUIRE_EQUAL(b.size(), 3);
  BOOST_REQUIRE_EQUAL(b, (SDR{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(iter_ctor) {
  std::vector<int> v{1, 2, 3};
  BOOST_REQUIRE_EQUAL(SDR(v.begin(), v.end()), (SDR{1, 2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>(v.begin(), v.end())), (SDR{1, 2, 3}));
  auto a = SDR<SDR_t<>, std::forward_list<SDR_t<>>>(v.begin(), v.end());
  BOOST_REQUIRE_EQUAL(a.size(), 3);
  BOOST_REQUIRE_EQUAL(a, (SDR{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(init_list_ctor) {
  auto a = SDR<SDR_t<int, UnitData>, std::forward_list<SDR_t<int, UnitData>>>{SDR_t<int, UnitData>(1, 0.5), SDR_t<int, UnitData>(1, 0.0)};
  BOOST_REQUIRE_EQUAL(a.size(), 1);
  BOOST_REQUIRE_EQUAL(a, SDR{1});
}

BOOST_AUTO_TEST_CASE(test_encode) {
  BOOST_REQUIRE_EQUAL(SDR(0, 3, 100), (SDR{0, 1, 2}));
  BOOST_REQUIRE_EQUAL(SDR(0.5, 3, 100), (SDR{49, 50, 51}));
  BOOST_REQUIRE_EQUAL(SDR(1, 3, 100), (SDR{97, 98, 99}));

  BOOST_REQUIRE_EQUAL(SDR(0.8, 1, 3, 10), (SDR{0, 8, 9}));
  BOOST_REQUIRE_EQUAL(SDR(0, 1, 3, 10), (SDR{0, 1, 2}));

  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>(0, 3, 100)), (SDR{0, 1, 2}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>(0.5, 3, 100)), (SDR{49, 50, 51}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>(1, 3, 100)), (SDR{97, 98, 99}));

  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>(0.8, 1, 3, 10)), (SDR<>{0, 8, 9}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>(0, 1, 3, 10)), (SDR{0, 1, 2}));

  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>(0, 3, 100)), (SDR{0, 1, 2}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>(0.5, 3, 100)), (SDR{49, 50, 51}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>(1, 3, 100)), (SDR{97, 98, 99}));

  auto b0 = SDR<SDR_t<>, std::forward_list<SDR_t<>>>(0.8, 1, 3, 10);
  BOOST_REQUIRE_EQUAL(b0, (SDR<>{0, 8, 9}));
  BOOST_REQUIRE_EQUAL(b0.size(), 3);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>(0, 1, 3, 10)), (SDR<>{0, 1, 2}));
}

BOOST_AUTO_TEST_CASE(andop_range) {
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ande(2, 7)), (SDR{2, 3, 5}));
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ands(2, 7)), 3);
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ande(2, 5)), (SDR{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ands(2, 5)), 2);
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ande(20, 70)), (SDR{20}));
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ands(20, 70)), 1);
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ande(0, 0)), (SDR{}));
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3, 5, 20}.ands(0, 0)), 0);
  
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ande(2, 7)), (SDR{2, 3, 5}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ands(2, 7)), 3);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ande(2, 5)), (SDR{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ands(2, 5)), 2);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ande(20, 70)), (SDR{20}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ands(20, 70)), 1);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ande(0, 0)), (SDR{}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::set<SDR_t<>>>{1, 2, 3, 5, 20}.ands(0, 0)), 0);

  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ande(2, 7)), (SDR{2, 3, 5}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ands(2, 7)), 3);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ande(2, 5)), (SDR{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ands(2, 5)), 2);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ande(20, 70)), (SDR{20}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ands(20, 70)), 1);
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ande(0, 0)), (SDR{}));
  BOOST_REQUIRE_EQUAL((SDR<SDR_t<>, std::forward_list<SDR_t<>>>{1, 2, 3, 5, 20}.ands(0, 0)), 0);
}

BOOST_AUTO_TEST_CASE(append) {
  SDR a0{1, 2, 3};
  SDR b0{4, 5, 6};
  a0.append(b0);
  BOOST_REQUIRE_EQUAL(a0, (SDR{1, 2, 3, 4, 5, 6}));

  SDR<SDR_t<>, std::set<SDR_t<>>> a1{1, 2, 3};
  SDR<SDR_t<>, std::set<SDR_t<>>> b1{4, 5, 6};
  a1.append(b1);
  BOOST_REQUIRE_EQUAL(a1, (SDR{1, 2, 3, 4, 5, 6}));

  SDR<SDR_t<>, std::forward_list<SDR_t<>>> a2{1, 2, 3};
  SDR<SDR_t<>, std::forward_list<SDR_t<>>> b2{4, 5, 6};
  a2.append(b2);
  BOOST_REQUIRE_EQUAL(a2.size(), 6);
  BOOST_REQUIRE_EQUAL(a2, (SDR{1, 2, 3, 4, 5, 6}));
}

BOOST_AUTO_TEST_CASE(sample_portion) {
  std::mt19937 twister(time(NULL) * getpid());

  SDR a{1, 2, 3};
  a.sample_portion(0.8, twister);
  BOOST_REQUIRE(a.size() <= 3);
  auto ita = a.cbegin();
  while (ita != a.cend()) {
    BOOST_REQUIRE(*ita > 0 && *ita < 4);
    ++ita;
  }

  SDR<SDR_t<>, std::set<SDR_t<>>> b{1, 2, 3};
  b.sample_portion(0.8, twister);
  BOOST_REQUIRE(b.size() <= 3);
  auto itb = b.cbegin();

  while (itb != b.cend()) {
    BOOST_REQUIRE(*itb > 0 && *itb < 4);
    ++itb;
  }

  SDR<SDR_t<>, std::forward_list<SDR_t<>>> c{1, 2, 3};
  c.sample_portion(0.8, twister);
  BOOST_REQUIRE(c.size() <= 3);
  auto itc = c.cbegin();

  while (itc != c.cend()) {
    BOOST_REQUIRE(*itc > 0 && *itc < 4);
    ++itc;
  }
}

BOOST_AUTO_TEST_CASE(test_comparison) {
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3}), (SDR{1, 2, 3}));
  BOOST_REQUIRE_LE((SDR{1, 2, 3}), (SDR{1, 2, 3}));
  BOOST_REQUIRE_GE((SDR{1, 2, 3}), (SDR{1, 2, 3}));
  BOOST_REQUIRE_NE((SDR{0, 2, 3}), (SDR{1, 2, 3}));
  BOOST_REQUIRE_LT((SDR{0, 2, 3}), (SDR{1, 2, 3}));
  BOOST_REQUIRE_GT((SDR{4}), (SDR{0, 1, 2}));
}

BOOST_AUTO_TEST_CASE(test_ret_type) {
  {
    SDR a {1, 2, 3};
    SDR b {2, 3, 4};
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_and = a.ande<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_and, a.ande(b));
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_or = a.ore<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_or, a.ore(b));
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_xor = a.xore<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_xor, a.xore(b));
    SDR<SDR_t<long>, std::forward_list<SDR_t<long>>> r_rm = a.rme<SDR_t<long>, std::forward_list<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_rm, a.rme(b));
  }
  {
    SDR a {1, 2, 3};
    SDR b {2, 3, 4};
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_and = a.ande<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_and, a.ande(b));
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_or = a.ore<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_or, a.ore(b));
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_xor = a.xore<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_xor, a.xore(b));
    SDR<SDR_t<long>, std::set<SDR_t<long>>> r_rm = a.rme<SDR_t<long>, std::set<SDR_t<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_rm, a.rme(b));
  }
}

BOOST_AUTO_TEST_CASE(test_float_data) {
  SDR<SDR_t<int, FloatData>>a{SDR_t<int, FloatData>(0, 3), SDR_t<int, FloatData>(1, 2), SDR_t<int, FloatData>(2, 1)};
  SDR<SDR_t<int, FloatData>>b{SDR_t<int, FloatData>(0, 2), SDR_t<int, FloatData>(1, 2), SDR_t<int, FloatData>(2, 2)};
  auto result = a - b;
  float val = 1;
  for (const auto& elem : result) {
    BOOST_REQUIRE_EQUAL(elem.data.value, val--);
  }
}

// BOOST_AUTO_TEST_CASE(test_printing) {
//   std::cout << FloatData(0.5555) << '\n';
//   UnitData a(0.5555);
//   std::cout << a << " ";
//   a.value = 1.1;
//   std::cout << a << '\n';
//   std::cout << SDR{1, 2, 3} << " " << SDR<SDR_t<int, FloatData>>{1, 2, 3} << '\n';
//   std::cout << SDR<SDR_t<int, FloatData>>{1, 2, 3} << std::endl;
// }

BOOST_AUTO_TEST_SUITE_END()
