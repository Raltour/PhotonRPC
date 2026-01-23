#!/bin/bash

# 确保脚本在出错时停止
set -e

# 获取项目根目录绝对路径
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BIN_DIR="$PROJECT_ROOT/bin"

echo "======================================"
echo "   PhotonRPC 一键构建与运行脚本"
echo "======================================"

# 1. 环境准备
echo "[1/3] 正在配置 CMake (使用 WSL GCC 工具链)..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 2. 执行构建
# 指定使用 Debug 模式并并行编译
cmake -DCMAKE_BUILD_TYPE=Release ..
echo "[2/3] 正在编译项目..."
make -j$(nproc)

# 3. 验证并运行
echo "[3/3] 准备启动测试程序..."

# 检查可执行文件是否生成
if [[ ! -f "$BIN_DIR/TestProvider" || ! -f "$BIN_DIR/TestConsumer" ]]; then
    echo "错误: 在 $BIN_DIR 未找到 TestProvider 或 TestConsumer。"
    exit 1
fi

# 定义清理函数：当脚本退出或被中断时，自动关闭后台的 Provider
cleanup() {
    echo ""
    if [ ! -z "$PROVIDER_PID" ]; then
        echo "正在关闭 TestProvider (PID: $PROVIDER_PID)..."
        kill $PROVIDER_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

# 启动 Provider (后台运行)
echo "-> 正在启动 TestProvider..."
"$BIN_DIR/TestProvider" &
PROVIDER_PID=$!

# 等待 1 秒确保服务端已监听端口
sleep 1

# 启动 Consumer (前台运行)
echo "-> 正在启动 TestConsumer..."
"$BIN_DIR/TestConsumer"

echo "======================================"
echo "   所有任务已执行完毕"
echo "======================================"