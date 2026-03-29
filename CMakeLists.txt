cmake_minimum_required(VERSION 3.16)
project(market_data)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -march=native -pthread")

# ================= COMMON LIB =================
add_library(common
        src/common/cache.cpp
        src/common/memory_pool.cpp
        src/common/latency_tracker.cpp
)

target_include_directories(common PUBLIC
        include
        src/common
)

# ================= SERVER =================
add_executable(server
        src/server/main.cpp
        src/server/exchange_simulator.cpp
        src/server/tick_generator.cpp
        src/server/client_manager.cpp
)

target_include_directories(server PRIVATE
        include
        src/server
)

target_link_libraries(server common)

# ================= CLIENT =================
add_executable(client
        src/client/feed_handler.cpp
        src/client/parser.cpp
        src/client/visualizer.cpp
        src/client/market_socket.cpp
)

target_include_directories(client PRIVATE
        include
        src/client
)

target_link_libraries(client common)

# ================= BENCHMARK =================
add_executable(benchmark
        benchmarks/latency_benchmark.cpp
)

target_include_directories(benchmark PRIVATE
        include
)

target_link_libraries(benchmark common)