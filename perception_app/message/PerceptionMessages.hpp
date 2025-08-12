#pragma once

#include "MessageProtocol.hpp"
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

namespace perception {

/**
 * @brief 感知应用消息ID定义 - 符合充电枪协议规范
 */
namespace MessageIds {
    // 系统消息 (0x0001-0x00FF)
    static constexpr uint16_t HEARTBEAT = 0x0001;
    static constexpr uint16_t SERVICE_DISCOVERY = 0x0002;
    static constexpr uint16_t CONNECTION_REQUEST = 0x0003;
    static constexpr uint16_t CONNECTION_RESPONSE = 0x0004;
    
    // 感知应用消息 (0x0100-0x01FF)
    static constexpr uint16_t PERCEPTION_START = 0x0100;
    static constexpr uint16_t PERCEPTION_STOP = 0x0101;
    static constexpr uint16_t PERCEPTION_STATUS = 0x0102;
    static constexpr uint16_t PERCEPTION_CONFIG = 0x0103;
    
    // 推理相关消息 (0x0200-0x02FF)
    static constexpr uint16_t INFERENCE_REQUEST = 0x0200;
    static constexpr uint16_t INFERENCE_RESPONSE = 0x0201;
    static constexpr uint16_t INFERENCE_STATUS = 0x0202;
    static constexpr uint16_t MODEL_LOAD_REQUEST = 0x0203;
    static constexpr uint16_t MODEL_LOAD_RESPONSE = 0x0204;
    
    // 相机控制消息 (0x0300-0x03FF)
    static constexpr uint16_t CAMERA_START = 0x0300;
    static constexpr uint16_t CAMERA_STOP = 0x0301;
    static constexpr uint16_t CAMERA_CONFIG = 0x0302;
    static constexpr uint16_t CAMERA_STATUS = 0x0303;
    static constexpr uint16_t FRAME_DATA = 0x0304;
    
    // 数据处理消息 (0x0400-0x04FF)
    static constexpr uint16_t DATA_PROCESS_REQUEST = 0x0400;
    static constexpr uint16_t DATA_PROCESS_RESPONSE = 0x0401;
    static constexpr uint16_t DATA_SAVE_REQUEST = 0x0402;
    static constexpr uint16_t DATA_SAVE_RESPONSE = 0x0403;
}

/**
 * @brief 感知应用子消息ID定义 (阶段码) - 符合充电枪协议规范
 */
namespace SubMessageIds {
    // 系统阶段 (0x00-0x0F)
    static constexpr uint8_t IDLE = 0x00;
    static constexpr uint8_t INITIALIZING = 0x01;
    static constexpr uint8_t CONNECTING = 0x02;
    static constexpr uint8_t READY = 0x03;
    static constexpr uint8_t ERROR = 0x0F;
    
    // 感知应用阶段 (0x10-0x1F)
    static constexpr uint8_t PERCEPTION_INITIALIZING = 0x10;
    static constexpr uint8_t PERCEPTION_RUNNING = 0x11;
    static constexpr uint8_t PERCEPTION_PROCESSING = 0x12;
    static constexpr uint8_t PERCEPTION_COMPLETED = 0x13;
    static constexpr uint8_t PERCEPTION_FAILED = 0x14;
    
    // 推理阶段 (0x20-0x2F)
    static constexpr uint8_t MODEL_LOADING = 0x20;
    static constexpr uint8_t INFERENCE_PROCESSING = 0x21;
    static constexpr uint8_t INFERENCE_COMPLETED = 0x22;
    static constexpr uint8_t INFERENCE_FAILED = 0x23;
    
    // 相机阶段 (0x30-0x3F)
    static constexpr uint8_t CAMERA_INITIALIZING = 0x30;
    static constexpr uint8_t CAMERA_CAPTURING = 0x31;
    static constexpr uint8_t CAMERA_PROCESSING = 0x32;
    static constexpr uint8_t CAMERA_SAVING = 0x33;
    
    // 数据处理阶段 (0x40-0x4F)
    static constexpr uint8_t DATA_RECEIVING = 0x40;
    static constexpr uint8_t DATA_PROCESSING = 0x41;
    static constexpr uint8_t DATA_SAVING = 0x42;
    static constexpr uint8_t DATA_COMPLETED = 0x43;
}

/**
 * @brief 错误码定义 - 符合充电枪协议规范
 */
namespace ErrorCodes {
    static constexpr uint16_t SUCCESS = 0x0000;
    static constexpr uint16_t GENERAL_ERROR = 0x0001;
    static constexpr uint16_t INVALID_PARAMETER = 0x0002;
    static constexpr uint16_t TIMEOUT = 0x0003;
    static constexpr uint16_t NOT_FOUND = 0x0004;
    static constexpr uint16_t ALREADY_EXISTS = 0x0005;
    static constexpr uint16_t PERMISSION_DENIED = 0x0006;
    static constexpr uint16_t RESOURCE_UNAVAILABLE = 0x0007;
    
    // 感知应用错误 (0x0100-0x01FF)
    static constexpr uint16_t PERCEPTION_INIT_FAILED = 0x0100;
    static constexpr uint16_t PERCEPTION_PROCESS_FAILED = 0x0101;
    static constexpr uint16_t PERCEPTION_CONFIG_ERROR = 0x0102;
    
    // 推理相关错误 (0x0200-0x02FF)
    static constexpr uint16_t MODEL_LOAD_FAILED = 0x0200;
    static constexpr uint16_t INFERENCE_FAILED = 0x0201;
    static constexpr uint16_t INVALID_INPUT_DATA = 0x0202;
    static constexpr uint16_t MODEL_NOT_LOADED = 0x0203;
    
    // 相机相关错误 (0x0300-0x03FF)
    static constexpr uint16_t CAMERA_NOT_FOUND = 0x0300;
    static constexpr uint16_t CAMERA_INIT_FAILED = 0x0301;
    static constexpr uint16_t CAMERA_CAPTURE_FAILED = 0x0302;
    static constexpr uint16_t CAMERA_DISCONNECTED = 0x0303;
    
    // 数据处理错误 (0x0400-0x04FF)
    static constexpr uint16_t DATA_PROCESS_FAILED = 0x0400;
    static constexpr uint16_t DATA_SAVE_FAILED = 0x0401;
    static constexpr uint16_t INVALID_DATA_FORMAT = 0x0402;
    static constexpr uint16_t INSUFFICIENT_MEMORY = 0x0403;
}

/**
 * @brief 感知应用启动消息
 */
class PerceptionStartMessage : public RequestMessage {
public:
    PerceptionStartMessage();
    
    /**
     * @brief 设置感知配置
     * @param config 配置参数
     */
    void SetPerceptionConfig(const nlohmann::json& config);
    
    /**
     * @brief 获取感知配置
     * @return 配置参数
     */
    nlohmann::json GetPerceptionConfig() const;
    
    std::string ToString() const override;

private:
    nlohmann::json config_;
};

/**
 * @brief 感知应用停止消息
 */
class PerceptionStopMessage : public RequestMessage {
public:
    PerceptionStopMessage();
    
    /**
     * @brief 设置停止原因
     * @param reason 停止原因
     */
    void SetStopReason(const std::string& reason);
    
    /**
     * @brief 获取停止原因
     * @return 停止原因
     */
    std::string GetStopReason() const;
    
    std::string ToString() const override;

private:
    std::string stop_reason_;
};

/**
 * @brief 感知应用状态消息
 */
class PerceptionStatusMessage : public NotifyMessage {
public:
    PerceptionStatusMessage();
    
    /**
     * @brief 设置感知状态
     * @param status 状态信息
     */
    void SetPerceptionStatus(const nlohmann::json& status);
    
    /**
     * @brief 设置处理统计
     * @param total_frames 总帧数
     * @param processed_frames 已处理帧数
     * @param fps 帧率
     * @param processing_time 处理时间
     */
    void SetStatistics(uint32_t total_frames, uint32_t processed_frames, 
                      float fps, uint32_t processing_time);
    
    /**
     * @brief 获取感知状态
     * @return 状态信息
     */
    nlohmann::json GetPerceptionStatus() const;
    
    std::string ToString() const override;

private:
    nlohmann::json status_;
};

/**
 * @brief 推理请求消息
 */
class InferenceRequestMessage : public RequestMessage {
public:
    InferenceRequestMessage();
    
    /**
     * @brief 设置推理参数
     * @param model_name 模型名称
     * @param confidence_threshold 置信度阈值
     * @param input_size 输入尺寸
     */
    void SetInferenceParams(const std::string& model_name, 
                           float confidence_threshold,
                           const cv::Size& input_size);
    
    /**
     * @brief 设置图像数据
     * @param image 图像数据
     */
    void SetImageData(const cv::Mat& image);
    
    /**
     * @brief 设置深度数据
     * @param depth_data 深度数据
     */
    void SetDepthData(const cv::Mat& depth_data);
    
    /**
     * @brief 获取推理参数
     * @return JSON格式的参数
     */
    nlohmann::json GetInferenceParams() const;
    
    /**
     * @brief 获取图像数据
     * @return 图像数据
     */
    cv::Mat GetImageData() const;
    
    /**
     * @brief 获取深度数据
     * @return 深度数据
     */
    cv::Mat GetDepthData() const;
    
    std::string ToString() const override;

private:
    nlohmann::json params_;
    cv::Mat image_data_;
    cv::Mat depth_data_;
};

/**
 * @brief 推理响应消息
 */
class InferenceResponseMessage : public ResponseMessage {
public:
    InferenceResponseMessage();
    
    /**
     * @brief 设置推理结果
     * @param results 推理结果
     */
    void SetInferenceResults(const nlohmann::json& results);
    
    /**
     * @brief 设置处理时间
     * @param processing_time_ms 处理时间(毫秒)
     */
    void SetProcessingTime(uint32_t processing_time_ms);
    
    /**
     * @brief 设置输出图像
     * @param output_image 输出图像
     */
    void SetOutputImage(const cv::Mat& output_image);
    
    /**
     * @brief 获取推理结果
     * @return 推理结果
     */
    nlohmann::json GetInferenceResults() const;
    
    /**
     * @brief 获取处理时间
     * @return 处理时间(毫秒)
     */
    uint32_t GetProcessingTime() const;
    
    /**
     * @brief 获取输出图像
     * @return 输出图像
     */
    cv::Mat GetOutputImage() const;
    
    std::string ToString() const override;

private:
    nlohmann::json results_;
    uint32_t processing_time_ms_;
    cv::Mat output_image_;
};

/**
 * @brief 相机控制消息
 */
class CameraControlMessage : public RequestMessage {
public:
    enum class ControlType {
        START,
        STOP,
        CONFIGURE,
        GET_STATUS
    };
    
    CameraControlMessage(ControlType control_type);
    
    /**
     * @brief 设置相机配置
     * @param config 相机配置
     */
    void SetCameraConfig(const nlohmann::json& config);
    
    /**
     * @brief 获取控制类型
     * @return 控制类型
     */
    ControlType GetControlType() const;
    
    /**
     * @brief 获取相机配置
     * @return 相机配置
     */
    nlohmann::json GetCameraConfig() const;
    
    std::string ToString() const override;

private:
    ControlType control_type_;
    nlohmann::json camera_config_;
};

/**
 * @brief 帧数据消息
 */
class FrameDataMessage : public NotifyMessage {
public:
    FrameDataMessage();
    
    /**
     * @brief 设置帧数据
     * @param frame_id 帧ID
     * @param timestamp 时间戳
     * @param color_image 彩色图像
     * @param depth_image 深度图像
     */
    void SetFrameData(uint64_t frame_id, 
                     uint64_t timestamp,
                     const cv::Mat& color_image,
                     const cv::Mat& depth_image = cv::Mat());
    
    /**
     * @brief 设置元数据
     * @param metadata 元数据
     */
    void SetMetadata(const nlohmann::json& metadata);
    
    /**
     * @brief 获取帧ID
     * @return 帧ID
     */
    uint64_t GetFrameId() const;
    
    /**
     * @brief 获取时间戳
     * @return 时间戳
     */
    uint64_t GetTimestamp() const;
    
    /**
     * @brief 获取彩色图像
     * @return 彩色图像
     */
    cv::Mat GetColorImage() const;
    
    /**
     * @brief 获取深度图像
     * @return 深度图像
     */
    cv::Mat GetDepthImage() const;
    
    /**
     * @brief 获取元数据
     * @return 元数据
     */
    nlohmann::json GetMetadata() const;
    
    std::string ToString() const override;

private:
    uint64_t frame_id_;
    uint64_t timestamp_;
    cv::Mat color_image_;
    cv::Mat depth_image_;
    nlohmann::json metadata_;
};

/**
 * @brief 系统状态消息
 */
class SystemStatusMessage : public NotifyMessage {
public:
    SystemStatusMessage();
    
    /**
     * @brief 设置系统状态
     * @param status 状态信息
     */
    void SetSystemStatus(const nlohmann::json& status);
    
    /**
     * @brief 设置资源使用情况
     * @param cpu_usage CPU使用率
     * @param memory_usage 内存使用率
     * @param gpu_usage GPU使用率
     */
    void SetResourceUsage(float cpu_usage, float memory_usage, float gpu_usage = 0.0f);
    
    /**
     * @brief 设置设备状态
     * @param devices 设备状态列表
     */
    void SetDeviceStatus(const std::vector<nlohmann::json>& devices);
    
    /**
     * @brief 获取系统状态
     * @return 系统状态
     */
    nlohmann::json GetSystemStatus() const;
    
    std::string ToString() const override;

private:
    nlohmann::json system_status_;
};

/**
 * @brief 消息工厂注册函数
 */
void RegisterPerceptionMessageCreators();

} // namespace perception
