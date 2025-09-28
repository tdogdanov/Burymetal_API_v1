Burymetal_API/
├── include/burymetal/          # Публичные заголовки
│   ├── burymetal.h             # Основной API
│   ├── bm_types.h              # Типы данных (BMDevice, BMBuffer, BMKernel)
│   ├── bm_rall.h               # RAII / безопасное управление ресурсами
│   ├── bm_backend.h            # Интерфейс backend’ов
│   └── bm_utils.h              # Вспомогательные функции
│
├── src/core/                   # Ядро API
│   ├── bm_device.c
│   ├── bm_buffer.c
│   ├── bm_kernel.c
│   └── bm_utils.c
│
├── src/backends/               # Реализации backend’ов
│   ├── bm_backend_cpu.c
│   ├── bm_backend_nvidia.c
│   ├── bm_backend_amd.c
│   └── bm_backend_intel.c
│
├── examples/                   # Примеры
│   ├── simple_compute.c
│   ├── bm_rall.c
│   ├── buffer_test.c
│   ├── matmul_ai.c
│   └── main.c
│
├── tests/                      # Unit-тесты
│   ├── test_device.c
│   ├── test_buffer.c
│   ├── test_kernel.c
│   ├── test_burymetal.c 
│   ├── test_mem_pool.c     // Тесты пула буферов
│   └── test_mem_utils.c    // Тесты вспомогательных функций copy, zero, align
│
├── memory/
│    ├── bm_mem_alloc.c       # CPU/GPU выделение памяти
│    ├── bm_mem_alloc.h       # Заголовок для bm_mem_alloc.c
│    ├── bm_mem_pool.c        # Пулы буферов для CPU/GPU
│    ├── bm_mem_pool.h
│    ├── bm_mem_utils.c       # Вспомогательные функции: copy, zero, align
│    └── bm_mem_utils.h
│
├── docs/                        # Документация
│   └── API.md
│
├── scripts/                     # Скрипты сборки
│   └── build.sh
│
├── cmake/                        # CMake модули
│   └── FindBurymetal.cmake
│
├── .gitignore
├── CMakeLists.txt
├── Makefile
└── README.md



------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

myapp/
├── include/
│   └── myapp/
│       └── myapp.h         # заголовки с API твоего приложения
│
├── src/
│   ├── main.c              # точка входа
│   └── myapp_core.c        # внутренняя логика
│
├── tests/                  # тесты конкретно для приложения
│   └── test_myapp.c
│
├── CMakeLists.txt
├── Makefile
├── README.md
└── .gitignore
