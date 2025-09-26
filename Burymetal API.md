/home/admin1/Desktop/Burymetal_API/ v1/

Burymetal_API/ v1                   # Библиотека вычислений (API)
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── launch.json
│   ├── tasks.json
│   └── settings.json
│
├── include/                     # Заголовочные файлы API
│   ├── burymetal.h              # Основной публичный заголовок
│   ├── bm_types.h               # Типы данных (BMDevice, BMBuffer, BMKernel)
│   └── bm_backend.h             # Интерфейс для backend (GPU/CPU)
│
├── src/                         # Исходники реализации API
│   ├── bm_device.c              # Инициализация и управление устройствами
│   ├── bm_buffer.c              # Буферы CPU/GPU
│   ├── bm_kernel.c              # Управление вычислительными ядрами
│   ├── bm_backend_cpu.c         # CPU backend
│   ├── bm_backend_nvidia.c      # NVIDIA backend
│   ├── bm_backend_amd.c         # AMD backend
│   ├── bm_backend_intel.c       # Intel GPU backend
│   └── bm_utils.c               # Вспомогательные функции
│
├── examples/                    # Мини-примеры использования API
│   ├── simple_compute.c
│   └── buffer_test.c
│
├── tests/                       # Unit-тесты
│   ├── test_device.c
│   ├── test_burymetal.c
│   ├── test_buffer.c
│   └── test_kernel.c
│
├── build/                       # Каталог сборки (CMake / Make)
├── CMakeLists.txt
├── Makefile
└── README.md

myapp/                           # Приложение, использующее Burymetal
├── include/                     # Заголовочные файлы приложения
│   └── myapp.h
├── src/                         # Исходники приложения
│   └── main.c
├── build/                       # Каталог сборки (CMake / Make)
├── CMakeLists.txt
├── Makefile
└── README.md
