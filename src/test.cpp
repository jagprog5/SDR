#define BOOST_TEST_MODULE sparse_distribued_representation_test_module
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
#include "SparseDistributedRepresentation/SDR.hpp"
#include "SparseDistributedRepresentation/IDContiguousContainer.hpp"
#include "SparseDistributedRepresentation/DataTypes/ArithData.hpp"
#include "SparseDistributedRepresentation/DataTypes/UnitData.hpp"
#include <random>
#include <unistd.h>
#include <alloca.h>
#include <forward_list>
#include <set>
using namespace sparse_distributed_representation;

BOOST_AUTO_TEST_SUITE(sdr)

BOOST_AUTO_TEST_CASE(copy_assignment_ctor) {
  SDR<SDRElem<>, std::forward_list<SDRElem<>>> a{1, 2, 3};
  SDR<SDRElem<>, std::forward_list<SDRElem<>>> b;
  b = a;
  BOOST_REQUIRE_EQUAL(b, a);
  BOOST_REQUIRE_EQUAL(b.size(), a.size());
}

BOOST_AUTO_TEST_CASE(move_ctor) {
  SDR<SDRElem<>, std::forward_list<SDRElem<>>> a{1, 2, 3};
  auto b(std::move(a));
  BOOST_REQUIRE_EQUAL(b.size(), 3);
  BOOST_REQUIRE_EQUAL(b, (SDR{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(move_assign_ctor) {
  SDR<SDRElem<>, std::forward_list<SDRElem<>>> a{1, 2, 3};
  SDR<SDRElem<>, std::forward_list<SDRElem<>>> b;
  b = std::move(a);
  BOOST_REQUIRE_EQUAL(b.size(), 3);
  BOOST_REQUIRE_EQUAL(b, (SDR{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(iter_ctor) {
  std::vector<SDRElem<>> v{SDRElem<>(1), SDRElem<>(2), SDRElem<>(3)};
  BOOST_REQUIRE_EQUAL(SDR(v.begin(), v.end()), (SDR{1, 2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>(v.begin(), v.end())), (SDR{1, 2, 3}));
  auto a = SDR<SDRElem<>, std::forward_list<SDRElem<>>>(v.begin(), v.end());
  BOOST_REQUIRE_EQUAL(a.size(), 3);
  BOOST_REQUIRE_EQUAL(a, (SDR{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(init_list_ctor) {
  auto a = SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>{SDRElem<int, UnitData>(1, 0.5), SDRElem<int, UnitData>(1, 0.0)};
  BOOST_REQUIRE_EQUAL(a.size(), 1);
  BOOST_REQUIRE_EQUAL(a, SDR{1});
}

BOOST_AUTO_TEST_CASE(test_encode) {
  BOOST_REQUIRE_EQUAL(SDR(0, 3, 100), (SDR{0, 1, 2}));
  BOOST_REQUIRE_EQUAL(SDR(0.5, 3, 100), (SDR{49, 50, 51}));
  BOOST_REQUIRE_EQUAL(SDR(1, 3, 100), (SDR{97, 98, 99}));

  BOOST_REQUIRE_EQUAL(SDR(0.8, 1, 3, 10), (SDR{0, 8, 9}));
  BOOST_REQUIRE_EQUAL(SDR(0, 1, 3, 10), (SDR{0, 1, 2}));

  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>(0, 3, 100)), (SDR{0, 1, 2}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>(0.5, 3, 100)), (SDR{49, 50, 51}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>(1, 3, 100)), (SDR{97, 98, 99}));

  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>(0.8, 1, 3, 10)), (SDR<>{0, 8, 9}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>(0, 1, 3, 10)), (SDR{0, 1, 2}));

  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>(0, 3, 100)), (SDR{0, 1, 2}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>(0.5, 3, 100)), (SDR{49, 50, 51}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>(1, 3, 100)), (SDR{97, 98, 99}));

  auto b0 = SDR<SDRElem<>, std::forward_list<SDRElem<>>>(0.8, 1, 3, 10);
  BOOST_REQUIRE_EQUAL(b0, (SDR<>{0, 8, 9}));
  BOOST_REQUIRE_EQUAL(b0.size(), 3);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>(0, 1, 3, 10)), (SDR<>{0, 1, 2}));
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
  
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ande(2, 7)), (SDR{2, 3, 5}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ands(2, 7)), 3);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ande(2, 5)), (SDR{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ands(2, 5)), 2);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ande(20, 70)), (SDR{20}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ands(20, 70)), 1);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ande(0, 0)), (SDR{}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3, 5, 20}.ands(0, 0)), 0);

  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ande(2, 7)), (SDR{2, 3, 5}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ands(2, 7)), 3);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ande(2, 5)), (SDR{2, 3}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ands(2, 5)), 2);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ande(20, 70)), (SDR{20}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ands(20, 70)), 1);
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ande(0, 0)), (SDR{}));
  BOOST_REQUIRE_EQUAL((SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3, 5, 20}.ands(0, 0)), 0);
}

BOOST_AUTO_TEST_CASE(andop_single) {
  SDR a{1, 2, 3};
  BOOST_REQUIRE_EQUAL(a & 4, nullptr);
  BOOST_REQUIRE_EQUAL(a & 0, nullptr);
  BOOST_REQUIRE_NE(a & 2, nullptr);
}

BOOST_AUTO_TEST_CASE(shift) {
  SDR a{1, 2, 3};
  a.shift(2);
  BOOST_REQUIRE_EQUAL(a, (SDR{3, 4, 5}));
}

BOOST_AUTO_TEST_CASE(append) {
  SDR a0{1, 2, 3};
  SDR b0{4, 5, 6};
  a0.append(std::move(b0));
  BOOST_REQUIRE_EQUAL(a0, (SDR{1, 2, 3, 4, 5, 6}));

  SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> a1{1, 2, 3};
  SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> b1{4, 5, 6};
  a1.append(std::move(b1));
  BOOST_REQUIRE_EQUAL(a1, (SDR{1, 2, 3, 4, 5, 6}));
}

BOOST_AUTO_TEST_CASE(sample) {
  // this seed happens to fully cover sample
  std::mt19937 twister(3334);

  SDR a{1, 2, 3};
  a.sample(0.8, twister);
  BOOST_REQUIRE(a.size() <= 3);
  auto ita = a.cbegin();
  while (ita != a.cend()) {
    BOOST_REQUIRE(*ita > 0 && *ita < 4);
    ++ita;
  }

  SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> b{1, 2, 3};
  b.sample(0.8, twister);
  BOOST_REQUIRE(b.size() <= 3);
  auto itb = b.cbegin();

  while (itb != b.cend()) {
    BOOST_REQUIRE(*itb > 0 && *itb < 4);
    ++itb;
  }

  SDR<SDRElem<>, std::forward_list<SDRElem<>>> c{1, 2, 3};
  c.sample(0.8, twister);
  BOOST_REQUIRE(c.size() <= 3);
  auto itc = c.cbegin();

  while (itc != c.cend()) {
    BOOST_REQUIRE(*itc > 0 && *itc < 4);
    ++itc;
  }
}

BOOST_AUTO_TEST_CASE(test_shift) {
  BOOST_REQUIRE_EQUAL((SDR{1, 2, 3} << 1), (SDR{2, 3, 4}));
}

BOOST_AUTO_TEST_CASE(test_readme_visitor) {
  SDR a{1, 2, 3};
  SDR b{2, 3, 4};

  int result = 0;
  auto increment_visitor = [&result](typename decltype(a)::iterator,
                                     typename decltype(b)::iterator) {
    ++result;
  };

  a.andv(b, increment_visitor);
  BOOST_REQUIRE_EQUAL(result, 2);
}

BOOST_AUTO_TEST_CASE(test_readme_container) {
  SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> a{1, 2, 3};
  SDR<SDRElem<>, std::forward_list<SDRElem<>>> b{4, 5, 6};
  auto result = a.ore<SDRElem<>, std::list<SDRElem<>>>(b);
  BOOST_REQUIRE_EQUAL(result, (SDR{1, 2, 3, 4, 5, 6}));
}


BOOST_AUTO_TEST_CASE(test_readme_walled_garden) {
  SDR a{1, 2, 3};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto brute_force_ptr = reinterpret_cast<std::vector<SDRElem<>>*>(&a);
  const_cast<int&>((*brute_force_ptr)[1].id()) = 17;
  BOOST_REQUIRE_EQUAL((a.begin() + 0)->id(), 1);
  BOOST_REQUIRE_EQUAL((a.begin() + 1)->id(), 17);
  BOOST_REQUIRE_EQUAL((a.begin() + 2)->id(), 3);
}

BOOST_AUTO_TEST_CASE(test_ret_type) {
  {
    SDR a {1, 2, 3};
    SDR b {2, 3, 4};
    auto r_and = a.ande<SDRElem<long>, std::forward_list<SDRElem<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_and, a.ande(b));
    auto r_or = a.ore<SDRElem<long>, std::forward_list<SDRElem<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_or, a.ore(b));
    auto r_xor = a.xore<SDRElem<long>, std::forward_list<SDRElem<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_xor, a.xore(b));
    auto r_rm = a.rme<SDRElem<long>, std::forward_list<SDRElem<long>>>(b);
    BOOST_REQUIRE_EQUAL(r_rm, a.rme(b));
  }
  {
    SDR a {1, 2, 3};
    SDR b {2, 3, 4};
    auto r_and = a.ande<SDRElem<long>, std::set<SDRElem<long>, std::less<>>>(b);
    BOOST_REQUIRE_EQUAL(r_and, a.ande(b));
    auto r_or = a.ore<SDRElem<long>, std::set<SDRElem<long>, std::less<>>>(b);
    BOOST_REQUIRE_EQUAL(r_or, a.ore(b));
    auto r_xor = a.xore<SDRElem<long>, std::set<SDRElem<long>, std::less<>>>(b);
    BOOST_REQUIRE_EQUAL(r_xor, a.xore(b));
    auto r_rm = a.rme<SDRElem<long>, std::set<SDRElem<long>, std::less<>>>(b);
    BOOST_REQUIRE_EQUAL(r_rm, a.rme(b));
  }
}

BOOST_AUTO_TEST_CASE(test_float_data) {
  SDR<SDRElem<int, ArithData<>>>a{SDRElem<int, ArithData<>>(0, 3), SDRElem<int, ArithData<>>(1, 2), SDRElem<int, ArithData<>>(2, 1)};
  SDR<SDRElem<int, ArithData<>>>b{SDRElem<int, ArithData<>>(0, 2), SDRElem<int, ArithData<>>(1, 2), SDRElem<int, ArithData<>>(2, 2)};
  auto result = a - b;
  float val = 1;
  for (auto& elem : result) {
    BOOST_REQUIRE_EQUAL(elem.data().value(), val--);
  }
}

BOOST_AUTO_TEST_CASE(test_aliasing) {
  {
    SDR a{1, 2, 3};
    BOOST_REQUIRE_EQUAL(a.rmi(a), decltype(a)());
    SDR b{1, 2, 3};
    BOOST_REQUIRE_EQUAL(b.xori(b), decltype(b)());
    SDR c{1, 2, 3};
    BOOST_REQUIRE_EQUAL(c.ori(c), (SDR{1, 2, 3}));
    SDR d{1, 2, 3};
    BOOST_REQUIRE_EQUAL(d.andi(d), (SDR{1, 2, 3}));
  }
  {
    SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> a{1, 2, 3};
    BOOST_REQUIRE_EQUAL(a.rmi(a), decltype(a)());
    SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> b{1, 2, 3};
    BOOST_REQUIRE_EQUAL(b.xori(b), decltype(b)());
    SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> c{1, 2, 3};
    BOOST_REQUIRE_EQUAL(c.ori(c), (SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3}));
    SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>> d{1, 2, 3};
    BOOST_REQUIRE_EQUAL(d.andi(d), (SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>{1, 2, 3}));
  }
  {
    SDR<SDRElem<>, std::forward_list<SDRElem<>>> a{1, 2, 3};
    BOOST_REQUIRE_EQUAL(a.rmi(a), decltype(a)());
    SDR<SDRElem<>, std::forward_list<SDRElem<>>> b{1, 2, 3};
    BOOST_REQUIRE_EQUAL(b.xori(b), decltype(b)());
    SDR<SDRElem<>, std::forward_list<SDRElem<>>> c{1, 2, 3};
    BOOST_REQUIRE_EQUAL(c.ori(c), (SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3}));
    SDR<SDRElem<>, std::forward_list<SDRElem<>>> d{1, 2, 3};
    BOOST_REQUIRE_EQUAL(d.andi(d), (SDR<SDRElem<>, std::forward_list<SDRElem<>>>{1, 2, 3}));
  }
}

BOOST_AUTO_TEST_CASE(test_set_merge_specialization) {
  using T = SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>;
  T a{1, 2, 3};
  T b{3, 4, 5};
  a.ori(std::move(b));
  BOOST_REQUIRE_EQUAL(a, (T{1, 2, 3, 4, 5}));
}

BOOST_AUTO_TEST_CASE(test_printing) {
  boost::test_tools::output_test_stream output;
  output << ArithData<>(0.5555);
  BOOST_REQUIRE(output.is_equal("0.5555"));
  output << UnitData();
  BOOST_REQUIRE(output.is_equal("1.0"));
  output << UnitData(0.5555);
  BOOST_REQUIRE(output.is_equal(".55"));

  struct Breaker : UnitData {
    float value() const { return 1.1f; }
  };
  output << (UnitData().andi(Breaker()));
  BOOST_REQUIRE(output.is_equal("!!!"));

  output << SDR{1, 2, 3};
  BOOST_REQUIRE(output.is_equal("[1,2,3]"));

  output << SDR<SDRElem<int, ArithData<>>>{SDRElem<int, ArithData<>>{1, 5},
    SDRElem<int, ArithData<>>{2, 6},
    SDRElem<int, ArithData<>>{3, 7}};
  BOOST_REQUIRE(output.is_equal("[1(5),2(6),3(7)]"));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int copy_count;
struct CopyDetector {
  CopyDetector() {}
  CopyDetector(const CopyDetector&) {
    ++copy_count;
  }
  CopyDetector& operator=(const CopyDetector&) {
    ++copy_count;
    return *this;
  }

  CopyDetector(CopyDetector&&) noexcept {}

  CopyDetector& operator=(CopyDetector&&) noexcept {
    return *this;
  }

  template<typename T>
  CopyDetector ore(const CopyDetector&) const {
    return CopyDetector();
  }

  template<typename T>
  CopyDetector xore(const CopyDetector&) const {
    return CopyDetector();
  }

  bool operator!=(const CopyDetector&) const {
    return false;
  }

  bool rm_relevant() const {
      return false;
  }
};

inline std::ostream& operator<<(std::ostream& os, const CopyDetector&) {
    os << "COPYDETECTOR";
    return os;
}

BOOST_AUTO_TEST_CASE(test_move) {
  // ensure that the rvalue overloads actually move the data
  // (there's a lot going on there, so its best to check)
  using E = SDRElem<int, CopyDetector>;

  // or rval overloads
  {
    SDR<E> a{1, 3};
    SDR<E> b{2, 3};
    copy_count = 0;
    auto r = a.ore(std::move(b));

    // a single copy occurs when copying "1" from a into the result
    // the "2" is moved (as it should)
    // and the "3" in a and b being combined does not count as a copy
    BOOST_REQUIRE_EQUAL(copy_count, 1);
    BOOST_REQUIRE_EQUAL(r, (SDR<E>{1, 2, 3}));
  }
  {
    SDR<E> a{1, 3};
    SDR<E> b{2, 3};
    copy_count = 0;
    a.ori(std::move(b));

    BOOST_REQUIRE_EQUAL(copy_count, 0);
    BOOST_REQUIRE_EQUAL(a, (SDR<E>{1, 2, 3}));
  }

  // xor rval overloads
  {
    SDR<E> a{1, 3};
    SDR<E> b{2, 3};
    copy_count = 0;
    auto r = a.xore(std::move(b));

    BOOST_REQUIRE_EQUAL(copy_count, 1);
    BOOST_REQUIRE_EQUAL(r, (SDR<E>{1, 2}));
  }
  {
    SDR<E> a{1, 3};
    SDR<E> b{2, 3};
    copy_count = 0;
    a.xori(std::move(b));

    BOOST_REQUIRE_EQUAL(copy_count, 0);
    BOOST_REQUIRE_EQUAL(a, (SDR<E>{1, 2}));
  }
}

BOOST_AUTO_TEST_CASE(testIDContiguousMove) {
  using E = SDRElem<int, CopyDetector>;
  using SDRIDC = SDR<E, IDContiguousContainer<E>>;
  // exact same as test above, but with different types
  {
    SDRIDC a{1, 3};
    SDRIDC b{2, 3};
    copy_count = 0;
    auto r = a.ore(std::move(b));
    BOOST_REQUIRE_EQUAL(copy_count, 1);
    BOOST_REQUIRE_EQUAL(r, (SDRIDC{1, 2, 3}));
  }
  {
    SDRIDC a{1, 3};
    SDRIDC b{2, 3};
    copy_count = 0;
    a.ori(std::move(b));
    BOOST_REQUIRE_EQUAL(copy_count, 0);
    BOOST_REQUIRE_EQUAL(a, (SDRIDC{1, 2, 3}));
  }
  {
    SDRIDC a{1, 3};
    SDRIDC b{2, 3};
    copy_count = 0;
    auto r = a.xore(std::move(b));
    BOOST_REQUIRE_EQUAL(copy_count, 1);
    BOOST_REQUIRE_EQUAL(r, (SDRIDC{1, 2}));
  }
  {
    SDRIDC a{1, 3};
    SDRIDC b{2, 3};
    copy_count = 0;
    a.xori(std::move(b));
    BOOST_REQUIRE_EQUAL(copy_count, 0);
    BOOST_REQUIRE_EQUAL(a, (SDRIDC{1, 2}));
  }
}

BOOST_AUTO_TEST_CASE(test_inner) {
  using Elem = SDRElem<int, ArithData<>>;
  SDR<Elem> a{Elem(0, 0), Elem(1, 1), Elem(2, 2)};
  SDR<Elem> b{Elem(0, 0), Elem(1, 2), Elem(2, 4)};
  ArithData<> data = a.inner(b);
  BOOST_REQUIRE_EQUAL(data.value(), 10);
}

BOOST_AUTO_TEST_CASE(test_outer) {
  using Elem = SDRElem<int, ArithData<>>;
  SDR<Elem> a{Elem(0, 0), Elem(1, 1)};
  SDR<Elem> b{Elem(0, 2), Elem(1, 3)};
  auto data = a.outer(b);
  auto r = SDR<SDRElem<int, SDR<Elem>>>{
    SDRElem<int, SDR<Elem>>(0, SDR<Elem>{Elem(0, 0), Elem(1, 0)}),
    SDRElem<int, SDR<Elem>>(1, SDR<Elem>{Elem(0, 2), Elem(1, 3)})
  };
  BOOST_REQUIRE_EQUAL(data, r);
}

using Element = SDRElem<unsigned int, ArithData<>>;
using Row = SDRElem<unsigned int, SDR<Element>>;
using Column = Row;
using Matrix = SDR<Row>;

BOOST_AUTO_TEST_CASE(row_matrix_vector_multiply) {
  //  1 2   10   32
  //  3 4 * 11 = 74
  {
    Row row0(0, SDR<Element>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element>{Element(0, 3.0f), Element(1, 4.0f)});
    Matrix m{row0, row1};
    {
      auto input = SDR<Element>{Element(0, 10.0f), Element(1, 11.0f)};
      auto result = m.row_major_mul_vec(input); 
      BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 32.0f), Element(1, 74.0f)}));  
    }
    {
      auto input = SDR<Element, std::set<Element, std::less<>>>{Element(0, 10.0f), Element(1, 11.0f)};
      auto result = m.row_major_mul_vec(input); 
      BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 32.0f), Element(1, 74.0f)}));  
    }
    {
      auto input = SDR<Element, std::forward_list<Element>>{Element(0, 10.0f), Element(1, 11.0f)};
      auto result = m.row_major_mul_vec(input); 
      BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 32.0f), Element(1, 74.0f)}));  
    }
  }
  {
    Column column0(0, SDR<Element>{Element(0, 1.0f), Element(1, 3.0f)});
    Column column1(1, SDR<Element>{Element(0, 2.0f), Element(1, 4.0f)});
    Matrix m{column0, column1};
    {
      auto input = SDR<Element>{Element(0, 10.0f), Element(1, 11.0f)};
      auto result = m.col_major_mul_vec(input); 
      BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 32.0f), Element(1, 74.0f)}));  
    }
    {
      auto input = SDR<Element, std::set<Element, std::less<>>>{Element(0, 10.0f), Element(1, 11.0f)};
      auto result = m.col_major_mul_vec(input); 
      BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 32.0f), Element(1, 74.0f)}));  
    }
    {
      auto input = SDR<Element, std::forward_list<Element>>{Element(0, 10.0f), Element(1, 11.0f)};
      auto result = m.col_major_mul_vec(input); 
      BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 32.0f), Element(1, 74.0f)}));  
    }
  }
  // some other weird cases
  {
    BOOST_REQUIRE_EQUAL(SDR<Element>(), (Matrix().row_major_mul_vec(SDR<Element>())));
  }
  {
    auto input = SDR<Element>{Element(0, 10.0f), Element(1, 11.0f)};
    Column column0(0, SDR<Element>{Element(1, 3.0f)});
    Column column1(1, SDR<Element>{Element(0, 2.0f)});
    SDR<Column, std::set<Column, std::less<>>> m{column0, column1};
    auto result = m.col_major_mul_vec(input); 
    BOOST_REQUIRE_EQUAL(result, (SDR<Element>{Element(0, 22.0f), Element(1, 30.0f)}));  
  }
}

BOOST_AUTO_TEST_CASE(matrix_transpose) {
  //  1 2    1 3
  //  3 4 -> 2 4
  {
    Row row0(0, SDR<Element>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element>{Element(0, 3.0f), Element(1, 4.0f)});
    Matrix m{row0, row1};
    Row row2(0, SDR<Element>{Element(0, 1.0f), Element(1, 3.0f)});
    Row row3(1, SDR<Element>{Element(0, 2.0f), Element(1, 4.0f)});
    Matrix result{row2, row3};
    BOOST_REQUIRE_EQUAL(m.transpose(), result);
  }
  {
    Row row0(0, SDR<Element>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element>{Element(0, 3.0f), Element(1, 4.0f)});
    SDR<Row, std::set<Row, std::less<>>> m{row0, row1};
    Row row2(0, SDR<Element>{Element(0, 1.0f), Element(1, 3.0f)});
    Row row3(1, SDR<Element>{Element(0, 2.0f), Element(1, 4.0f)});
    SDR<Row, std::set<Row, std::less<>>> result{row2, row3};
    BOOST_REQUIRE_EQUAL(m.transpose(), result);
  }
  {
    Row row0(0, SDR<Element>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element>{Element(0, 3.0f), Element(1, 4.0f)});
    SDR<Row, std::forward_list<Row>> m{row0, row1};
    Row row2(0, SDR<Element>{Element(0, 1.0f), Element(1, 3.0f)});
    Row row3(1, SDR<Element>{Element(0, 2.0f), Element(1, 4.0f)});
    SDR<Row, std::forward_list<Row>> result{row2, row3};
    BOOST_REQUIRE_EQUAL(m.transpose(), result);
  }
  {
    Matrix m;
    BOOST_REQUIRE_EQUAL(m.transpose(), Matrix());
  }
  {
    using Row = SDRElem<unsigned int, SDR<Element, std::forward_list<Element>>>;
    Row row0(0, SDR<Element, std::forward_list<Element>>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element, std::forward_list<Element>>{Element(0, 3.0f), Element(1, 4.0f)});
    SDR<Row> m{row0, row1};
    Row row2(0, SDR<Element, std::forward_list<Element>>{Element(0, 1.0f), Element(1, 3.0f)});
    Row row3(1, SDR<Element, std::forward_list<Element>>{Element(0, 2.0f), Element(1, 4.0f)});
    SDR<Row> result{row2, row3};
    BOOST_REQUIRE_EQUAL(m.transpose(), result);
  }
  {
    using Row = SDRElem<unsigned int, SDR<Element, std::set<Element, std::less<>>>>;
    Row row0(0, SDR<Element, std::set<Element, std::less<>>>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element, std::set<Element, std::less<>>>{Element(0, 3.0f), Element(1, 4.0f)});
    SDR<Row> m{row0, row1};
    Row row2(0, SDR<Element, std::set<Element, std::less<>>>{Element(0, 1.0f), Element(1, 3.0f)});
    Row row3(1, SDR<Element, std::set<Element, std::less<>>>{Element(0, 2.0f), Element(1, 4.0f)});
    SDR<Row> result{row2, row3};
    BOOST_REQUIRE_EQUAL(m.transpose(), result);
  }
}

BOOST_AUTO_TEST_CASE(transpose2) {
  using A_inner_elem = SDRElem<double>;
  using A_inner = SDR<A_inner_elem>;
  using A_outer_elem = SDRElem<float, A_inner>;
  using A_outer = SDR<A_outer_elem, std::forward_list<A_outer_elem>>;
  using B_inner_elem = SDRElem<float>;
  using B_inner = SDR<B_inner_elem, std::forward_list<B_inner_elem>>;
  using B_outer_elem = SDRElem<double, B_inner>;
  using B_outer = SDR<B_outer_elem>;
  static_assert(std::is_same_v<A_outer, decltype(B_outer().transpose2())>);
}

BOOST_AUTO_TEST_CASE(matrix_trace_and_sum) {
  Row row0(0, SDR<Element>{Element(0, 1.0f), Element(1, 2.0f)});
  Row row1(1, SDR<Element>{Element(0, 3.0f), Element(1, 4.0f)});
  Matrix m{row0, row1};
  BOOST_REQUIRE_EQUAL(m.trace(), 5);
  BOOST_REQUIRE_EQUAL(Matrix().trace(), 0);
  auto sum = m.sum();
  BOOST_REQUIRE_EQUAL(sum, 10);
  static_assert(std::is_same_v<decltype(sum), ArithData<>>);
}

BOOST_AUTO_TEST_CASE(matrix_matrix_multiply) {
  //  [1 2]   [5 6]   19 22
  //  [3 4] * [7 8] = 43 50
  {
    Row row0(0, SDR<Element>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element>{Element(0, 3.0f), Element(1, 4.0f)});
    Matrix m0{row0, row1};

    Row row2(0, SDR<Element>{Element(0, 19.0f), Element(1, 22.0f)});
    Row row3(1, SDR<Element>{Element(0, 43.0f), Element(1, 50.0f)});
    Matrix result{row2, row3};
    {
      Row row4(0, SDR<Element>{Element(0, 5.0f), Element(1, 6.0f)});
      Row row5(1, SDR<Element>{Element(0, 7.0f), Element(1, 8.0f)});
      {
        Matrix m1{row4, row5};
        BOOST_REQUIRE_EQUAL(m0.same_mul(m1), result);
      }
      {
        SDR<Row, std::set<Row, std::less<>>> m1{row4, row5};
        BOOST_REQUIRE_EQUAL(m0.same_mul(m1), result);
      }
      {
        SDR<Row, std::forward_list<Row>> m1{row4, row5};
        BOOST_REQUIRE_EQUAL(m0.same_mul(m1), result);
      }
    }
    {
      Column col0(0, SDR<Element>{Element(0, 5.0f), Element(1, 7.0f)});
      Column col1(1, SDR<Element>{Element(0, 6.0f), Element(1, 8.0f)});
      Matrix m1{col0, col1};
      BOOST_REQUIRE_EQUAL(m0.diff_mul(m1), result);
    }
    {
      using Column = SDRElem<unsigned int, SDR<Element, std::forward_list<Element>>>;
      Column col0(0, SDR<Element, std::forward_list<Element>>{Element(0, 5.0f), Element(1, 7.0f)});
      Column col1(1, SDR<Element, std::forward_list<Element>>{Element(0, 6.0f), Element(1, 8.0f)});
      SDR<Column, std::forward_list<Column>> m1{col0, col1};
      BOOST_REQUIRE_EQUAL(m0.diff_mul(m1), result);
    }
  }
  {
    BOOST_REQUIRE_EQUAL(Matrix().diff_mul(Matrix()), Matrix());
  }
  {
    using Element = SDRElem<unsigned int, ArithData<>>;
    using Row = SDRElem<unsigned int, SDR<Element, std::set<Element, std::less<>>>>;
    using Matrix = SDR<Row, std::set<Row, std::less<>>>;
    Row row0(0, SDR<Element, std::set<Element, std::less<>>>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element, std::set<Element, std::less<>>>{Element(0, 3.0f), Element(1, 4.0f)});
    Matrix m0{row0, row1};

    Row row2(0, SDR<Element, std::set<Element, std::less<>>>{Element(0, 19.0f), Element(1, 22.0f)});
    Row row3(1, SDR<Element, std::set<Element, std::less<>>>{Element(0, 43.0f), Element(1, 50.0f)});
    Matrix result{row2, row3};

    Row row4(0, SDR<Element, std::set<Element, std::less<>>>{Element(0, 5.0f), Element(1, 6.0f)});
    Row row5(1, SDR<Element, std::set<Element, std::less<>>>{Element(0, 7.0f), Element(1, 8.0f)});
    Matrix m1{row4, row5};

    BOOST_REQUIRE_EQUAL(m0.same_mul(m1), result);
  }
  {
    using Element = SDRElem<unsigned int, ArithData<>>;
    using Row = SDRElem<unsigned int, SDR<Element, std::forward_list<Element>>>;
    using Matrix = SDR<Row, std::forward_list<Row>>;
    Row row0(0, SDR<Element, std::forward_list<Element>>{Element(0, 1.0f), Element(1, 2.0f)});
    Row row1(1, SDR<Element, std::forward_list<Element>>{Element(0, 3.0f), Element(1, 4.0f)});
    Matrix m0{row0, row1};

    Row row2(0, SDR<Element, std::forward_list<Element>>{Element(0, 19.0f), Element(1, 22.0f)});
    Row row3(1, SDR<Element, std::forward_list<Element>>{Element(0, 43.0f), Element(1, 50.0f)});
    Matrix result{row2, row3};

    Row row4(0, SDR<Element, std::forward_list<Element>>{Element(0, 5.0f), Element(1, 6.0f)});
    Row row5(1, SDR<Element, std::forward_list<Element>>{Element(0, 7.0f), Element(1, 8.0f)});
    Matrix m1{row4, row5};

    BOOST_REQUIRE_EQUAL(m0.same_mul(m1), result);
  }
}

BOOST_AUTO_TEST_SUITE_END()
