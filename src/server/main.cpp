#include "exchange_simulator.h"

int main() {
    ExchangeSimulator sim(9876, 100);

    sim.start();
    sim.run();

    return 0;
}