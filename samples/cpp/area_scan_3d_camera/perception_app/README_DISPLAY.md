# 实时显示功能使用说明

## 功能描述

这个感知应用程序现在支持实时显示摄像头采集到的数据，包括：
- **2D彩色图像** - 左侧显示真彩色图像  
- **深度图** - 右侧显示伪彩色深度图（使用JET色彩映射）

## 显示窗口特性

- **窗口标题**: "Mech-Eye Real-time Display"
- **默认尺寸**: 1280x720
- **显示方式**: 彩色图像和深度图水平拼接显示
- **刷新率**: 默认30毫秒间隔（约33FPS）

## 控制方式

- **ESC键**: 退出程序和关闭窗口
- **窗口关闭按钮**: 停止采集并退出程序

## 配置选项

在 `ConfigHelper.hpp` 中的 `RenderConfig` 结构体中可以配置：

```cpp
struct RenderConfig {
    bool enable = true;                    // 是否启用渲染功能
    bool show_real_time = true;           // 是否显示实时窗口
    std::string window_title = "Mech-Eye Real-time Display";  // 窗口标题
    int window_width = 1280;              // 窗口宽度
    int window_height = 720;              // 窗口高度
    int display_refresh_ms = 30;          // 显示刷新间隔（毫秒）
}
```

## 文件保存

实时显示的同时，所有数据仍会保存到 `./img/` 目录：
- `时间戳_2DImage.png` - 2D彩色图像
- `时间戳_DepthMap.tiff` - 深度图  
- `时间戳_PointCloud.ply` - 点云文件
- `时间戳_TexturedPointCloud.ply` - 纹理点云文件

## 运行方式

```bash
cd build
./perception_app
```

程序启动后会自动：
1. 连接摄像头
2. 初始化实时显示窗口
3. 开始采集和显示循环
4. 同时保存文件到img目录

## 性能优化

- 显示刷新间隔可调节（默认30ms）
- 图像会自动缩放到合适尺寸显示
- 使用多线程安全的显示更新

## 故障排除

1. **窗口不显示**: 检查 `render_config_.enable` 和 `render_config_.show_real_time` 是否为 true
2. **性能问题**: 增大 `display_refresh_ms` 值来降低刷新率
3. **显示异常**: 确保OpenCV正确安装并支持GUI功能 