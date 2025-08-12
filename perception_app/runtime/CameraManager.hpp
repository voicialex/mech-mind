#include <string>
#include <atomic>
#include <deque>
#include <memory>
#include "area_scan_3d_camera/Camera.h"
#include "FrameSet.hpp"
#include "utils/CVWindow.hpp"
#include "InferenceInterface.hpp"

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    bool Init();
    bool Start();
    bool Stop();

    bool Connect();

    void Capture(mmind::eye::Camera& camera, const std::string& suffix);

    /**
     * @brief 启用推理处理
     * @param config_path 推理配置文件路径（可选，默认使用配置文件中的路径）
     * @return true 启用成功，false 启用失败
     */
    bool EnableInference(const std::string& config_path = "");

    /**
     * @brief 禁用推理处理
     */
    void DisableInference();

    /**
     * @brief 获取推理结果
     * @return 推理结果字符串
     */
    std::string GetInferenceResult() const;

private:
    void CheckFilesLimit();

    void SaveImages(FrameSet& frame, const std::string& suffix);

    void ShowImages(FrameSet& frame);

    /**
     * @brief 处理推理
     * @param frame_set 帧数据
     */
    void ProcessInference(FrameSet& frame_set);

private:
    mmind::eye::Camera camera_;
    std::atomic<bool> is_running_ {false};
    std::deque<std::vector<std::string>> file_queue_;
    std::unique_ptr<CVWindow> display_window_;
    bool inference_enabled_ = false;
};