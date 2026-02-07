cmake_minimum_required(VERSION 3.12)

# 專案名稱
project(UnitTests C)

# 設定 C 標準
set(CMAKE_C_STANDARD 11)

# 【關鍵修正 1】將 Unity 和 待測程式碼 (sentinel_core.c) 加入編譯列表
add_executable(run_tests
    test_sentinel.c
    ../lib/unity/src/unity.c
    ../src/app/sentinel_core.c   # <--- 之前就是少了這一行！
)

# 【關鍵修正 2】設定標頭檔 (.h) 的搜尋路徑
target_include_directories(run_tests PRIVATE
    ../lib/unity/src
    ../src/app                   # 讓編譯器找得到 sentinel_core.h
)

# 啟用測試功能
enable_testing()
add_test(NAME SentinelCoreTest COMMAND run_tests)