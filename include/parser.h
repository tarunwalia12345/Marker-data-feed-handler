#pragma once

#include <vector>
#include <cstddef>
#include "protocol.h"
#include "cache.h"

struct ParseResult {
    bool ok;
};

class Parser {
public:
    explicit Parser(Cache& cache);

    // Feed raw TCP data (can be fragmented)
    void feed(const char* data, size_t len);

private:
    // Parse a single complete message
    ParseResult parse(const char* data, size_t len);

private:
    Cache& cache;
    std::vector<char> buffer;
};