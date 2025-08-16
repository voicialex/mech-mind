#include "ConfigHelper.hpp"
#include <iostream>

std::string ConfigHelper::SaveConfig::save_2d_image_file(const std::string& suffix) const
{
    return  this->save_path + "/" + suffix + "_2DImage.png";
}

std::string ConfigHelper::SaveConfig::save_depth_map_file(const std::string& suffix) const
{
    return this->save_path + "/" + suffix + "_DepthMap.tiff";
}

std::string ConfigHelper::SaveConfig::save_point_cloud_file(const std::string& suffix) const
{
    return this->save_path + "/" + suffix + "_PointCloud.ply";
}

std::string ConfigHelper::SaveConfig::save_textured_point_cloud_file(const std::string& suffix) const
{
    return this->save_path + "/" + suffix + "_TexturedPointCloud.ply";
}

bool ConfigHelper::loadConfigFromJson(const std::string& configPath)
{
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "无法打开配置文件: " << configPath << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;

        // 解析渲染配置
        if (j.contains("render_config")) {
            auto& render = j["render_config"];
            render_config_.enable = render.value("enable", true);
            render_config_.window_title = render.value("window_title", "Real-time Capture Display");
            render_config_.window_width = render.value("window_width", 1280);
            render_config_.window_height = render.value("window_height", 720);
        }

        // 解析捕获配置
        if (j.contains("capture_config")) {
            auto& capture = j["capture_config"];
            capture_config_.enable = capture.value("enable", true);
            capture_config_.suffix = capture.value("suffix", "");
            capture_config_.color_file = capture.value("color_file", "2DImage.png");
            capture_config_.depth_file = capture.value("depth_file", "DepthMap.tiff");
            capture_config_.point_cloud_file = capture.value("point_cloud_file", "PointCloud.ply");
            capture_config_.textured_point_cloud_file = capture.value("textured_point_cloud_file", "TexturedPointCloud.ply");
        }

        // 解析保存配置
        if (j.contains("save_config")) {
            auto& save = j["save_config"];
            save_config_.save_path = save.value("save_path", "./dumps/");
            save_config_.save_2d_image = save.value("save_2d_image", true);
            save_config_.save_depth_map = save.value("save_depth_map", true);
            save_config_.save_point_cloud = save.value("save_point_cloud", true);
            save_config_.save_textured_point_cloud = save.value("save_textured_point_cloud", true);
            save_config_.max_save_count = save.value("max_save_count", 20);
        }

        // 解析日志配置
        if (j.contains("log_config")) {
            auto& log = j["log_config"];
            log_config_.enable = log.value("enable", true);
            log_config_.level = log.value("level", "INFO");
            log_config_.show_timestamp = log.value("show_timestamp", true);
            log_config_.show_level = log.value("show_level", true);
        }

        std::cout << "配置文件加载成功: " << configPath << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "解析配置文件时发生错误: " << e.what() << std::endl;
        return false;
    }
}

void ConfigHelper::printConfig() const
{
    std::cout << "=== 当前配置信息 ===" << std::endl;
    
    std::cout << "渲染配置:" << std::endl;
    std::cout << "  启用: " << (render_config_.enable ? "是" : "否") << std::endl;
    std::cout << "  窗口标题: " << render_config_.window_title << std::endl;
    std::cout << "  窗口尺寸: " << render_config_.window_width << "x" << render_config_.window_height << std::endl;
    
    std::cout << "捕获配置:" << std::endl;
    std::cout << "  启用: " << (capture_config_.enable ? "是" : "否") << std::endl;
    std::cout << "  后缀: " << capture_config_.suffix << std::endl;
    std::cout << "  颜色文件: " << capture_config_.color_file << std::endl;
    std::cout << "  深度文件: " << capture_config_.depth_file << std::endl;
    std::cout << "  点云文件: " << capture_config_.point_cloud_file << std::endl;
    std::cout << "  纹理点云文件: " << capture_config_.textured_point_cloud_file << std::endl;
    
    std::cout << "保存配置:" << std::endl;
    std::cout << "  保存路径: " << save_config_.save_path << std::endl;
    std::cout << "  保存2D图像: " << (save_config_.save_2d_image ? "是" : "否") << std::endl;
    std::cout << "  保存深度图: " << (save_config_.save_depth_map ? "是" : "否") << std::endl;
    std::cout << "  保存点云: " << (save_config_.save_point_cloud ? "是" : "否") << std::endl;
    std::cout << "  保存纹理点云: " << (save_config_.save_textured_point_cloud ? "是" : "否") << std::endl;
    std::cout << "  最大保存数量: " << save_config_.max_save_count << std::endl;
    
    std::cout << "日志配置:" << std::endl;
    std::cout << "  启用: " << (log_config_.enable ? "是" : "否") << std::endl;
    std::cout << "  级别: " << log_config_.level << std::endl;
    std::cout << "  显示时间戳: " << (log_config_.show_timestamp ? "是" : "否") << std::endl;
    std::cout << "  显示级别: " << (log_config_.show_level ? "是" : "否") << std::endl;
    
    std::cout << "==================" << std::endl;
}