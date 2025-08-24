#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

class ConfigHelper
{
public:
    // Camera 相关配置统一结构体
    struct CameraConfig
    {
        struct RenderConfig
        {
            bool enable = true;
            std::string window_title = "Real-time Capture Display";
            int window_width = 1280;
            int window_height = 720;
        } render;

        struct CaptureConfig
        {
            bool enable = true;
            std::string suffix = "";
            std::string color_file = "2DImage.png";
            std::string depth_file = "DepthMap.tiff";
            std::string point_cloud_file = "PointCloud.ply";
            std::string textured_point_cloud_file = "TexturedPointCloud.ply";
        } capture;

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
        } save;
    } camera_config_;

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

    // 新增通信配置结构
    struct CommunicationConfig
    {
        struct ServiceDiscoveryConfig
        {
            std::string local_address = "0.0.0.0";
            uint16_t discovery_port = 8080;
            uint32_t broadcast_interval = 5000; // 毫秒
            bool enable_broadcast = true;
        } service_discovery;

        struct ServerConfig
        {
            std::string id = "device_server";
            std::string name = "Device Server";
            std::string address = "127.0.0.1";
            uint16_t port = 9090;
            uint32_t max_clients = 100;
            uint32_t client_timeout = 30000; // 毫秒
        } server;

        struct ClientConfig
        {
            std::string id = "device_client";
            std::string name = "Device Client";
            std::string address = "0.0.0.0";
            uint16_t port = 0;
            bool enable_auto_reconnect = true;
            uint32_t max_reconnect_attempts = 10;
            uint32_t reconnect_interval = 5000; // 毫秒
            uint32_t connection_check_interval = 10000; // 毫秒
        } client;

        struct HeartbeatConfig
        {
            bool enable = true;
            uint32_t interval = 3000; // 毫秒
            uint32_t timeout_multiplier = 3; // 超时倍数
            uint32_t max_missed_responses = 3; // 最大未响应次数
        } heartbeat;

        struct MasterNodeConfig
        {
            uint32_t client_timeout_interval = 60000; // 客户端超时时间（毫秒）
            uint32_t status_check_interval = 10000; // 状态检查间隔（毫秒）
            uint32_t state_sync_interval = 5000; // 状态同步间隔（毫秒）
            bool enable_auto_cleanup = true; // 是否启用自动清理离线客户端
        } master_node;

        struct MessageConfig
        {
            uint32_t max_payload_size = 65536; // 最大负载大小
            uint32_t message_timeout = 5000; // 消息超时时间（毫秒）
            bool enable_crc_check = true; // 是否启用CRC校验
        } message;
    } communication_config_;

public:
    static ConfigHelper& getInstance() {
        static ConfigHelper instance;
        return instance;
    }

    // 添加配置加载函数
    bool loadConfigFromJson(const std::string& configPath = "config/config.json");
    bool loadCommunicationConfigFromJson(const std::string& configPath = "config/communication_config.json");
    void printConfig() const; // 用于调试，打印当前配置
    void printCommunicationConfig() const; // 打印通信配置

private:
    ConfigHelper() = default;
    ~ConfigHelper() = default;
    ConfigHelper(const ConfigHelper&) = delete;
    ConfigHelper& operator=(const ConfigHelper&) = delete;
    ConfigHelper(ConfigHelper&&) = delete;
    ConfigHelper& operator=(ConfigHelper&&) = delete;
};