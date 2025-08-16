#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

class ConfigHelper
{
public:
    struct RenderConfig
    {
        bool enable = true;
        std::string window_title = "Real-time Capture Display";
        int window_width = 1280;
        int window_height = 720;
    } render_config_;

    struct CaptureConfig
    {
        bool enable = true;
        std::string suffix = "";
        std::string color_file = "2DImage.png";
        std::string depth_file = "DepthMap.tiff";
        std::string point_cloud_file = "PointCloud.ply";
        std::string textured_point_cloud_file = "TexturedPointCloud.ply";
    } capture_config_;

    struct SaveConfig
    {
        std::string save_path = "./dumps/";
        bool save_2d_image = true;
        bool save_depth_map = true;
        bool save_point_cloud = true;
        bool save_textured_point_cloud = true;
        int max_save_count = 20; // 添加最大保存数量控制

        std::string save_2d_image_file(const std::string& suffix) const;
        std::string save_depth_map_file(const std::string& suffix) const;
        std::string save_point_cloud_file(const std::string& suffix) const;
        std::string save_textured_point_cloud_file(const std::string& suffix) const;
    } save_config_;

    struct LogConfig
    {
        bool enable = true;
        std::string level = "INFO"; // DEBUG, INFO, WARNING, ERROR
        bool show_timestamp = true;
        bool show_level = true;
    } log_config_;

    struct InferenceConfig
    {
        bool enable = false;
        std::string config_path = "config/inference_config.json";
    } inference_config_;

public:
    static ConfigHelper& getInstance() {
        static ConfigHelper instance;
        return instance;
    }

    // 添加配置加载函数
    bool loadConfigFromJson(const std::string& configPath = "config/config.json");
    void printConfig() const; // 用于调试，打印当前配置

private:
    ConfigHelper() = default;
    ~ConfigHelper() = default;
    ConfigHelper(const ConfigHelper&) = delete;
    ConfigHelper& operator=(const ConfigHelper&) = delete;
    ConfigHelper(ConfigHelper&&) = delete;
    ConfigHelper& operator=(ConfigHelper&&) = delete;
};