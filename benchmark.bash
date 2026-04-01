#!/bin/bash

# 确保脚本在出错时停止
set -e

# 获取项目根目录绝对路径
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BIN_DIR="$PROJECT_ROOT/bin"

echo "======================================"
echo "   PhotonRPC Benchmark 自动化测试脚本"
echo "======================================"

# 1. 构建项目 (确保使用 Release 模式以获得准确性能数据)
echo "[1/3] 正在构建项目 (Release 模式)..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# 2. 验证可执行文件
if [[ ! -f "$BIN_DIR/TestProvider" || ! -f "$BIN_DIR/Benchmark" ]]; then
    echo "错误: 在 $BIN_DIR 未找到 TestProvider 或 Benchmark。"
    exit 1
fi

# 3. 运行测试
echo "[2/3] 准备启动服务端并运行 Benchmark..."

# 定义清理函数
cleanup() {
    if [ ! -z "$PROVIDER_PID" ]; then
        echo "正在关闭 TestProvider (PID: $PROVIDER_PID)..."
        kill $PROVIDER_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

# 启动 Provider (后台运行)
echo "-> 正在后台启动 TestProvider..."
"$BIN_DIR/TestProvider" > /dev/null 2>&1 &
PROVIDER_PID=$!

# 等待 1 秒确保服务端已准备就绪
sleep 1

# 启动 Benchmark (前台运行)
echo "-> 正在启动 Benchmark 测试..."
echo "--------------------------------------"
"$BIN_DIR/Benchmark"
echo "--------------------------------------"

echo "[3/3] 测试完成。"
echo "======================================"
