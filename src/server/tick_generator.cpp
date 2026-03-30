#include <random>
#include <vector>
#include <cmath>

class TickGenerator {
    std::vector<double> prices;
    std::vector<double> sigma;
    std::mt19937 rng;
    std::normal_distribution<> norm{0.0, 1.0};

public:
    TickGenerator(int n) : prices(n), sigma(n), rng(std::random_device{}()) {
        std::uniform_real_distribution<> price_dist(100, 5000);
        std::uniform_real_distribution<> vol_dist(0.01, 0.06);

        for (int i = 0; i < n; i++) {
            prices[i] = price_dist(rng);
            sigma[i] = vol_dist(rng);
        }
    }

    double next(int id) {
        double S = prices[id];
        double mu = 0.0;
        double dt = 0.001;

        double dW = norm(rng) * sqrt(dt);
        double dS = mu * S * dt + sigma[id] * S * dW;

        prices[id] += dS;
        return prices[id];
    }
};
