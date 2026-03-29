#include <vector>

class MemoryPool {
    std::vector<char> buffer;
    size_t offset = 0;

public:
    MemoryPool(size_t size) : buffer(size) {}

    void* alloc(size_t size) {
        if (offset + size > buffer.size()) return nullptr;
        void* ptr = buffer.data() + offset;
        offset += size;
        return ptr;
    }

    void reset() { offset = 0; }
};