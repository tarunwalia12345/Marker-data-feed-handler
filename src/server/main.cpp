#include "exchange_simulator.h"

int main() {
    ExchangeSimulator sim(9877, 100);

    sim.start();
    sim.run();

    return 0;
}
