# Компилятор и флаги
CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -Wpedantic -O2 -std=c11 -Iinclude
AR      ?= ar
ARFLAGS ?= rcs

# Директории
SRC_DIR     = src
EXAMPLES_DIR= examples
TESTS_DIR   = tests
BUILD_DIR   = build

# Исходники библиотеки
SRC = $(SRC_DIR)/bm_device.c \
      $(SRC_DIR)/bm_buffer.c \
      $(SRC_DIR)/bm_kernel.c \
      $(SRC_DIR)/bm_backend_cpu.c \
      $(SRC_DIR)/bm_backend_nvidia.c \
      $(SRC_DIR)/bm_backend_amd.c \
      $(SRC_DIR)/bm_backend_intel.c \
      $(SRC_DIR)/bm_utils.c

OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
LIB = $(BUILD_DIR)/libburymetal.a

# Примеры и тесты
EXAMPLES = $(BUILD_DIR)/examples/simple_compute \
           $(BUILD_DIR)/examples/buffer_test

TESTS    = $(BUILD_DIR)/tests/test_device \
           $(BUILD_DIR)/tests/test_buffer \
           $(BUILD_DIR)/tests/test_kernel

# --- Сборка всех объектов ---
all: $(LIB) $(EXAMPLES) $(TESTS)

# --- Компиляция исходников в объектные файлы ---
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Создание статической библиотеки ---
$(LIB): $(OBJ)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^

# --- Сборка примеров ---
$(BUILD_DIR)/examples/%: $(EXAMPLES_DIR)/%.c $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lburymetal -o $@

# --- Сборка тестов ---
$(BUILD_DIR)/tests/%: $(TESTS_DIR)/%.c $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lburymetal -o $@

# --- Запуск всех тестов ---
test: $(TESTS)
	@for t in $(TESTS); do echo "==> Running $$t"; $$t || exit 1; done

# --- Очистка сборки ---
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test
