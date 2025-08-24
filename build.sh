#!/bin/bash

# =============================================================================
# Perception App 统一构建脚本
# =============================================================================

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# 默认配置
BUILD_TYPE="Release"
BUILD_DIR="output/build"
CLEAN_BUILD=false
INSTALL_PREFIX="output/install"

# 显示帮助信息
show_help() {
    echo -e "${BOLD}Perception App 构建脚本${NC}"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help              显示此帮助信息"
    echo "  -c, --clean             清理构建目录后全量编译"
    echo ""
    echo "示例:"
    echo "  $0 -c                    # 清理构建目录后全量编译"
    echo "  $0                       # 增量编译 (默认)"
    echo ""
}

# 解析命令行参数
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            *)
                echo -e "${RED}错误: 未知选项 $1${NC}"
                show_help
                exit 1
                ;;
        esac
    done
}

# 检查依赖
check_dependencies() {
    echo -e "${BOLD}${BLUE}🔍 检查依赖...${NC}"
    
    # 检查 CMake
    if ! command -v cmake &> /dev/null; then
        echo -e "${RED}错误: CMake 未找到，请先安装 CMake${NC}"
        exit 1
    fi
    
    # 检查编译器
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        echo -e "${RED}错误: C++ 编译器未找到，请安装 g++ 或 clang++${NC}"
        exit 1
    fi

    export PKG_CONFIG_PATH="$PWD/thirdparty/lib/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="$PWD/thirdparty/lib:$LD_LIBRARY_PATH"
    # 检查 MechEyeApi
    if ! pkg-config --exists MechEyeApi; then
        echo -e "${RED}错误: MechEyeApi 未找到，请先安装 MechEyeApi${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ 依赖检查通过${NC}"
}

# 清理构建目录
clean_build() {
    if [ "$CLEAN_BUILD" = true ]; then
        echo -e "${BOLD}${YELLOW}🧹 清理构建目录...${NC}"
        if [ -d "$BUILD_DIR" ]; then
            rm -rf "$BUILD_DIR"
            echo -e "${GREEN}✓ 构建目录已清理${NC}"
        fi
    else
        echo -e "${CYAN}📝 使用增量编译模式${NC}"
    fi
}

# 创建构建目录
create_build_dir() {
    echo -e "${BOLD}${BLUE}📁 准备构建目录...${NC}"
    
    # 检查是否在根目录，如果是则进入 perception_app 子目录
    if [ -d "perception_app" ] && [ -f "perception_app/CMakeLists.txt" ]; then
        echo -e "${CYAN}📁 进入 perception_app 目录${NC}"
        cd perception_app
    elif [ ! -f "CMakeLists.txt" ]; then
        echo -e "${RED}错误: 找不到 CMakeLists.txt 文件${NC}"
        echo -e "${RED}请确保在 mech-mind 根目录或 perception_app 目录中运行此脚本${NC}"
        exit 1
    fi
    
    # 创建构建目录（相对于脚本所在位置）
    mkdir -p "../$BUILD_DIR"
    cd "../$BUILD_DIR"
}

# 配置 CMake
configure_cmake() {
    echo -e "${BOLD}${BLUE}⚙️  配置 CMake...${NC}"
    
    # 检查是否需要重新配置
    if [ "$CLEAN_BUILD" = true ] || [ ! -f "CMakeCache.txt" ]; then
        # 构建 CMake 命令（使用 CMakeLists.txt 中的默认选项）
        CMAKE_CMD="cmake ../../perception_app"
        CMAKE_CMD="$CMAKE_CMD -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        CMAKE_CMD="$CMAKE_CMD -DCMAKE_INSTALL_PREFIX=../../output/install"
        
        echo -e "${CYAN}执行: $CMAKE_CMD${NC}"
        
        if ! eval $CMAKE_CMD; then
            echo -e "${RED}❌ CMake 配置失败${NC}"
            exit 1
        fi
        
        echo -e "${GREEN}✓ CMake 配置成功${NC}"
    else
        echo -e "${CYAN}📝 使用现有 CMake 配置${NC}"
    fi
}

# 编译项目
build_project() {
    echo -e "${BOLD}${BLUE}🔨 编译项目...${NC}"
    
    # 确定并行编译的线程数
    if command -v nproc &> /dev/null; then
        JOBS=$(nproc)
    else
        JOBS=4
    fi
    
    BUILD_CMD="make -j$JOBS"
    echo -e "${CYAN}执行: $BUILD_CMD${NC}"
    
    if ! eval $BUILD_CMD; then
        echo -e "${RED}❌ 编译失败${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ 编译成功${NC}"
}

# 安装项目
install_project() {
    echo -e "${BOLD}${BLUE}📦 安装项目...${NC}"
    
    if ! make install; then
        echo -e "${RED}❌ 安装失败${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ 安装成功${NC}"
}

# 显示构建信息
show_build_info() {
    echo -e "${BOLD}${GREEN}🎉 构建完成！${NC}"
    echo ""
    echo -e "${BOLD}构建信息:${NC}"
    echo -e "  构建目录: ${CYAN}$BUILD_DIR${NC}"
    echo -e "  构建模式: ${CYAN}$([ "$CLEAN_BUILD" = true ] && echo "全量编译" || echo "增量编译")${NC}"
    echo -e "  安装目录: ${CYAN}$INSTALL_PREFIX${NC}"
    
    echo ""
    echo -e "${BOLD}可执行文件位置:${NC}"
    echo -e "  主程序: ${CYAN}$BUILD_DIR/binary/perception_app${NC}"
    echo -e "  示例程序: ${CYAN}$BUILD_DIR/binary/${NC}"
    
    echo ""
    echo -e "${BOLD}库文件位置:${NC}"
    echo -e "  静态库: ${CYAN}$BUILD_DIR/lib/${NC}"
}

# 主函数
main() {
    echo -e "${BOLD}${BLUE}🚀 Perception App 构建脚本${NC}"
    echo ""
    
    # 解析参数
    parse_args "$@"
    
    # 显示配置
    echo -e "${BOLD}构建配置:${NC}"
    echo -e "  构建目录: $BUILD_DIR"
    echo -e "  构建模式: $([ "$CLEAN_BUILD" = true ] && echo "全量编译" || echo "增量编译")"
    echo -e "  安装目录: $INSTALL_PREFIX"
    echo ""
    
    # 检查依赖
    check_dependencies
    
    # 清理构建
    clean_build
    
    # 创建构建目录
    create_build_dir
    
    # 配置 CMake
    configure_cmake
    
    # 编译项目
    build_project
    
    # 安装项目
    install_project
    
    # 显示构建信息
    show_build_info
}

# 执行主函数
main "$@" 