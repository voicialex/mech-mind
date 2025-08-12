#pragma once

#include "InferenceInterface.hpp"
#include <opencv2/opencv.hpp>
#include <string>

/**
 * @brief 示例推理算法类
 * 
 * 这个类演示了如何继承InferenceInterface来实现具体的推理算法。
 * 它提供了一个简单的示例，展示如何处理图像数据并返回结果。
 */
class ExampleInference : public InferenceInterface {
public:
    /**
     * @brief 构造函数
     */
    ExampleInference();

    /**
     * @brief 析构函数
     */
    ~ExampleInference() override;

    /**
     * @brief 初始化推理模型
     * @param config_path 配置文件路径
     * @return true 初始化成功，false 初始化失败
     */
    bool Initialize(const std::string& config_path) override;

    /**
     * @brief 处理图像数据
     * @param frame_set 包含2D和3D数据的帧集合
     * @return true 处理成功，false 处理失败
     */
    bool Process(const FrameSet& frame_set) override;

    /**
     * @brief 获取推理结果
     * @return 推理结果的字符串表示
     */
    std::string GetResult() const override;

    /**
     * @brief 清理资源
     */
    void Cleanup() override;

    /**
     * @brief 检查推理模型是否已初始化
     * @return true 已初始化，false 未初始化
     */
    bool IsInitialized() const override;

    /**
     * @brief 获取推理算法的名称
     * @return 算法名称
     */
    std::string GetAlgorithmName() const override;

private:
    /**
     * @brief 处理2D图像
     * @param color_image 彩色图像
     * @return 处理结果
     */
    std::string ProcessColorImage(const cv::Mat& color_image);

    /**
     * @brief 处理深度图像
     * @param depth_image 深度图像
     * @return 处理结果
     */
    std::string ProcessDepthImage(const cv::Mat& depth_image);

    /**
     * @brief 处理点云数据
     * @param point_cloud 点云数据
     * @return 处理结果
     */
    std::string ProcessPointCloud(const mmind::eye::PointCloud& point_cloud);

private:
    bool is_initialized_;
    std::string last_result_;
    int processed_frame_count_;
    
    // 示例配置参数
    double confidence_threshold_;
    int min_object_size_;
    std::string model_path_;
};
