#define BOOST_TEST_MODULE sparse_distribued_representation_test_module
#include <boost/test/included/unit_test.hpp>
#include "SparseDistributedRepresentation.hpp"

BOOST_AUTO_TEST_SUITE(sdr)

BOOST_AUTO_TEST_CASE(andop_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} & SDR<>{2, 3, 4}), (SDR<>{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} && SDR<>{2, 3, 4}), 2);
}

BOOST_AUTO_TEST_CASE(andop_range_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 5, 20}.andb(2, 7)), (SDR<>{2, 3, 5}));
}

// BOOST_AUTO_TEST_CASE(andop_positions_simple) {
//   auto s = SDR<>{1, 2, 3, 99};
//   BOOST_REQUIRE((s.andp(SDR<>{2, 99})) ==
//                 (std::vector<SDR<>::index_type*>{std::advanced(s, 1),
//                                                  std::advanced(s, 3)}));
// }

BOOST_AUTO_TEST_CASE(andop_inplace_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} &= SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{1, 2, 99}));
}

BOOST_AUTO_TEST_CASE(orop_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} | SDR<>{2, 3, 4}), (SDR<>{1, 2, 3, 4}));
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} || SDR<>{2, 3, 4}), 4);
}

BOOST_AUTO_TEST_CASE(orop_inplace_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} |= SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{0, 1, 2, 3, 99, 100}));
}

BOOST_AUTO_TEST_CASE(xorop_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} ^ SDR<>{2, 3, 4}), (SDR<>{1, 4}));
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3} / SDR<>{2, 3, 4}), 2);
}

BOOST_AUTO_TEST_CASE(xorop_inplace_simple) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3, 99} ^= SDR<>{0, 1, 2, 99, 100}),
                      (SDR<>{0, 3, 100}));
}

BOOST_AUTO_TEST_CASE(rm_simple) {
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

struct LoadedType {
  int index;
  int data;
  friend std::ostream& operator<<(std::ostream&, const LoadedType&);
  constexpr bool operator<(const LoadedType& l) const {
    return index < l.index;
  }
  constexpr bool operator==(const LoadedType& l) const {
    return index == l.index;
  }
  constexpr bool operator==(decltype(index) l) const { return index == l; }
  constexpr bool operator<(decltype(index) l) const { return index < l; }

  constexpr operator decltype(index)() const { return index; }

  constexpr LoadedType(decltype(index) index, int data)
      : index(index), data(data) {}
  constexpr LoadedType(decltype(index) index) : index(index), data(0) {}
  constexpr LoadedType() : index(0), data(0) {}
};

std::ostream& operator<<(std::ostream& os, const LoadedType& l) {
  os << l.index;
  return os;
}

BOOST_AUTO_TEST_CASE(test_loaded_types) {
  SDR<LoadedType, std::set<LoadedType>> loaded;
  loaded.push_back(LoadedType{0, 2});
  loaded.push_back(LoadedType{1, 1});
  loaded.push_back(LoadedType{2, 0});
  SDR<decltype(LoadedType::index)> selection{0};

  SDR<LoadedType, std::set<LoadedType>> and_result = loaded.andb(selection);
  BOOST_REQUIRE_EQUAL(and_result.cbegin()->data, 2);

  SDR<LoadedType, std::set<LoadedType>> rm_result = loaded.rmb(selection);
  BOOST_REQUIRE_EQUAL(rm_result.cbegin()->data, 1);
  BOOST_REQUIRE_EQUAL((++rm_result.cbegin())->data, 0);

  SDR<LoadedType, std::set<LoadedType>> rmi_result = loaded.rmi(selection);
  BOOST_REQUIRE_EQUAL(rmi_result.cbegin()->data, 1);
  BOOST_REQUIRE_EQUAL((++rmi_result.cbegin())->data, 0);
}

BOOST_AUTO_TEST_CASE(test_comparison) {
  BOOST_REQUIRE_EQUAL((SDR<>{1, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_NE((SDR<>{0, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_LT((SDR<>{0, 2, 3}), (SDR<>{1, 2, 3}));
  BOOST_REQUIRE_GT((SDR<>{4}), (SDR<>{0, 1, 2}));
}

BOOST_AUTO_TEST_CASE(test_different_container) {
  BOOST_REQUIRE_EQUAL((SDR<int, std::set<int>>{1, 2, 3} & SDR<int, std::vector<int>>{2, 3, 4}), (SDR<int, std::set<int>>{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<int, std::vector<int>>{1, 2, 3} & SDR<int, std::set<int>>{2, 3, 4}), (SDR<int, std::vector<int>>{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<int, std::set<int>>{1, 2, 3} & SDR<int, std::list<int>>{2, 3, 4}), (SDR<int, std::set<int>>{2, 3}));
}

BOOST_AUTO_TEST_SUITE_END()
