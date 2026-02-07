# 檔案位置: test/CMakeLists.txt

cmake_minimum_required(VERSION 3.13)
project(Sentinel_Unit_Tests C)

set(CMAKE_C_STANDARD 11)

# 1. 開啟測試巨集 (讓代碼知道現在是測試模式)
add_definitions(-DUNIT_TESTING)

# 2. 包含路徑
include_directories(
    unity
    ../src/app
    ../src/hal
)

# 3. 定義來源檔案
# 這裡我們只抓 "純邏輯" 檔案 (sentinel_core.c) 與 測試腳本
set(SOURCE_FILES
    test_sentinel.c
    unity/unity.c
    ../src/app/sentinel_core.c
)

# 4. 產生執行檔 (這是跑在 macOS 上的)
add_executable(run_tests ${SOURCE_FILES})

# 5. 連結數學庫 (如果演算法有用到 math.h)
target_link_libraries(run_tests m)