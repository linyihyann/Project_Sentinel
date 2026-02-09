cmake_minimum_required(VERSION 3.12)
project(UnitTests C)
set(CMAKE_C_STANDARD 11)
enable_testing()

# 設定 Unity
set(UNITY_SRC ../lib/unity/src/unity.c)
set(UNITY_INCLUDE ../lib/unity/src)

# --- 測試 1: Core Logic ---
add_executable(run_tests
    test_sentinel.c
    ${UNITY_SRC}
    ../src/app/sentinel_core.c
)
target_include_directories(run_tests PRIVATE
    ${UNITY_INCLUDE}
    ../src/app
)
add_test(NAME SentinelCoreTest COMMAND run_tests)

# --- 測試 2: DMA Driver (Day 6) ---
add_executable(test_dma
    test_hal_uart_dma.c
    ${UNITY_SRC}
    # 注意：不要加入 ../src/hal/hal_uart_dma.c，因為 test_hal_uart_dma.c 裡面已經 include 了
)

target_include_directories(test_dma PRIVATE
    ${UNITY_INCLUDE}
    ../src
    # ✅【關鍵修正】這裡必須跟步驟 1 建立的資料夾名稱一模一樣！
    ${CMAKE_CURRENT_SOURCE_DIR}/mock_headers 
)

add_test(NAME DmaDriverTest COMMAND test_dma)