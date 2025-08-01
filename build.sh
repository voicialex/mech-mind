#!/bin/bash

# Mech-Eye SDK 构建脚本
# 解决 pkg-config 无法找到 MechEyeApi 和 LZ4 链接问题

set -e

# 配置开关 (可修改)
ENABLE_PCL=true      # 是否启用 PCL 功能
ENABLE_HALCON=false  # 是否启用 Halcon 功能
BUILD_TYPE="Release" # 构建类型: Debug/Release

# 获取脚本目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="$SCRIPT_DIR"

# 设置输出目录
OUTPUT_DIR="$SDK_ROOT/output"
BUILD_DIR="$OUTPUT_DIR/build"
INSTALL_DIR="$OUTPUT_DIR/install"

echo "=== Mech-Eye SDK 构建脚本 ==="
echo "SDK 目录: $SDK_ROOT"
echo "输出目录: $OUTPUT_DIR"
echo "构建目录: $BUILD_DIR"
echo "安装目录: $INSTALL_DIR"
echo "PCL 支持: $ENABLE_PCL"
echo "Halcon 支持: $ENABLE_HALCON"
echo "构建类型: $BUILD_TYPE"

# 检查必要文件
if [ ! -f "$SDK_ROOT/lib/pkgconfig/MechEyeApi.pc" ]; then
    echo "错误: 找不到 MechEyeApi.pc 文件"
    exit 1
fi

# 设置环境变量
export PKG_CONFIG_PATH="$SDK_ROOT/lib/pkgconfig:$PKG_CONFIG_PATH"
export LD_LIBRARY_PATH="$SDK_ROOT/lib:$LD_LIBRARY_PATH"

# 验证 MechEyeApi
echo "验证 MechEyeApi..."
if ! pkg-config --exists MechEyeApi; then
    echo "错误: MechEyeApi 包配置未找到"
    exit 1
fi
echo "✓ MechEyeApi 版本: $(pkg-config --modversion MechEyeApi)"

# 检查 LZ4 版本
LZ4_VERSION=$(pkg-config --modversion liblz4 2>/dev/null || echo "unknown")
echo "系统 LZ4 版本: $LZ4_VERSION"

# 根据配置设置编译参数
if [[ "$ENABLE_PCL" == "true" ]]; then
    USE_PCL="ON"
    echo "启用 PCL 功能"
else
    USE_PCL="OFF"
    echo "禁用 PCL 功能"
fi

if [[ "$ENABLE_HALCON" == "true" ]]; then
    USE_HALCON="ON"
    echo "启用 Halcon 功能"
else
    USE_HALCON="OFF"
    echo "禁用 Halcon 功能"
fi

# 创建输出目录结构
echo "创建输出目录..."
mkdir -p "$BUILD_DIR"
mkdir -p "$INSTALL_DIR"

# 清理并创建构建目录
if [ -d "$BUILD_DIR" ]; then
    echo "清理构建目录: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置 CMake
echo "配置 CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    -DUSE_OPENCV=ON
    -DUSE_PCL=$USE_PCL
    -DUSE_HALCON=$USE_HALCON
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
)

# 如果启用 PCL，添加 LZ4 链接参数
if [[ "$USE_PCL" == "ON" ]]; then
    CMAKE_ARGS+=(
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--no-as-needed -llz4"
        -DCMAKE_SHARED_LINKER_FLAGS="-Wl,--no-as-needed -llz4"
    )
fi

# 使用绝对路径指向源码目录
SOURCE_DIR="$SDK_ROOT/samples/cpp/area_scan_3d_camera"
cmake "$SOURCE_DIR" "${CMAKE_ARGS[@]}"

# 编译
echo "开始编译..."
make -j$(nproc)

# 手动安装到指定目录
echo "开始安装..."
echo "复制可执行文件到安装目录..."

# 创建安装目录结构
mkdir -p "$INSTALL_DIR"

# 复制所有可执行文件
find . -name "*" -type f -executable | grep -v CMake | while read -r file; do
    if [ -f "$file" ] && [ -x "$file" ]; then
        echo "安装: $file"
        cp "$file" "$INSTALL_DIR/"
    fi
done

# 显示结果
echo "✓ 编译完成！"
echo "构建目录: $BUILD_DIR"
echo "安装目录: $INSTALL_DIR"
echo "编译的示例:"
find . -name "*" -type f -executable | grep -v CMake | sort

echo "安装的示例:"
find "$INSTALL_DIR" -name "*" -type f -executable 2>/dev/null | sort

# 显示说明
if [[ "$USE_PCL" == "OFF" ]]; then
    echo ""
    echo "注意: 已禁用 PCL 功能"
    echo "如需启用，请修改脚本中的 ENABLE_PCL=true"
fi

echo ""
echo "=== 构建完成 ==="
echo "所有文件已安装到: $INSTALL_DIR" 