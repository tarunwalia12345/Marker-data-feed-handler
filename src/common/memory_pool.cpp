#include "memory_pool.h"
#include <cstdlib>

MemoryPool::MemoryPool(size_t bsize, size_t pool_size)
    : block_size(bsize) {

    free_list.reserve(pool_size);

    for (size_t i = 0; i < pool_size; i++) {
        free_list.push_back(std::malloc(block_size));
    }
}

void* MemoryPool::allocate() {
    std::lock_guard<std::mutex> lock(m);

    if (!free_list.empty()) {
        void* ptr = free_list.back();
        free_list.pop_back();
        return ptr;
    }

    return std::malloc(block_size);
}

void MemoryPool::deallocate(void* ptr) {
    std::lock_guard<std::mutex> lock(m);
    free_list.push_back(ptr);
}

MemoryPool::~MemoryPool() {
    for (void* ptr : free_list) {
        std::free(ptr);
    }
}