#include "SparseDistributedRepresentation.hpp"
#include <cstring>
#include <chrono>

using namespace std::chrono;

#define REQUIRE_TRUE(x) if (!(x)) return false;

template<typename SDRA, typename SDRB>
bool validate_andop(const SDRA& a, const SDRB& b, const SDRA& r) {
    // for every element in a, if it is also in b, then it must be in the result
    for(auto it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        if (a_elem_in_b) REQUIRE_TRUE(r & a_elem);
    }
    // for every element in b, if it is also in a, then it must be in the result
    for(auto it = b.cbegin(); it != b.cend(); ++it) {
        auto b_elem = *it;
        bool b_elem_in_a = std::find(a.cbegin(), a.cend(), b_elem) != a.cend();
        if (b_elem_in_a) REQUIRE_TRUE(r & b_elem);
    }
    // the result can't contain any elements not in a or not in b
    typename SDRA::size_type i = 0;
    for(auto it = r.cbegin(); it != r.cend(); ++it) {
        ++i;
        auto r_elem = *it;
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
    for(auto it = a.cbegin(); it != a.cend(); ++it) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), *it) != r.cend());
    }
    // every element in b must be in result
    for(auto it = b.cbegin(); it != b.cend(); ++it) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), *it) != r.cend());
    }
    // the result can't contain any elements not in a or not in b
    typename SDRA::size_type i = 0;
    for(auto it = r.cbegin(); it != r.cend(); ++it) {
        ++i;
        auto r_elem = *it;
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
    for(auto it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        if (!a_elem_in_b) REQUIRE_TRUE(r & a_elem);
    }
    // for every element in b, if it is not in a, then it must be in the result
    for(auto it = b.cbegin(); it != b.cend(); ++it) {
        auto b_elem = *it;
        bool b_elem_in_a = std::find(a.cbegin(), a.cend(), b_elem) != a.cend();
        if (!b_elem_in_a) REQUIRE_TRUE(r & b_elem);
    }
    // the result can't contain any elements not in a or not in b
    typename SDRA::size_type i = 0;
    for(auto it = r.cbegin(); it != r.cend(); ++it) {
        ++i;
        auto r_elem = *it;
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
    for(auto it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        if (!a_elem_in_b) REQUIRE_TRUE(r & a_elem);
        if (!a_elem_in_b) i++;
    }
    // the result can't contain any other elements
    REQUIRE_TRUE(i == r.size());
    return true;
}

// generate a unique SDR based on a number
template<typename SDR>
SDR get_sdr(unsigned int val) {
    SDR ret;
    [[maybe_unused]] typename SDR::iterator it;
    if constexpr(SDR::usesForwardList) {
        it = ret.before_begin();
    }
    for (long unsigned i = 0; i < sizeof(decltype(val)) * 8; ++i) {
        if ((1 << i) & val) {
            if constexpr(SDR::usesForwardList) {
                it = ret.insert_end(it, i);
            } else {
                ret.push_back(i);
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
void time_op(std::string name, funct f, unsigned int fuzz_amount) {
    duration<int64_t, std::nano> duration(0);
    for (unsigned int i = 0; i < fuzz_amount; ++i) {
        for (unsigned int j = 0; j < fuzz_amount; ++j) {
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
void series(unsigned int fuzz_amount) {
    auto andb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA and_result = a.andb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_andop(a, b, and_result);
    };
    time_op<SDRA, SDRB>("andb", andb, fuzz_amount);

    auto andi = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.andi(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_andop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>("andi", andi, fuzz_amount);

    auto ands = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.ands(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return s == a.andb(b).size();
    };
    time_op<SDRA, SDRB>("ands", ands, fuzz_amount);

    auto orb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA or_result = a.orb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_orop(a, b, or_result);
    };
    time_op<SDRA, SDRB>(" orb", orb, fuzz_amount);

    auto ori = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.ori(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_orop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>(" ori", ori, fuzz_amount);

    auto ors = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.ors(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return s == a.orb(b).size();
    };
    time_op<SDRA, SDRB>(" ors", ors, fuzz_amount);

    auto xorb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA xor_result = a.xorb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_xorop(a, b, xor_result);
    };
    time_op<SDRA, SDRB>("xorb", xorb, fuzz_amount);

    auto xori = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.xori(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_xorop(a, b, a_cp);
    };
    time_op<SDRA, SDRB>("xori", xori, fuzz_amount);

    auto xors = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        auto s = a.xors(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return s == a.xorb(b).size();
    };
    time_op<SDRA, SDRB>("xors", xors, fuzz_amount);

    auto rmb = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        auto start = high_resolution_clock::now();
        SDRA rm_result = a.rmb(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_rmop(a, b, rm_result);
    };
    time_op<SDRA, SDRB>(" rmb", rmb, fuzz_amount);

    auto rmi = [](const SDRA& a, const SDRB& b, duration<int64_t, std::nano>& total_time) {
        SDRA a_cp(a);
        auto start = high_resolution_clock::now();
        a_cp.rmi(b);
        auto stop = high_resolution_clock::now();
        total_time += stop - start;
        return validate_rmop(a, b, a_cp);
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

    series<SDR<int, std::vector<int>>, SDR<int, std::vector<int>>>(fuzz_amount);
    series<SDR<int, std::vector<int>>, SDR<int, std::set<int>>>(fuzz_amount);
    series<SDR<int, std::vector<int>>, SDR<int, std::forward_list<int>>>(fuzz_amount);

    series<SDR<int, std::set<int>>, SDR<int, std::vector<int>>>(fuzz_amount);
    series<SDR<int, std::set<int>>, SDR<int, std::set<int>>>(fuzz_amount);
    series<SDR<int, std::set<int>>, SDR<int, std::forward_list<int>>>(fuzz_amount);

    series<SDR<int, std::forward_list<int>>, SDR<int, std::vector<int>>>(fuzz_amount);
    series<SDR<int, std::forward_list<int>>, SDR<int, std::set<int>>>(fuzz_amount);
    series<SDR<int, std::forward_list<int>>, SDR<int, std::forward_list<int>>>(fuzz_amount);
}