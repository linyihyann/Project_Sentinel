#!/bin/bash
set -e  # 遇到錯誤立即停止

# --- 1. 初始化變數 ---
BUILD_TYPE="Debug"
TARGET="firmware" 
DO_CLEAN=0        

# --- 2. 參數解析 ---
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--test) TARGET="test" ;;
        -c|--clean) DO_CLEAN=1 ;;
        -r|--release) BUILD_TYPE="Release" ;;
        *) echo "Unknown parameter: $1"; exit 1 ;;
    esac
    shift
done

# --- 3. 執行清除 ---
if [ $DO_CLEAN -eq 1 ]; then
    echo "🧹 Cleaning up build artifacts..."
    rm -rf build build_test
    echo "✅ Clean complete."
fi

# 取得 CPU 核心數 (相容 Mac/Linux)
JOBS=$(sysctl -n hw.ncpu 2>/dev/null || nproc || echo 4)

# --- 4. 執行編譯 ---
if [ "$TARGET" == "test" ]; then
    # === 測試模式 (Host Build) ===
    echo "🧪 Building and Running Unit Tests..."
    
    mkdir -p build_test
    cd build_test
    
    # 【關鍵修正】：指向 ../test 而不是 ..
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../test
    make -j$JOBS
    
    echo "🚀 Running Tests..."
    ./run_tests
    
else
    # === 韌體模式 (Target Build) ===
    echo "🔨 Building Firmware for RP2350..."
    
    mkdir -p build
    cd build
    
    # 這裡指向根目錄 (..) 是正確的，因為韌體設定在根目錄
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    make -j$JOBS
    
    echo "✅ Firmware built successfully: build/project_sentinel.uf2"
fi