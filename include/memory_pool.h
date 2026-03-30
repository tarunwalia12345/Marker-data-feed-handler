#pragma once
#include <vector>
#include <cstddef>
#include <mutex>

class MemoryPool {
public:
    MemoryPool(size_t block_size, size_t pool_size);

    void *allocate();

    void deallocate(void *ptr);

    ~MemoryPool();

private:
    size_t block_size;
    std::vector<void *> free_list;
    std::mutex m;
};
