cmake_minimum_required(VERSION 3.10)
project(Burymetal LANGUAGES C)

# --- Опции компиляции ---
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Включаем строгие предупреждения
add_compile_options(-Wall -Wextra -Wpedantic)

# --- Пути к заголовкам ---
include_directories(${CMAKE_SOURCE_DIR}/include)

# --- Исходники библиотеки ---
set(BM_SOURCES
    src/bm_device.c
    src/bm_buffer.c
    src/bm_kernel.c
    src/bm_backend_cpu.c
    src/bm_backend_nvidia.c
    src/bm_backend_amd.c
    src/bm_backend_intel.c
    src/bm_utils.c
)

# --- Статическая библиотека ---
add_library(burymetal STATIC ${BM_SOURCES})

# Можно добавить алиас для удобства
add_library(Burymetal::burymetal ALIAS burymetal)

# --- Примеры ---
file(GLOB EXAMPLE_SOURCES examples/*.c)
foreach(EXAMPLE_SRC ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
    target_link_libraries(${EXAMPLE_NAME} Burymetal::burymetal)
endforeach()

# --- Основное приложение ---
add_executable(myapp MyApp/main.c)
target_link_libraries(myapp Burymetal::burymetal)

# --- Тесты ---
enable_testing()
file(GLOB TEST_SOURCES tests/*.c)
foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} Burymetal::burymetal)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
