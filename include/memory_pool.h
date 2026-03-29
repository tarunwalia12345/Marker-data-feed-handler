/*#pragma once
#include <vector>
#include <cstddef>

class MemoryPool {
    std::vector<char> buffer;
    size_t offset;

public:
    explicit MemoryPool(size_t size);

    void* allocate(size_t size);
    void reset();

    size_t capacity() const;
    size_t used() const;
};*/