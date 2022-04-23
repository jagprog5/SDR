#include "SparseDistributedRepresentation.hpp"
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <random>

using namespace SparseDistributedRepresentation;
using namespace std::chrono;

#define REQUIRE_TRUE(x) if (!(x)) return false;

// only do a speed test, don't check for correctness
static constexpr bool disable_validation = false;

template<typename SDRA, typename SDRB>
bool validate_andop(const SDRA& a, const SDRB& b, const SDRA& r) {
    // for every element in a, if it is also in b, then it must be in the result
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        auto a_elem = *a_pos;
        auto b_pos = std::find(b.cbegin(), b.cend(), a_elem);
        if (b_pos != b.cend()) {
            // a element is in b
            auto data = a_elem.data.andb(b_pos->data);
            if (data.relevant()) {
                REQUIRE_TRUE(r & a_elem);
            }
        }
    }
    // for every element in b, if it is also in a, then it must be in the result
    for(auto b_pos = b.cbegin(); b_pos != b.cend(); ++b_pos) {
        auto b_elem = *b_pos;
        auto a_pos = std::find(a.cbegin(), a.cend(), b_elem);
        if (a_pos != a.cend()) {
            // b element is in a
            auto data = a_pos->data.andb(b_elem.data);
            if (data.relevant()) {
                REQUIRE_TRUE(r & b_elem);
            }
        }
    }
    // the result can't contain any elements not in a or not in b
    typename SDRA::size_type i = 0;
    for(auto r_pos = r.cbegin(); r_pos != r.cend(); ++r_pos) {
        ++i;
        auto r_elem = *r_pos;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    // ensure the size is correct
    REQUIRE_TRUE(i == r.size());
    return true;
}

template<typename SDRA, typename SDRB>
bool validate_orop(const SDRA& a, const SDRB& b, const SDRA& r) {
    // every element in a must be in result
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), *a_pos) != r.cend());
    }
    // every element in b must be in result
    for(auto b_pos = b.cbegin(); b_pos != b.cend(); ++b_pos) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), *b_pos) != r.cend());
    }
    // the result can't contain any elements not in a or not in b
    typename SDRA::size_type i = 0;
    for(auto r_pos = r.cbegin(); r_pos != r.cend(); ++r_pos) {
        ++i;
        auto r_elem = *r_pos;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    // ensure the size is correct
    REQUIRE_TRUE(i == r.size());
    return true;
}

template<typename SDRA, typename SDRB>
bool validate_xorop(const SDRA& a, const SDRB& b, const SDRA& r) {
    // for every element in a, if it is not in b, then it must be in the result
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        auto a_elem = *a_pos;
        auto b_pos = std::find(b.cbegin(), b.cend(), a_elem);
        if (b_pos == b.cend() || a_elem.data.xorb(b_pos->data).rm_relevant()) {
            // a elem is not in b
            REQUIRE_TRUE(r & a_elem)
        }
    }
    // for every element in b, if it is not in a, then it must be in the result
    for(auto b_pos = b.cbegin(); b_pos != b.cend(); ++b_pos) {
        auto b_elem = *b_pos;
        auto a_pos = std::find(a.cbegin(), a.cend(), b_elem);
        if (a_pos == a.cend() || a_pos->data.xorb(b_elem.data).rm_relevant()) {
            // b elem is not in a
            REQUIRE_TRUE(r & b_elem);
        }
    }
    // the result can't contain any elements not in a or not in b
    typename SDRA::size_type i = 0;
    for(auto r_pos = r.cbegin(); r_pos != r.cend(); ++r_pos) {
        ++i;
        auto r_elem = *r_pos;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    // ensure the size is correct
    REQUIRE_TRUE(i == r.size());
    return true;
}

template<typename SDRA, typename SDRB>
bool validate_rmop(const SDRA& a, const SDRB& b, const SDRA& r) {
    // for every elements in a, if it is not in b, then it must be in the result
    typename SDRA::size_type i = 0;
    for(auto a_pos = a.cbegin(); a_pos != a.cend(); ++a_pos) {
        auto a_elem = *a_pos;
        auto b_pos = std::find(b.cbegin(), b.cend(), a_elem);
        if (b_pos == b.cend() || a_elem.data.rmb(b_pos->data).rm_relevant()) {
            // a elem is not in b
            REQUIRE_TRUE(r & a_elem);
            ++i;
        }
    }
    // the result can't contain any other elements
    REQUIRE_TRUE(i == r.size());
    return true;
}

inline std::mt19937 twister(time(NULL) * getpid());

// generate a unique SDR based on a number
// if the specialization uses SDRFloatData, then generate some random data as well
template<typename SDR>
SDR get_sdr(int val) {
    SDR ret;
    long start;
    if constexpr(SDR::usesForwardList) {
        start = sizeof(decltype(val)) * 8 - 1;
    } else {
        start = 0;
    }
    long stop; // exclusive
    if constexpr(SDR::usesForwardList) {
        stop = -1;
    } else {
        stop = sizeof(decltype(val)) * 8;
    }
    long change;
    if constexpr(SDR::usesForwardList) {
        change = -1;
    } else {
        change = 1;
    }
    for (long i = start; i != stop; i += change) {
        if ((1 << i) & val) {
            typename SDR::value_type::data_type data;
            if constexpr(std::is_same<typename SDR::value_type::data_type, SDRFloatData>::value) {
                data.value = (float)twister() / (float)twister.max();
            }
            typename SDR::value_type elem(i, data);
            if constexpr(SDR::usesForwardList) {
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
    if constexpr(SDR::usesVector) {
        return "vec";
    } else if constexpr(SDR::usesSet) {
        return "set";
    } else if constexpr(SDR::usesForwardList) {
        return "lst";
    } else {
        return "?";
    }
}

// name: some sort of identifier
// f: the function to time and test. should return false if it failed
template<typename SDRA, typename SDRB, typename funct>
void time_op(std::string name, funct f, int fuzz_amount) {
    duration<int64_t, std::nano> duration(0);
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
    auto andb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA and_result = a.andb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_andop(a, b, and_result);
    };
    time_op<SDRA, SDRB>("andb", andb, fuzz_amount);

    auto andi = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.andi(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_andop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>("andi", andi, fuzz_amount);

    auto ands = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.ands(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || s == a.andb(b).size();
    };
    time_op<SDRA, SDRB>("ands", ands, fuzz_amount);

    auto orb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA or_result = a.orb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_orop(a, b, or_result);
    };
    time_op<SDRA, SDRB>(" orb", orb, fuzz_amount);

    auto ori = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.ori(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_orop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>(" ori", ori, fuzz_amount);

    auto ors = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.ors(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || s == a.orb(b).size();
    };
    time_op<SDRA, SDRB>(" ors", ors, fuzz_amount);

    auto xorb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA xor_result = a.xorb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_xorop(a, b, xor_result);
    };
    time_op<SDRA, SDRB>("xorb", xorb, fuzz_amount);

    auto xori = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.xori(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_xorop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>("xori", xori, fuzz_amount);

    auto xors = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.xors(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || s == a.xorb(b).size();
    };
    time_op<SDRA, SDRB>("xors", xors, fuzz_amount);

    auto rmb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA rm_result = a.rmb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_rmop(a, b, rm_result);
    };
    time_op<SDRA, SDRB>(" rmb", rmb, fuzz_amount);

    auto rmi = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.rmi(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return disable_validation || validate_rmop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>(" rmi", rmi, fuzz_amount);

    auto rms = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.rms(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return s == a.rmb(b).size();
    };
    time_op<SDRA, SDRB>(" rms", rms, fuzz_amount);    
}

int main(int argc, char** argv) {
    int fuzz_amount;
    if (argc > 1) {
        if (std::strcmp(argv[1], "--help") == 0) {
            std::cout << "Usage: fuzz_sdr [<amount>]\n";
            exit(0);
        }
        fuzz_amount = std::atoi(argv[1]);
    } else {
        fuzz_amount = 1000;
    }

    // yes, this makes a stupidly large binary from all the template specializations.
    // but no. realistically, nobody will make nearly this many specializations

    series<SDR<SDR_t<>, std::vector<SDR_t<>>>, SDR<SDR_t<>, std::vector<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<>, std::vector<SDR_t<>>>, SDR<SDR_t<>, std::set<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<>, std::vector<SDR_t<>>>, SDR<SDR_t<>, std::forward_list<SDR_t<>>>>(fuzz_amount);

    series<SDR<SDR_t<>, std::set<SDR_t<>>>, SDR<SDR_t<>, std::vector<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<>, std::set<SDR_t<>>>, SDR<SDR_t<>, std::set<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<>, std::set<SDR_t<>>>, SDR<SDR_t<>, std::forward_list<SDR_t<>>>>(fuzz_amount);

    series<SDR<SDR_t<>, std::forward_list<SDR_t<>>>, SDR<SDR_t<>, std::vector<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<>, std::forward_list<SDR_t<>>>, SDR<SDR_t<>, std::set<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<>, std::forward_list<SDR_t<>>>, SDR<SDR_t<>, std::forward_list<SDR_t<>>>>(fuzz_amount);

    std::cout << "======With float data elements======" << std::endl;

    series<SDR<SDR_t<long, SDRFloatData>, std::vector<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::vector<SDR_t<int, SDRFloatData>>>>(fuzz_amount);
    series<SDR<SDR_t<long, SDRFloatData>, std::vector<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::set<SDR_t<int, SDRFloatData>>>>(fuzz_amount);
    series<SDR<SDR_t<long, SDRFloatData>, std::vector<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::forward_list<SDR_t<int, SDRFloatData>>>>(fuzz_amount);

    series<SDR<SDR_t<long, SDRFloatData>, std::set<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::vector<SDR_t<int, SDRFloatData>>>>(fuzz_amount);
    series<SDR<SDR_t<long, SDRFloatData>, std::set<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::set<SDR_t<int, SDRFloatData>>>>(fuzz_amount);
    series<SDR<SDR_t<long, SDRFloatData>, std::set<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::forward_list<SDR_t<int, SDRFloatData>>>>(fuzz_amount);

    series<SDR<SDR_t<long, SDRFloatData>, std::forward_list<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::vector<SDR_t<int, SDRFloatData>>>>(fuzz_amount);
    series<SDR<SDR_t<long, SDRFloatData>, std::forward_list<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::set<SDR_t<int, SDRFloatData>>>>(fuzz_amount);
    series<SDR<SDR_t<long, SDRFloatData>, std::forward_list<SDR_t<long, SDRFloatData>>>, SDR<SDR_t<int, SDRFloatData>, std::forward_list<SDR_t<int, SDRFloatData>>>>(fuzz_amount);

    std::cout << "======Mixed with and without data======" << std::endl;

    series<SDR<SDR_t<int, SDRFloatData>, std::vector<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::vector<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<int, SDRFloatData>, std::vector<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::set<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<int, SDRFloatData>, std::vector<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::forward_list<SDR_t<>>>>(fuzz_amount);

    series<SDR<SDR_t<int, SDRFloatData>, std::set<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::vector<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<int, SDRFloatData>, std::set<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::set<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<int, SDRFloatData>, std::set<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::forward_list<SDR_t<>>>>(fuzz_amount);

    series<SDR<SDR_t<int, SDRFloatData>, std::forward_list<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::vector<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<int, SDRFloatData>, std::forward_list<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::set<SDR_t<>>>>(fuzz_amount);
    series<SDR<SDR_t<int, SDRFloatData>, std::forward_list<SDR_t<int, SDRFloatData>>>, SDR<SDR_t<>, std::forward_list<SDR_t<>>>>(fuzz_amount);
}