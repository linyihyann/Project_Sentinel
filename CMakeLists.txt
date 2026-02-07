cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)

# Board and platform MUST be set before pico_sdk_import.cmake (SDK 2.0.0+)
set(PICO_BOARD pico2_w)

# 禁用 picotool 自動下載 (Docker 環境不需要) ← 重要！解決你的錯誤
set(PICOTOOL_FETCH_FROM_GIT_PATH OFF CACHE BOOL "Disable picotool in Docker")

include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)

# 改這裡 ↓
project(project_sentinel C CXX ASM)
pico_sdk_init()

# 改這裡 ↓
add_executable(project_sentinel
    src/main.c
    src/app/sentinel_core.c
    src/hal/hal_led.c
)

# 改這裡 ↓
target_include_directories(project_sentinel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 改這裡 ↓
target_link_libraries(project_sentinel
    pico_stdlib
    pico_cyw43_arch_none
)

# 改這裡 ↓
pico_enable_stdio_usb(project_sentinel 1)
pico_add_extra_outputs(project_sentinel)