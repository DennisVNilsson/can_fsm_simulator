cmake_minimum_required(VERSION 3.20)
project(can_fsm_simulator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror -g -O0)

# ── FSM library ───────────────────────────────────────────────────────────
add_library(fsm
    src/fsm.cpp
)
target_include_directories(fsm PUBLIC include third_party/emalloc)
add_library(can_bus
    src/can_bus.cpp
)
target_include_directories(can_bus PUBLIC include third_party/emalloc)
add_library(nodes
    src/sensor_node.cpp
)
target_include_directories(nodes PUBLIC include third_party/emalloc)
target_link_libraries(nodes PRIVATE fsm can_bus)

# ── FSM unit tests ────────────────────────────────────────────────────────
add_executable(tests_fsm
    tests/test_fsm.cpp
)
target_link_libraries(tests_fsm PRIVATE fsm)
add_executable(tests_sensor_node
    tests/test_sensor_node.cpp
)
target_link_libraries(tests_sensor_node PRIVATE nodes)

# ── CTest ─────────────────────────────────────────────────────────────────
enable_testing()
add_test(NAME fsm_tests COMMAND tests_fsm)
add_test(NAME sensor_node_tests COMMAND tests_sensor_node)