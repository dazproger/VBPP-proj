#pragma once

#include <stdexcept>
#include <iostream>
#include <vector>
#include <memory>
#include <random>

std::vector<std::vector<long long>> create_instance(int from, int to, long long dimensions, long long amount_of_vms, std::mt19937& gen);

bool IsVecFits(std::vector<long long> a, std::vector<long long> b) {
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] > b[i]) {
            return false;
        }
    }
    return true;
}

void PutItemIn(std::vector<long long>& a, std::vector<long long>& b) {
    for (size_t i = 0; i < a.size(); ++i) {
        b[i] -= a[i];
    }
}

struct FFD_Comps {
    virtual bool operator()(std::vector<long long>& a, std::vector<long long>& b) = 0;
};

struct FFDsum_Comp : public FFD_Comps {
    FFDsum_Comp(std::vector<long long> factors) : factors_(factors) {}

    bool operator()(std::vector<long long>& a, std::vector<long long>& b) override;

    std::vector<long long> factors_;
};

struct FFDmul_Comp : public FFD_Comps {
    FFDmul_Comp() {}

    bool operator()(std::vector<long long>& a, std::vector<long long>& b) override;
};

struct Dot_Product_Comp {
    explicit Dot_Product_Comp(std::vector<long long> factors) : factors_(factors) {}

    long long operator()(std::vector<long long>&, std::vector<long long>&);

    std::vector<long long> factors_;

};

struct Sample_Variance_Comp {
    explicit Sample_Variance_Comp() {}

    long long operator()(std::vector<long long>&, std::vector<long long>&);

};

struct Minmax_Comp {
    explicit Minmax_Comp() {}

    long long operator()(std::vector<long long>, std::vector<long long>);

};

struct L2_Based_Norm {
    explicit L2_Based_Norm(std::vector<long long> factors) : factors_(factors) {}

    long long operator()(const std::vector<long long>&, const std::vector<long long>&);

    std::vector<long long> factors_;
};

struct Dot_Minus_L2x4 {
    explicit Dot_Minus_L2x4(std::vector<long long> factors) : factors_(factors) {}

    long long operator()(const std::vector<long long>&, const std::vector<long long>&);

    std::vector<long long> factors_;
};

struct Bins {
    Bins(std::vector<std::vector<long long>> items, std::vector<long long> capacities) : items_(items), capacities_(capacities) {
        for (size_t i = 0; i < items.size(); ++i) {
            bins_.emplace_back(capacities);
        }
    }
    template <typename T>
    long long FFD(T&);

    template <typename T>
    long long FFD3(T&);

    template <typename T>
    long long FFD_BinCentric(T&);

    template <typename T>
    long long FFD_BinCentric2(T& Norm_Operator);

    template <typename T>
    long long FFD_BinCentric3(T& Norm_Operator);

    long long LowerBound();

    std::vector<std::vector<long long> > items_;
    std::vector<std::vector<long long> > bins_;
    std::vector<long long> capacities_;
};

