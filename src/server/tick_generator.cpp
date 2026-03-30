#include "tick_generator.h"
#include <cmath>
#include <algorithm>

TickGenerator::TickGenerator(int n)
    : prices(n),
      sigma(n),
      rng(std::random_device{}()),
      norm(0.0, 1.0),
      dt(0.001)
{
    std::uniform_real_distribution<double> price_dist(100.0, 5000.0);
    std::uniform_real_distribution<double> vol_dist(0.005, 0.02);
    for (int i = 0; i < n; i++) {
        prices[i] = price_dist(rng);
        sigma[i] = vol_dist(rng);
    }
}

double TickGenerator::next(int id) {

    double S = prices[id];
    double mu = 0.0;
    double Z = norm(rng);

    double new_price = S * std::exp(
        (mu - 0.5 * sigma[id] * sigma[id]) * dt +
        sigma[id] * std::sqrt(dt) * Z
    );

    new_price = std::max(100.0, new_price);
    prices[id] = new_price;

    return new_price;
}