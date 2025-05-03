#include "project.h"
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <cmath>
#include <climits>
#include <numeric>
#include <chrono>
#include <random>
#include <array>

// Constants
constexpr int CAPACITY_SIZE = 1000;
constexpr long long ITEM_AMOUNT = 1000;
constexpr int AMOUNT_OF_ALGORITHMS = 6;
constexpr int OUTPUT_SIZE = 400;

// Predefined arrays
constexpr auto H_VEC = []() {
    std::array<int, 390> arr{};
    for (int i = 0; i < 390; ++i) {
        arr[i] = i + 1;
    }
    return arr;
}();

constexpr int DIMENSIONS[] = {4};
constexpr int H_VEC_SIZE = H_VEC.size();
constexpr int DIMENSIONS_SIZE = std::size(DIMENSIONS);

// Global data structures
std::vector<int> output(OUTPUT_SIZE, 0);
std::vector<std::vector<long long>> results(H_VEC_SIZE, 
                                          std::vector<long long>(AMOUNT_OF_ALGORITHMS));
std::vector<std::vector<long long>> untouched_results = results;
std::vector<std::vector<std::chrono::duration<float>>> time_used(12, 
                                          std::vector<std::chrono::duration<float>>(10));

// Utility functions
std::vector<long long> calculate_sum_factors(const std::vector<std::vector<long long>>& instance) {
    std::vector<long long> factors(instance[0].size(), 0);
    for (const auto& item : instance) {
        for (size_t i = 0; i < factors.size(); ++i) {
            factors[i] += item[i];
        }
    }
    return factors;
}

std::vector<long long> calculate_exp_factors(const std::vector<std::vector<long long>>& instance) {
    auto factors = calculate_sum_factors(instance);
    for (size_t i = 0; i < factors.size(); ++i) {
        factors[i] = static_cast<long long>(exp(static_cast<float>(i) * 0.01));
    }
    return factors;
}

// Bin packing algorithms implementation
template <typename T>
long long Bins::FFD(T& comparator) {
    auto saved_items = items_;
    auto saved_bins = bins_;
    
    std::sort(items_.begin(), items_.end(), comparator);
    long long used_bins = 0;
    
    for (auto& item : items_) {
        for (size_t j = 0; j < bins_.size(); ++j) {
            if (IsVecFits(item, bins_[j])) {
                PutItemIn(item, bins_[j]);
                used_bins = std::max(static_cast<long long>(j) + 1, used_bins);
                break;
            }
        }
    }
    
    items_ = std::move(saved_items);
    bins_ = std::move(saved_bins);
    return used_bins;
}

template <typename T>
long long Bins::FFD_BinCentric(T& norm_operator) {
    auto saved_items = items_;
    auto saved_bins = bins_;
    long long bin_id = 0;
    size_t processed_items = 0;
    
    while (processed_items < items_.size()) {
        long long max_item_id = -1;
        long long max_norm = LLONG_MIN;
        
        for (size_t i = 0; i < items_.size(); ++i) {
            if (items_[i][0] != -1 && 
                IsVecFits(items_[i], bins_[bin_id]) && 
                max_norm < norm_operator(items_[i], bins_[bin_id])) {
                max_norm = norm_operator(items_[i], bins_[bin_id]);
                max_item_id = i;
            }
        }
        
        if (max_item_id == -1) {
            bin_id++;
        } else {
            PutItemIn(items_[max_item_id], bins_[bin_id]);
            items_[max_item_id] = std::vector<long long>(items_[0].size(), -1);
            processed_items++;
        }
    }
    
    items_ = std::move(saved_items);
    bins_ = std::move(saved_bins);
    return bin_id + 1;
}

// Comparison operators
bool FFDsum_Comp::operator()(std::vector<long long>& a, std::vector<long long>& b) {
    long long diff = 0;
    for (size_t i = 0; i < factors_.size(); ++i) {
        diff += (factors_[i] * (b[i] - a[i]));
    }
    return diff < 0;
}

bool FFDmul_Comp::operator()(std::vector<long long>& a, std::vector<long long>& b) {
    long long left = 1, right = 1;
    for (size_t i = 0; i < a.size(); ++i) {
        left *= a[i];
        right *= b[i];
    }
    return left > right;
}

// Norm operators
long long Dot_Product_Comp::operator()(std::vector<long long>& item, 
                                      std::vector<long long>& bin) {
    long long result = 0;
    for (size_t i = 0; i < item.size(); ++i) {
        result += factors_[i] * item[i] * bin[i];
    }
    return result;
}

long long Sample_Variance_Comp::operator()(std::vector<long long>& item, 
                                         std::vector<long long>& bin) {
    long long sum = std::accumulate(item.begin(), item.end(), 
                    std::accumulate(bin.begin(), bin.end(), 0LL));
    long long avg = sum / item.size();
    
    long long variance = 0;
    for (size_t i = 0; i < item.size(); ++i) {
        long long diff = item[i] + bin[i] - avg;
        variance += diff * diff;
    }
    return variance;
}

long long L2_Based_Norm::operator()(const std::vector<long long>& item, 
                                   const std::vector<long long>& bin) {
    long long norm = 0;
    for (size_t i = 0; i < item.size(); ++i) {
        long long diff = bin[i] - item[i];
        norm += factors_[i] * diff * diff;
    }
    return -norm;
}

long long Bins::LowerBound() {
    std::vector<long long> max_taken(bins_[0].size(), 0);
    for (const auto& item : items_) {
        for (size_t j = 0; j < item.size(); ++j) {
            max_taken[j] += item[j];
        }
    }
    long long m = *std::max_element(max_taken.begin(), max_taken.end());
    return (m + CAPACITY_SIZE - 1) / CAPACITY_SIZE;
}

// Test functions
std::vector<std::vector<long long>> create_instance(int from, int to, 
                                                   long long dimensions, 
                                                   long long amount,
                                                   std::mt19937& gen) {
    std::vector<std::vector<long long>> instance(amount, 
                                               std::vector<long long>(dimensions));
    std::uniform_int_distribution<> dist(from, to);
    
    for (auto& item : instance) {
        for (auto& val : item) {
            val = dist(gen);
        }
    }
    return instance;
}

void run_single_test(long long from, long long to, 
                    long long dimensions, long long item_count, 
                    long long test_idx, int current_dim) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<long long> capacities(current_dim, CAPACITY_SIZE);
    
    auto instance = create_instance(from, to, dimensions, item_count, gen);
    Bins bins(instance, capacities);
    
    std::vector<long long> factors(dimensions, 0);
    Dot_Product_Comp dot_comp(factors);
    FFDsum_Comp ffdsum_comp(factors);
    FFDmul_Comp ffdmul_comp;
    L2_Based_Norm l2_norm(factors);
    Sample_Variance_Comp variance_comp;
    
    results[test_idx][0] += bins.FFD_BinCentric(dot_comp);
    results[test_idx][1] += bins.FFD_BinCentric(l2_norm);
    results[test_idx][2] += bins.FFD_BinCentric(variance_comp);
    results[test_idx][3] += bins.FFD(ffdsum_comp);
    results[test_idx][4] += bins.FFD(ffdmul_comp);
    results[test_idx][5] += bins.LowerBound();
}

void run_test_suite() {
    for (int dim_idx = 0; dim_idx < DIMENSIONS_SIZE; ++dim_idx) {
        long long current_dim = DIMENSIONS[dim_idx];
        
        for (auto h : H_VEC) {
            std::cout << '#' << std::flush;

            std::vector<std::vector<int>> test_ranges = {
                {h - h / 2, h + h / 2}
            };
            
            for (size_t test_idx = 0; test_idx < test_ranges.size(); ++test_idx) {
                run_single_test(test_ranges[test_idx][0], test_ranges[test_idx][1],
                              current_dim, ITEM_AMOUNT, test_idx, DIMENSIONS[dim_idx]);
            }
            
            int min_result = INT_MAX;
            for (size_t i = 0; i < test_ranges.size(); ++i) {
                for (size_t j = 0; j < AMOUNT_OF_ALGORITHMS; ++j) {
                    if (j != 2 && j != 5) {
                        min_result = std::min(static_cast<int>(results[i][j]), min_result);
                    }
                }
            }
            
            output[h] += floor(100 * (static_cast<float>(min_result) / results[0][2] - 1));
            results = untouched_results;
        }
    }
}

int main() {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Redirect output to file
    close(STDOUT_FILENO);
    open("output", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    
    // Run tests multiple times
    constexpr int TEST_RUNS = 10;
    for (int i = 0; i < TEST_RUNS; ++i) {
        run_test_suite();
    }
    
    // Output averaged results
    for (auto count : output) {
        std::cout << count / TEST_RUNS << ", ";
    }
    std::cout << '\n';
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = end_time - start_time;
    std::cout << "Total execution time: " << elapsed.count() << " seconds\n";
    
    return 0;
}