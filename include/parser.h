#pragma once

#include <vector>
#include <cstddef>
#include "cache.h"

class Parser {
    std::vector<char> buffer;
    Cache& cache;

public:
    explicit Parser(Cache& c);
    void on_data(const char* data, size_t len);
};