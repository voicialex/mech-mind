# Mech-Eye SDK 构建脚本

## 简介

这个脚本解决了 Mech-Eye SDK 编译时的常见问题：
- pkg-config 无法找到 MechEyeApi
- LZ4 库版本兼容性问题

## 使用方法

```bash
# 直接运行构建脚本
./build.sh
```

## 配置选项

在脚本开头可以修改以下配置：

```bash
# 配置开关
ENABLE_PCL=true      # 是否启用 PCL 功能 (默认: true)
ENABLE_HALCON=false  # 是否启用 Halcon 功能 (默认: false)
BUILD_TYPE="Release" # 构建类型: Debug/Release (默认: Release)
```

## 输出目录结构

编译完成后会生成以下目录结构：

```
mech-eye-sdk/
├── build.sh                    # 构建脚本
├── output/                     # 输出目录
│   ├── build/                  # 编译中间产物
│   └── install/                # 安装目录
│       ├── ConnectToCamera     # 示例程序
│       ├── Capture2DImage      # 示例程序
│       └── ...                 # 其他示例程序
```

## 运行示例

```bash
# 直接运行示例程序
output/install/ConnectToCamera
output/install/Capture2DImage
```

## 注意事项

- 脚本会自动设置正确的环境变量
- 如果遇到 LZ4 兼容性问题，可以设置 `ENABLE_PCL=false`
- 编译过程中可能会有库版本冲突警告，但不影响最终结果
- 所有示例程序都已安装到 `output/install/` 目录 