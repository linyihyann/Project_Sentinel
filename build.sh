#!/bin/bash

# 遇到錯誤立即停止，這是 DevOps 的基本修養
set -e

IMAGE_NAME="project_sentinel_builder"
DOCKERFILE_PATH="$(pwd)/Dockerfile"
BUILD_DIR="build"
TEST_DIR="test"
TEST_BUILD_DIR="test/build"

# 初始化變數
FORCE_RECONFIGURE=0
CLEAN_BUILD=0
VERBOSE=0
RUN_TESTS=0

# -----------------------------------------------------------------------------
# 1. 參數解析 (Argument Parsing)
# -----------------------------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--test)
            RUN_TESTS=1
            shift
            ;;
        -r|--reconfigure)
            FORCE_RECONFIGURE=1
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -h|--help)
            echo "Usage: ./build.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -t, --test           Run Unit Tests (TDD Mode) on Host"
            echo "  -r, --reconfigure    Force CMake reconfiguration"
            echo "  -c, --clean          Clean build directory before building"
            echo "  -v, --verbose        Verbose build output"
            echo "  -h, --help           Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# -----------------------------------------------------------------------------
# 2. Docker 環境檢查 (Environment Check)
# -----------------------------------------------------------------------------
if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
    echo "🔨 Image '${IMAGE_NAME}' not found. Building Docker image..."
    docker build -t ${IMAGE_NAME} -f ${DOCKERFILE_PATH} .
    echo "✅ Docker image '${IMAGE_NAME}' built successfully."
else
    echo "✅ Docker image '${IMAGE_NAME}' ready."
fi

# -----------------------------------------------------------------------------
# 3. 執行單元測試 (TDD Mode)
# -----------------------------------------------------------------------------
if [[ $RUN_TESTS -eq 1 ]]; then
    echo ""
    echo "🧪 =========================================="
    echo "🧪   Running Unit Tests (Host-Based TDD)    "
    echo "🧪 =========================================="
    
    # 清理測試建置 (如果需要)
    if [[ $CLEAN_BUILD -eq 1 ]]; then
        echo "🧹 Cleaning test build directory..."
        rm -rf ${TEST_BUILD_DIR}
    fi

    # 在 Docker 中執行測試
    # 注意：這裡使用 gcc (Native) 而非 arm-none-eabi-gcc
    docker run --rm \
        -v "$(pwd):/workspace" \
        -w /workspace/${TEST_DIR} \
        ${IMAGE_NAME} \
        bash -c "mkdir -p build && cd build && cmake .. && make && ./run_tests"
    
    echo ""
    echo "✅ All Tests Passed! (Project Sentinel Logic Verified)"
    exit 0
fi

# -----------------------------------------------------------------------------
# 4. 執行韌體編譯 (Firmware Build Mode)
# -----------------------------------------------------------------------------
echo ""
echo "🚀 =========================================="
echo "🚀   Building Firmware for Pico 2 W (ARM)    "
echo "🚀 =========================================="

# 清理
if [[ $CLEAN_BUILD -eq 1 ]]; then
    echo "🧹 Cleaning build directory..."
    rm -rf ${BUILD_DIR}
fi

# 判斷是否需要重新執行 CMake
NEED_CMAKE=0
if [[ ! -d "${BUILD_DIR}" ]]; then
    NEED_CMAKE=1
elif [[ $FORCE_RECONFIGURE -eq 1 ]]; then
    NEED_CMAKE=1
elif [[ ! -f "${BUILD_DIR}/build.ninja" ]]; then
    NEED_CMAKE=1
elif [[ "CMakeLists.txt" -nt "${BUILD_DIR}/build.ninja" ]]; then
    echo "🔄 CMakeLists.txt has been modified, re-running CMake."
    NEED_CMAKE=1
fi

# 構建指令
if [[ $NEED_CMAKE -eq 1 ]]; then
    # 加入 -DPICOTOOL_FETCH_FROM_GIT_PATH=OFF 以防萬一
    CMAKE_CMD="cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DPICOTOOL_FETCH_FROM_GIT_PATH=OFF -G Ninja .."
    if [[ $VERBOSE -eq 1 ]]; then
        BUILD_CMD="mkdir -p build && cd build && ${CMAKE_CMD} && ninja -v"
    else
        BUILD_CMD="mkdir -p build && cd build && ${CMAKE_CMD} && ninja"
    fi
else
    if [[ $VERBOSE -eq 1 ]]; then
        BUILD_CMD="cd build && ninja -v"
    else
        BUILD_CMD="cd build && ninja"
    fi
fi

# 在 Docker 中執行編譯
docker run --rm \
    -v "$(pwd):/workspace" \
    ${IMAGE_NAME} \
    bash -c "${BUILD_CMD}"

# 檢查產出
if [[ -f "${BUILD_DIR}/project_sentinel.uf2" ]]; then
    echo ""
    echo "✅ Build completed successfully!"
    echo "📦 Output: ${BUILD_DIR}/project_sentinel.uf2"
else
    echo ""
    echo "❌ Build failed! No .uf2 file found."
    exit 1
fi