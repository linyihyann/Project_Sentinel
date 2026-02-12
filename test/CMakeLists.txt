cmake_minimum_required(VERSION 3.12)
project(UnitTests C)
set(CMAKE_C_STANDARD 11)
enable_testing()

# ==========================================
# 1. 設定 Unity 路徑 (全域變數)
# ==========================================
# 確保這個路徑指向你下載 unity.c 的地方 (test/unity)
set(UNITY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../lib/unity/src/unity.c)
set(UNITY_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/unity/src)

# ==========================================
# 2. 測試目標 1: Sentinel Core (App Logic)
# ==========================================
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

# ==========================================
# 3. 測試目標 2: Ring Buffer (Day 7 新增) ✅
# ==========================================
add_executable(test_ring_buffer
    test_ring_buffer.c
    ${UNITY_SRC}
    ../src/common/ring_buffer.c
)
target_include_directories(test_ring_buffer PRIVATE
    ${UNITY_INCLUDE}
    ../src/common
)
add_test(NAME RingBufferTest COMMAND test_ring_buffer)

# ==========================================
# 4. 測試目標 3: HAL UART (Day 8 Observer Pattern) ✅ NEW!
# ==========================================
# 這裡將 test_hal_uart.c 與實作 hal_uart.c 連結
add_executable(test_hal_uart
    test_hal_uart.c
    ${UNITY_SRC}
    ../src/hal/hal_uart.c
)

# 設定 Include Path，讓 test_hal_uart.c 能找到 "hal_uart.h"
target_include_directories(test_hal_uart PRIVATE
    ${UNITY_INCLUDE}
    ../src/hal
)

# 註冊測試案例到 CTest
add_test(NAME HalUartTest COMMAND test_hal_uart)

# ==========================================
# 5. 測試目標 3: DMA Driver (Day 6) 🛠️ 修正處
# ==========================================
#add_executable(test_dma
#    test_hal_uart_dma.c
 #   ${UNITY_SRC}
    # 注意：不要加入 ../src/hal/hal_uart_dma.c，因為 test 檔裡已經 include 了 .c
#)

#target_include_directories(test_dma PRIVATE
 #   ${UNITY_INCLUDE}                       # <--- [關鍵修正] 之前這裡漏了或寫錯了，導致找不到 unity.h
  #  ../src
   # ${CMAKE_CURRENT_SOURCE_DIR}/mock_headers 
#)

#add_test(NAME DmaDriverTest COMMAND test_dma)