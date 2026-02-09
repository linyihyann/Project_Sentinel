cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)

# Board and platform MUST be set before pico_sdk_import.cmake (SDK 2.0.0+)
set(PICO_BOARD pico2_w)

# 禁用 picotool 自動下載 (Docker 環境不需要)
set(PICOTOOL_FETCH_FROM_GIT_PATH OFF CACHE BOOL "Disable picotool in Docker")

include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)

project(project_sentinel C CXX ASM)
pico_sdk_init()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(project_sentinel
    src/main.c
    src/app/sentinel_core.c
    src/hal/hal_led.c
    src/hal/hal_uart_dma.c  # <--- [NEW] 加入這個檔案
)

target_include_directories(project_sentinel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(project_sentinel
    pico_stdlib
    pico_cyw43_arch_none
    hardware_uart           # <--- [NEW] 必須加入這個
    hardware_dma            # <--- [NEW] 必須加入這個
)

# 啟用 USB (變成虛擬串口)
pico_enable_stdio_usb(project_sentinel 1)

# 禁用 UART (避免污染 Loopback 測試)
pico_enable_stdio_uart(project_sentinel 0)

pico_add_extra_outputs(project_sentinel)