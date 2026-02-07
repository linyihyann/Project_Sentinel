FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. 安裝基礎工具
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    git \
    python3 \
    build-essential \
    wget \
    ca-certificates \
    xxd \
    libnewlib-arm-none-eabi \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# 2. 【關鍵修正】下載 AArch64 (ARM64) 版本的 Toolchain
# 這是專門給 M1/M2/M3 Mac 以及 Linux ARM 主機用的原生編譯器
WORKDIR /toolchain
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz && \
    tar -xf arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz && \
    rm arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz

# 設定 PATH (注意資料夾名稱變成了 aarch64)
ENV PATH="/toolchain/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi/bin:$PATH"

# 3. 下載 SDK (保持不變)
RUN git clone --depth=1 --branch 2.1.0 https://github.com/raspberrypi/pico-sdk.git /pico-sdk && \
    cd /pico-sdk && \
    git submodule update --init && \
    echo "🧹 Cleaning up git history..." && \
    rm -rf .git && \
    find . -name ".git" -type d -exec rm -rf {} +

ENV PICO_SDK_PATH=/pico-sdk
ENV PICO_SDK_FETCH_FROM_GIT_PATH=0
ENV PICO_SDK_NO_VERSION_CHECK=1

WORKDIR /workspace
CMD ["/bin/bash"]