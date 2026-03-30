cmake_minimum_required(VERSION 3.16)
project(market_data LANGUAGES CXX)

# ================= GLOBAL CONFIG =================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(COMMON_WARNINGS -Wall -Wextra -Wshadow -Wconversion -Wpedantic)

find_package(Threads REQUIRED)

# ================= COMMON LIB =================
add_library(common STATIC
        src/common/cache.cpp
        src/common/latency_tracker.cpp
        src/common/memory_pool.cpp
)

target_include_directories(common PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_options(common PRIVATE ${COMMON_WARNINGS})
target_link_libraries(common PUBLIC Threads::Threads)

# ================= SERVER =================
add_executable(server
        src/server/main.cpp
        src/server/exchange_simulator.cpp
        src/server/tick_generator.cpp
        src/server/client_manager.cpp
)

target_link_libraries(server PRIVATE common Threads::Threads)
target_compile_options(server PRIVATE ${COMMON_WARNINGS})

# ================= CLIENT =================
add_executable(client
        src/client/feed_handler.cpp
        src/client/parser.cpp
        src/client/visualizer.cpp
        src/client/market_socket.cpp
)

target_link_libraries(client PRIVATE common Threads::Threads)
target_compile_options(client PRIVATE ${COMMON_WARNINGS})

# ================= BENCHMARKS =================
add_executable(latency_benchmark benchmarks/latency_benchmark.cpp)

add_executable(parser_benchmark
        benchmarks/parser_benchmark.cpp
        src/client/parser.cpp
)

add_executable(cache_benchmark benchmarks/cache_benchmark.cpp)

foreach(b IN ITEMS latency_benchmark parser_benchmark cache_benchmark)
    target_link_libraries(${b} PRIVATE common Threads::Threads)
    target_compile_options(${b} PRIVATE ${COMMON_WARNINGS})
endforeach()

# ================= TESTING =================
enable_testing()

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_cache tests/test_cache.cpp)

add_executable(test_parser
        tests/test_parser.cpp
        src/client/parser.cpp
)

add_executable(test_latency tests/test_latency.cpp)

foreach(t IN ITEMS test_cache test_parser test_latency)
    target_link_libraries(${t} PRIVATE
            common
            GTest::gtest
            GTest::gtest_main
            Threads::Threads
    )
    target_compile_options(${t} PRIVATE ${COMMON_WARNINGS})
endforeach()

add_test(NAME cache_test COMMAND test_cache)
add_test(NAME parser_test COMMAND test_parser)
add_test(NAME latency_test COMMAND test_latency)

# ================= SANITIZERS (DEBUG ONLY) =================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")

    foreach(target common server client latency_benchmark parser_benchmark cache_benchmark)
        target_compile_options(${target} PRIVATE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )

        target_link_options(${target} PRIVATE
                -fsanitize=address,undefined
        )
    endforeach()

endif()

# ================= LINUX OPTIMIZATION =================
if(UNIX AND NOT APPLE)
    target_link_libraries(server PRIVATE rt)
    target_link_libraries(client PRIVATE rt)
endif()

# ================= RELEASE OPTIMIZATION =================
if(CMAKE_BUILD_TYPE STREQUAL "Release")

    # Proper LTO (best practice)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

    foreach(target common server client latency_benchmark parser_benchmark cache_benchmark)
        target_compile_options(${target} PRIVATE
                -O3
                -march=native
        )
    endforeach()

endif()

# ================= BUILD INFO =================
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")