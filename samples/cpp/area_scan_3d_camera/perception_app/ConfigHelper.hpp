#include <string>

class ConfigHelper
{
public:
    struct RenderConfig
    {
        bool enable = true;
        std::string window_title = "Real-time Capture Display";
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