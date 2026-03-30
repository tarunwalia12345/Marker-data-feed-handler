#pragma once

#include <vector>
#include <random>

class TickGenerator {
public:
    explicit TickGenerator(int n);

    double next(int id);

private:
    std::vector<double> prices;
    std::vector<double> sigma;

    std::mt19937 rng;
    std::normal_distribution<double> norm;

    double dt;
};