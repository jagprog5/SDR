#include "SparseDistributedRepresentation.hpp"
#include <cstring>
#include <chrono>

using namespace std::chrono;

SDR<> get_sdr(int val) {
    SDR<> ret;
    for (long unsigned i = 0; i < sizeof(decltype(val)) * 8; ++i) {
        if ((1 << i) & val) {
            ret.push_back(i);
        }
    }
    return ret;
}

#define REQUIRE_TRUE(x) if (!(x)) return false;

bool validate_andop(const SDR<>& a, const SDR<>& b, const SDR<>& r) {
    SDR<>::const_iterator it;
    // for every element in a, if it is also in b, then it must be in the result
    for(it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        if (a_elem_in_b) REQUIRE_TRUE(r & a_elem);
    }
    // for every element in b, if it is also in a, then it must be in the result
    for(it = b.cbegin(); it != b.cend(); ++it) {
        auto b_elem = *it;
        bool b_elem_in_a = std::find(a.cbegin(), a.cend(), b_elem) != a.cend();
        if (b_elem_in_a) REQUIRE_TRUE(r & b_elem);
    }
    // the result can't contain any elements not in a or not in b
    for(it = r.cbegin(); it != r.cend(); ++it) {
        auto r_elem = *it;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    return true;
}

bool validate_orop(const SDR<>& a, const SDR<>& b, const SDR<>& r) {
    SDR<>::const_iterator it;
    // every element in a must be in result
    for(it = a.cbegin(); it != a.cend(); ++it) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), *it) != r.cend());
    }
    // every element in b must be in result
    for(it = b.cbegin(); it != b.cend(); ++it) {
        REQUIRE_TRUE(std::find(r.cbegin(), r.cend(), *it) != r.cend());
    }
    // the result can't contain any elements not in a or not in b
    for(it = r.cbegin(); it != r.cend(); ++it) {
        auto r_elem = *it;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    return true;
}

bool validate_xorop(const SDR<>& a, const SDR<>& b, const SDR<>& r) {
    SDR<>::const_iterator it;
    // for every element in a, if it is not in b, then it must be in the result
    for(it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        if (!a_elem_in_b) REQUIRE_TRUE(r & a_elem);
    }
    // for every element in b, if it is not in a, then it must be in the result
    for(it = b.cbegin(); it != b.cend(); ++it) {
        auto b_elem = *it;
        bool b_elem_in_a = std::find(a.cbegin(), a.cend(), b_elem) != a.cend();
        if (!b_elem_in_a) REQUIRE_TRUE(r & b_elem);
    }
    // the result can't contain any elements not in a or not in b
    for(it = r.cbegin(); it != r.cend(); ++it) {
        auto r_elem = *it;
        bool in_a = std::find(a.cbegin(), a.cend(), r_elem) != a.cend();
        bool in_b = std::find(b.cbegin(), b.cend(), r_elem) != b.cend();
        REQUIRE_TRUE(in_a || in_b);
    }
    return true;
}

bool validate_rmop(const SDR<>& a, const SDR<>& b, const SDR<>& r) {
    SDR<>::const_iterator it;
    // for every elements in a, if it is not in b, then it must be in the result
    unsigned long i = 0;
    for(it = a.cbegin(); it != a.cend(); ++it) {
        auto a_elem = *it;
        bool a_elem_in_b = std::find(b.cbegin(), b.cend(), a_elem) != b.cend();
        if (!a_elem_in_b) REQUIRE_TRUE(r & a_elem);
        if (!a_elem_in_b) i++;
    }
    // the result can't contain any other elements
    REQUIRE_TRUE(i == r.size());
    return true;
}

bool validate_separate(const SDR<>& a_before, const SDR<>& b_before, const SDR<>& a, const SDR<>& b) {
    return a == a_before - b_before && b == b_before - a_before;
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
        fuzz_amount = 10000;
    }
    assert(fuzz_amount >= 100);

    duration<int64_t, std::nano> and_time(0);
    duration<int64_t, std::nano> andi_time(0);
    duration<int64_t, std::nano> ands_time(0);
    duration<int64_t, std::nano> or_time(0);
    duration<int64_t, std::nano> ori_time(0);
    duration<int64_t, std::nano> ors_time(0);
    duration<int64_t, std::nano> xor_time(0);
    duration<int64_t, std::nano> xori_time(0);
    duration<int64_t, std::nano> xors_time(0);
    duration<int64_t, std::nano> rm_time(0);
    duration<int64_t, std::nano> rmi_time(0);
    duration<int64_t, std::nano> rms_time(0);


    for (int i = 0; i < fuzz_amount; ++i) {
        if (i % (fuzz_amount / 100) == 0)
            std::cout << "\r" << (float)(i) / fuzz_amount * 100 << "%" << std::flush;
        SDR<> a = get_sdr(i);
        for (int j = 0; j < fuzz_amount; ++j) {
            SDR<> b = get_sdr(j);
            {
                auto start = high_resolution_clock::now();
                SDR<> and_result = a.andb(b);
                auto stop = high_resolution_clock::now();
                if (!validate_andop(a, b, and_result)) {
                    std::cerr << "andb failed! " << i << " " << j << "\n";
                    exit(1);
                }
                and_time += stop - start;
                
                SDR<> a_cp(a);
                auto start_i = high_resolution_clock::now();
                SDR<> andi_result = a_cp.andi(b);
                auto stop_i = high_resolution_clock::now();
                if (a_cp != and_result) {
                    std::cerr << "andi failed! " << i << " " << j << "\n";
                    exit(1);
                }
                andi_time += stop_i - start_i;

                auto start_s = high_resolution_clock::now();
                auto size = a.ands(b);
                auto stop_s = high_resolution_clock::now();
                if (size != and_result.size()) {
                    std::cerr << "ands failed! " << i << " " << j << "\n";
                    exit(1);
                }
                ands_time += stop_s - start_s;
            }
            {
                auto start = high_resolution_clock::now();
                SDR<> or_result = a.orb(b);
                auto stop = high_resolution_clock::now();
                if (!validate_orop(a, b, or_result)) {
                    std::cerr << "orb failed! " << i << " " << j << "\n";
                    exit(1);
                }
                or_time += stop - start;
                
                SDR<> a_cp(a);
                auto start_i = high_resolution_clock::now();
                SDR<> ori_result = a_cp.ori(b);
                auto stop_i = high_resolution_clock::now();
                if (a_cp != or_result) {
                    std::cerr << "ori failed! " << i << " " << j << "\n";
                    exit(1);
                }
                ori_time += stop_i - start_i;

                auto start_s = high_resolution_clock::now();
                auto size = a.ors(b);
                auto stop_s = high_resolution_clock::now();
                if (size != or_result.size()) {
                    std::cerr << "ors failed! " << i << " " << j << "\n";
                    exit(1);
                }
                ors_time += stop_s - start_s;
            }
            {
                auto start = high_resolution_clock::now();
                SDR<> xor_result = a.xorb(b);
                auto stop = high_resolution_clock::now();
                if (!validate_xorop(a, b, xor_result)) {
                    std::cerr << "xorb failed! " << i << " " << j << "\n";
                    exit(1);
                }
                xor_time += stop - start;
                
                SDR<> a_cp(a);
                auto start_i = high_resolution_clock::now();
                SDR<> xori_result = a_cp.xori(b);
                auto stop_i = high_resolution_clock::now();
                if (a_cp != xor_result) {
                    std::cerr << "xori failed! " << i << " " << j << "\n";
                    exit(1);
                }
                xori_time += stop_i - start_i;

                auto start_s = high_resolution_clock::now();
                auto size = a.xors(b);
                auto stop_s = high_resolution_clock::now();
                if (size != xor_result.size()) {
                    std::cerr << "xors failed! " << i << " " << j << "\n";
                    exit(1);
                }
                xors_time += stop_s - start_s;
            }
            {
                auto start = high_resolution_clock::now();
                SDR<> rm_result = a.rmb(b);
                auto stop = high_resolution_clock::now();
                if (!validate_rmop(a, b, rm_result)) {
                    std::cerr << "rmb failed! " << i << " " << j << "\n";
                    exit(1);
                }
                rm_time += stop - start;
                
                SDR<> a_cp(a);
                auto start_i = high_resolution_clock::now();
                SDR<> rmi_result = a_cp.rmi(b);
                auto stop_i = high_resolution_clock::now();
                if (a_cp != rm_result) {
                    std::cerr << "rmi failed! " << i << " " << j << "\n";
                    exit(1);
                }
                rmi_time = stop_i - start_i;
                
                auto start_s = high_resolution_clock::now();
                auto size = a.rms(b);
                auto stop_s = high_resolution_clock::now();
                rms_time = stop_s - start_s;
                if (size != rm_result.size()) {
                    std::cerr << "rms failed! " << i << " " << j << "\n";
                    exit(1);
                }

            }
        }
    }
    std::cout << "\rDone!\n";
    std::cout << "and_time: " << and_time.count() << std::endl;
    std::cout << "andi_time: " << andi_time.count() << std::endl;
    std::cout << "ands_time: " << ands_time.count() << std::endl;
    std::cout << "or_time: " << or_time.count() << std::endl;
    std::cout << "ori_time: " << ori_time.count() << std::endl;
    std::cout << "ors_time: " << ors_time.count() << std::endl;
    std::cout << "xor_time: " << xor_time.count() << std::endl;
    std::cout << "xori_time: " << xori_time.count() << std::endl;
    std::cout << "xors_time: " << xors_time.count() << std::endl;
    std::cout << "rm_time: " << rm_time.count() << std::endl;
    std::cout << "rmi_time: " << rmi_time.count() << std::endl;
    std::cout << "rms_time: " << rms_time.count() << std::endl;
}