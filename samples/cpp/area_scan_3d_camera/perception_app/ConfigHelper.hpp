#include <string>

class ConfigHelper
{
public:
    struct RenderConfig
    {
        bool enable = true;
        bool show_real_time = true;
        std::string window_title = "Mech-Eye Real-time Display";
        int window_width = 1280;
        int window_height = 720;
        int display_refresh_ms = 10;  // 显示刷新间隔毫秒（优化为10ms）
        bool enable_performance_monitor = true;  // 性能监控（默认启用）
        int performance_report_interval = 100;    // 每100帧报告一次性能
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
        std::string save_path = "./img";
        bool save_2d_image = true;
        bool save_depth_map = true;
        bool save_point_cloud = true;
        bool save_textured_point_cloud = true;
        int max_save_count = 100; // 添加最大保存数量控制

        std::string save_2d_image_file(const std::string& suffix) const;
        std::string save_depth_map_file(const std::string& suffix) const;
        std::string save_point_cloud_file(const std::string& suffix) const;
        std::string save_textured_point_cloud_file(const std::string& suffix) const;
    } save_config_;

public:
    static ConfigHelper& getInstance() {
        static ConfigHelper instance;
        return instance;
    }

private:
    ConfigHelper() = default;
    ~ConfigHelper() = default;
    ConfigHelper(const ConfigHelper&) = delete;
    ConfigHelper& operator=(const ConfigHelper&) = delete;
    ConfigHelper(ConfigHelper&&) = delete;
    ConfigHelper& operator=(ConfigHelper&&) = delete;

    void loadConfig(const std::string& configPath);
    void saveConfig(const std::string& configPath);
};