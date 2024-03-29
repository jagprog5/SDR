#include "SparseDistributedRepresentation/SDR.hpp"
#include "SparseDistributedRepresentation/DataTypes/UnitData.hpp"
#include "SparseDistributedRepresentation/DataTypes/ArithData.hpp"
#include "SparseDistributedRepresentation/IDContiguousContainer.hpp"
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <random>
#include <forward_list>
#include <set>

using namespace sparse_distributed_representation;
using namespace std::chrono;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REQUIRE_TRUE(x) if (!(x)) return false;

// only do a speed test, don't check for correctness
static constexpr bool disable_validation = false;

template<typename SDRElem_t, typename container_t, typename arg_t, typename c_arg_t>
bool validate_andop(const SDR<SDRElem_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, const SDR<SDRElem_t, container_t>& r) {
    // for every element in a, if it is also in b, then it must be in the result
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        auto a_elem = *a_pos;
        auto b_pos = std::find(b.cbegin(), b.cend(), a_elem);
        if (b_pos != b.cend()) {
            auto data = a_elem.data().ande(b_pos->data());
            // check data correct in result
            auto pos = r & a_elem.id();
            if (data.relevant()) {
                if (pos == nullptr) {
                    REQUIRE_TRUE(false);
                } else {
                    REQUIRE_TRUE(*pos == data);
                }
            } else {
                REQUIRE_TRUE(pos == nullptr);
            }
        }
    }
    // for every element in b, if it is also in a, then it must be in the result
    for(auto b_pos = b.cbegin(); b_pos != b.cend(); ++b_pos) {
        auto b_elem = *b_pos;
        auto a_pos = std::find(a.cbegin(), a.cend(), b_elem);
        if (a_pos != a.cend()) {
            // b element is in a
            auto data = a_pos->data().ande(b_elem.data());
            // check data correct in result
            auto pos = r & b_elem.id();
            if (data.relevant()) {
                if (pos == nullptr) {
                    REQUIRE_TRUE(false);
                } else {
                    REQUIRE_TRUE(*pos == data);
                }
            } else {
                REQUIRE_TRUE(pos == nullptr);
            }
        }
    }
    // the result can't contain any elements not in a or not in b
    typename container_t::size_type i = 0;
    for(auto r_pos = r.cbegin(); r_pos != r.cend(); ++r_pos) {
        ++i;
        const auto& r_elem = *r_pos;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem.id()) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem.id()) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    // ensure the size is correct
    REQUIRE_TRUE(i == r.size());
    REQUIRE_TRUE(r.is_ascending());
    return true;
}

template<typename SDRElem_t, typename container_t, typename arg_t, typename c_arg_t>
bool validate_orop(const SDR<SDRElem_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, const SDR<SDRElem_t, container_t>& r) {
    // every element in a must be in the result
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), a_pos->id()) != r.cend());
    }
    // every element in b must be in the result
    for(auto b_pos = b.cbegin(); b_pos != b.cend(); ++b_pos) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), b_pos->id()) != r.cend());
    }
    // the result can't contain any elements not in a or not in b
    typename container_t::size_type i = 0;
    for(auto r_pos = r.cbegin(); r_pos != r.cend(); ++r_pos) {
        ++i;
        auto r_elem = *r_pos;
        auto a_pos = std::find(a.cbegin(), a.cend(), r_elem.id());
        auto b_pos = std::find(b.cbegin(), b.cend(), r_elem.id());
        REQUIRE_TRUE(a_pos != a.cend() || b_pos != b.cend());
        // data correctness
        if (a_pos != a.cend() && b_pos != b.cend()) {
            REQUIRE_TRUE(r_elem.data() == a_pos->data().ore(b_pos->data()));
        } else if (a_pos != a.cend()) {
            REQUIRE_TRUE(r_elem.data() == a_pos->data());
        } else {
            REQUIRE_TRUE(r_elem.data() == b_pos->data());
        }
    }
    // ensure the size is correct
    REQUIRE_TRUE(i == r.size());
    REQUIRE_TRUE(r.is_ascending());
    return true;
}

template<typename SDRElem_t, typename container_t, typename arg_t, typename c_arg_t>
bool validate_xorop(const SDR<SDRElem_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, const SDR<SDRElem_t, container_t>& r) {
    // for every element in a, if it is not in b, then it must be in the result
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        auto a_elem = *a_pos;
        auto b_pos = std::find(b.cbegin(), b.cend(), a_elem.id());
        if (b_pos == b.cend() || a_elem.data().xore(b_pos->data()).rm_relevant()) {
            // a elem is not in b
            REQUIRE_TRUE(r & a_elem.id())
        }
    }
    // for every element in b, if it is not in a, then it must be in the result
    for(auto b_pos = b.cbegin(); b_pos != b.cend(); ++b_pos) {
        auto b_elem = *b_pos;
        auto a_pos = std::find(a.cbegin(), a.cend(), b_elem.id());
        if (a_pos == a.cend() || a_pos->data().xore(b_elem.data()).rm_relevant()) {
            // b elem is not in a
            REQUIRE_TRUE(r & b_elem.id());
        }
    }
    // the result can't contain any elements not in a or not in b
    typename container_t::size_type i = 0;
    for(auto r_pos = r.cbegin(); r_pos != r.cend(); ++r_pos) {
        ++i;
        auto r_elem = *r_pos;
        auto a_pos = std::find(a.cbegin(), a.cend(), r_elem.id());
        auto b_pos = std::find(b.cbegin(), b.cend(), r_elem.id());
        REQUIRE_TRUE(a_pos != a.cend() || b_pos != b.cend());
        // data correctness
        if (a_pos != a.cend() && b_pos != b.cend()) {
            REQUIRE_TRUE(r_elem.data() == a_pos->data().xore(b_pos->data()));
        } else if (a_pos != a.cend()) {
            REQUIRE_TRUE(r_elem.data() == a_pos->data());
        } else {
            REQUIRE_TRUE(r_elem.data() == b_pos->data());
        }
    }
    // ensure the size is correct
    REQUIRE_TRUE(i == r.size());
    REQUIRE_TRUE(r.is_ascending());
    return true;
}

template<typename SDRElem_t, typename container_t, typename arg_t, typename c_arg_t>
bool validate_rmop(const SDR<SDRElem_t, container_t>& a, const SDR<arg_t, c_arg_t>& b, const SDR<SDRElem_t, container_t>& r) {
    // for every elements in a, if it is not in b, then it must be in the result
    typename container_t::size_type i = 0;
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        auto a_elem = *a_pos;
        auto b_pos = std::find(b.cbegin(), b.cend(), a_elem.id());
        typename SDRElem_t::data_type data;
        if (b_pos == b.cend()) {
            data = a_elem.data();
        } else {
            data = a_elem.data().rme(b_pos->data());
        }
        if (b_pos == b.cend() || data.rm_relevant()) {
            // a elem is not in b
            auto pos = r & a_elem.id();
            REQUIRE_TRUE(pos != nullptr);
            REQUIRE_TRUE(*pos == data);
            ++i;
        }
    }
    // the result can't contain any other elements
    REQUIRE_TRUE(i == r.size());
    REQUIRE_TRUE(r.is_ascending());
    return true;
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::mt19937 twister(time(NULL) * getpid());

// generate a unique SDR based on a number
// if the specialization uses UnitData, then generate some random data as well
template<typename SDR>
SDR get_sdr(int val) {
    SDR ret;
    long start;
    if constexpr(SDR::uses_flist_like) {
        start = sizeof(decltype(val)) * 8 - 1;
    } else {
        start = 0;
    }
    long stop; // exclusive
    if constexpr(SDR::uses_flist_like) {
        stop = -1;
    } else {
        stop = sizeof(decltype(val)) * 8;
    }
    long change;
    if constexpr(SDR::uses_flist_like) {
        change = -1;
    } else {
        change = 1;
    }
    for (long i = start; i != stop; i += change) {
        if ((1 << i) & val) {
            typename SDR::value_type::data_type data((float)twister() / (float)twister.max());
            // if (!data.relevant()) continue;
            typename SDR::value_type elem(i, data);
            if constexpr(SDR::uses_flist_like) {
                ret.push_front(elem);
            } else {
                ret.push_back(elem);
            }
        }
    }
    return ret;
}

template<typename SDR>
std::string get_template_name() {
    if constexpr(SDR::uses_vector_like) {
        return "vec";
    } else if constexpr(SDR::uses_set_like) {
        return "set";
    } else if constexpr(SDR::uses_flist_like) {
        return "lst";
    } else {
        return "???";
    }
}

// name: some sort of identifier
// f: the function to time and test. should return false if it failed
template<typename SDRA, typename SDRB, typename funct>
void time_op(const std::string& name, funct f, int fuzz_amount) {
    duration<long, std::nano> duration(0);
    for (int i = 0; i < fuzz_amount; ++i) {
        for (int j = 0; j < fuzz_amount; ++j) {
            SDRA sdra = get_sdr<SDRA>(i);
            SDRB sdrb = get_sdr<SDRB>(j);
            bool result = f(sdra, sdrb, duration);
            if (!result) {
                std::string on_fail = "fail: " + name + " (" + std::to_string(i) + "," + std::to_string(j) + ")";
                std::cout << on_fail << std::endl;
                exit(1);
            }
        }
    }
    std::cout << name << "<" << get_template_name<SDRA>() << ',' << get_template_name<SDRB>() << ">: " << duration.count() / 1000000 << "ms" << std::endl;
}


template<typename SDRA, typename SDRB>
void series(int fuzz_amount) {
    auto ande = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA and_result = a.ande(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_andop(a, b, and_result);
    };
    time_op<SDRA, SDRB>("ande", ande, fuzz_amount);

    auto andi = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.andi(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_andop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>("andi", andi, fuzz_amount);

    auto ands = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.ands(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || s == a.ande(b).size();
    };
    time_op<SDRA, SDRB>("ands", ands, fuzz_amount);

    auto ore = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA or_result = a.ore(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_orop(a, b, or_result);
    };
    time_op<SDRA, SDRB>(" ore", ore, fuzz_amount);

    auto ori = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.ori(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_orop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>(" ori", ori, fuzz_amount);

    auto ors = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.ors(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || s == a.ore(b).size();
    };
    time_op<SDRA, SDRB>(" ors", ors, fuzz_amount);

    if constexpr(has_xor<typename SDRA::value_type::data_type>::value) {
        auto xore = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
            auto start = high_resolution_clock::now();
            SDRA xor_result = a.xore(b);
            auto stop = high_resolution_clock::now();
            total_time += stop - start;
            return disable_validation || validate_xorop(a, b, xor_result);
        };
        time_op<SDRA, SDRB>("xore", xore, fuzz_amount);

        auto xori = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
            SDRA a_cp(a);
            auto start = high_resolution_clock::now();
            a_cp.xori(b);
            auto stop = high_resolution_clock::now();
            total_time += stop - start;
            return disable_validation || validate_xorop(a, b, a_cp);
        };
        time_op<SDRA, SDRB>("xori", xori, fuzz_amount);

        auto xors = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
            auto start = high_resolution_clock::now();
            auto s = a.xors(b);
            auto stop = high_resolution_clock::now();
            total_time += stop - start;
            return disable_validation || s == a.xore(b).size();
        };
        time_op<SDRA, SDRB>("xors", xors, fuzz_amount);
    }

    auto rme = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA rm_result = a.rme(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_rmop(a, b, rm_result);
    };
    time_op<SDRA, SDRB>(" rme", rme, fuzz_amount);

    auto rmi = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.rmi(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_rmop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>(" rmi", rmi, fuzz_amount);

    auto rms = [](const SDRA& a, const SDRB& b, duration<long, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.rms(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return s == a.rme(b).size();
    };
    time_op<SDRA, SDRB>(" rms", rms, fuzz_amount);    
}

int main(int argc, char** argv) {
    int fuzz_amount;
    if (argc > 1) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (std::strcmp(argv[1], "--help") == 0) {
            std::cout << "Usage: fuzz_sdr [<amount>]\n";
            exit(0);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        fuzz_amount = std::atoi(argv[1]);
    } else {
        fuzz_amount = 250; // default
    }

    // yes, this makes a large binary from all the template specializations.
    // but no. realistically, nobody will make nearly this many specializations

    series<SDR<SDRElem<>, std::vector<SDRElem<>>>, SDR<SDRElem<>, std::vector<SDRElem<>>>>(fuzz_amount);
    #ifdef FUZZ_FULL
    series<SDR<SDRElem<>, std::vector<SDRElem<>>>, SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<>, std::vector<SDRElem<>>>, SDR<SDRElem<>, std::forward_list<SDRElem<>>>>(fuzz_amount);
    #endif

    #ifdef FUZZ_FULL
    series<SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>, SDR<SDRElem<>, std::vector<SDRElem<>>>>(fuzz_amount);
    #endif
    series<SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>, SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>>(fuzz_amount);
    #ifdef FUZZ_FULL
    series<SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>, SDR<SDRElem<>, std::forward_list<SDRElem<>>>>(fuzz_amount);
    #endif

    #ifdef FUZZ_FULL
    series<SDR<SDRElem<>, std::forward_list<SDRElem<>>>, SDR<SDRElem<>, std::vector<SDRElem<>>>>(fuzz_amount);
    series<SDR<SDRElem<>, std::forward_list<SDRElem<>>>, SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>>(fuzz_amount);
    #endif
    series<SDR<SDRElem<>, std::forward_list<SDRElem<>>>, SDR<SDRElem<>, std::forward_list<SDRElem<>>>>(fuzz_amount);

    #ifdef FUZZ_FULL

    std::cout << "======With data elements======" << std::endl;

    series<SDR<SDRElem<long, UnitData>, std::vector<SDRElem<long, UnitData>>>, SDR<SDRElem<int, UnitData>, std::vector<SDRElem<int, UnitData>>>>(fuzz_amount);
    series<SDR<SDRElem<long, UnitData>, std::vector<SDRElem<long, UnitData>>>, SDR<SDRElem<int, UnitData>, std::set<SDRElem<int, UnitData>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<long, UnitData>, std::vector<SDRElem<long, UnitData>>>, SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>>(fuzz_amount);

    series<SDR<SDRElem<long, UnitData>, std::set<SDRElem<long, UnitData>, std::less<>>>, SDR<SDRElem<int, UnitData>, std::vector<SDRElem<int, UnitData>>>>(fuzz_amount);
    series<SDR<SDRElem<long, UnitData>, std::set<SDRElem<long, UnitData>, std::less<>>>, SDR<SDRElem<int, UnitData>, std::set<SDRElem<int, UnitData>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<long, UnitData>, std::set<SDRElem<long, UnitData>, std::less<>>>, SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>>(fuzz_amount);

    series<SDR<SDRElem<long, UnitData>, std::forward_list<SDRElem<long, UnitData>>>, SDR<SDRElem<int, UnitData>, std::vector<SDRElem<int, UnitData>>>>(fuzz_amount);
    series<SDR<SDRElem<long, UnitData>, std::forward_list<SDRElem<long, UnitData>>>, SDR<SDRElem<int, UnitData>, std::set<SDRElem<int, UnitData>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<long, UnitData>, std::forward_list<SDRElem<long, UnitData>>>, SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>>(fuzz_amount);

    std::cout << "======Mixed with and without data======" << std::endl;

    series<SDR<SDRElem<int, UnitData>, std::vector<SDRElem<int, UnitData>>>, SDR<SDRElem<>, std::vector<SDRElem<>>>>(fuzz_amount);
    series<SDR<SDRElem<int, UnitData>, std::vector<SDRElem<int, UnitData>>>, SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<int, UnitData>, std::vector<SDRElem<int, UnitData>>>, SDR<SDRElem<>, std::forward_list<SDRElem<>>>>(fuzz_amount);

    series<SDR<SDRElem<int, UnitData>, std::set<SDRElem<int, UnitData>, std::less<>>>, SDR<SDRElem<>, std::vector<SDRElem<>>>>(fuzz_amount);
    series<SDR<SDRElem<int, UnitData>, std::set<SDRElem<int, UnitData>, std::less<>>>, SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<int, UnitData>, std::set<SDRElem<int, UnitData>, std::less<>>>, SDR<SDRElem<>, std::forward_list<SDRElem<>>>>(fuzz_amount);

    series<SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>, SDR<SDRElem<>, std::vector<SDRElem<>>>>(fuzz_amount);
    series<SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>, SDR<SDRElem<>, std::set<SDRElem<>, std::less<>>>>(fuzz_amount);
    series<SDR<SDRElem<int, UnitData>, std::forward_list<SDRElem<int, UnitData>>>, SDR<SDRElem<>, std::forward_list<SDRElem<>>>>(fuzz_amount);

    std::cout << "======ID contiguous Comparison======" << std::endl;

    struct SomeBigData {
        SomeBigData(int){}
        bool operator==(const SomeBigData&) const { return true; }
        SomeBigData& operator*=(const SomeBigData&) { return *this; }
        SomeBigData& operator+=(const SomeBigData&) { return *this; }
        SomeBigData& operator-=(const SomeBigData&) { return *this; }
        char data[257]; // some large data
    };

    std::cout << "idc" << std::endl;
    series<SDR<SDRElem<int, ArithData<SomeBigData>>, IDContiguousContainer<SDRElem<int, ArithData<SomeBigData>>>>, SDR<SDRElem<int, ArithData<SomeBigData>>, IDContiguousContainer<SDRElem<int, ArithData<SomeBigData>>>>>(fuzz_amount);

    std::cout << "vec" << std::endl;
    series<SDR<SDRElem<int, ArithData<SomeBigData>>, std::vector<SDRElem<int, ArithData<SomeBigData>>>>, SDR<SDRElem<int, ArithData<SomeBigData>>, std::vector<SDRElem<int, ArithData<SomeBigData>>>>>(fuzz_amount);

    #endif

}