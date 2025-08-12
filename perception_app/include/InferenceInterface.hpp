#pragma once

#include <memory>
#include <string>
#include "../runtime/FrameSet.hpp"
#include "Logger.hpp"

/**
 * @brief Inference interface class for decoupling inference algorithms and runtime framework
 * 
 * This interface class defines the standard interface for inference algorithms, including:
 * 1. Initialize inference model
 * 2. Process image data
 * 3. Get inference results
 * 4. Clean up resources
 * 
 * Inference algorithms need to inherit this interface and implement corresponding methods.
 */
class InferenceInterface {
public:
    /**
     * @brief Virtual destructor to ensure proper resource release
     */
    virtual ~InferenceInterface() = default;

    /**
     * @brief Initialize inference model
     * @param config_path Configuration file path
     * @return true Initialization successful, false Initialization failed
     */
    virtual bool Initialize(const std::string& config_path) = 0;

    /**
     * @brief Process image data
     * @param frame_set Frame set containing 2D and 3D data
     * @return true Processing successful, false Processing failed
     */
    virtual bool Process(const FrameSet& frame_set) = 0;

    /**
     * @brief Get inference results
     * @return String representation of inference results
     */
    virtual std::string GetResult() const = 0;

    /**
     * @brief Clean up resources
     */
    virtual void Cleanup() = 0;

    /**
     * @brief Check if inference model is initialized
     * @return true Initialized, false Not initialized
     */
    virtual bool IsInitialized() const = 0;

    /**
     * @brief Get inference algorithm name
     * @return Algorithm name
     */
    virtual std::string GetAlgorithmName() const = 0;

protected:
    /**
     * @brief Default constructor
     */
    InferenceInterface() = default;

    /**
     * @brief Copy constructor (disabled)
     */
    InferenceInterface(const InferenceInterface&) = delete;

    /**
     * @brief Assignment operator (disabled)
     */
    InferenceInterface& operator=(const InferenceInterface&) = delete;
};

/**
 * @brief Inference manager class responsible for managing inference interface instances
 * 
 * This class uses singleton pattern and is responsible for:
 * 1. Creating and managing inference interface instances
 * 2. Calling inference processing in CameraManager
 * 3. Providing unified inference result access interface
 */
class InferenceManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to InferenceManager instance
     */
    static InferenceManager& getInstance();

    /**
     * @brief Register inference interface
     * @param inference_interface Smart pointer to inference interface
     * @return true Registration successful, false Registration failed
     */
    bool RegisterInference(std::shared_ptr<InferenceInterface> inference_interface);

    /**
     * @brief Initialize inference interface
     * @param config_path Configuration file path
     * @return true Initialization successful, false Initialization failed
     */
    bool InitializeInference(const std::string& config_path);

    /**
     * @brief Process image data
     * @param frame_set Frame set containing 2D and 3D data
     * @return true Processing successful, false Processing failed
     */
    bool Process(const FrameSet& frame_set);

    /**
     * @brief Get inference results
     * @return String representation of inference results
     */
    std::string GetResult() const;

    /**
     * @brief Clean up inference resources
     */
    void Cleanup();

    /**
     * @brief Check if inference is initialized
     * @return true Initialized, false Not initialized
     */
    bool IsInitialized() const;

    /**
     * @brief Get current inference algorithm name
     * @return Algorithm name
     */
    std::string GetCurrentAlgorithmName() const;

private:
    /**
     * @brief Private constructor to implement singleton pattern
     */
    InferenceManager() = default;

    /**
     * @brief Private destructor
     */
    ~InferenceManager() = default;

    /**
     * @brief Copy constructor (disabled)
     */
    InferenceManager(const InferenceManager&) = delete;

    /**
     * @brief Assignment operator (disabled)
     */
    InferenceManager& operator=(const InferenceManager&) = delete;

private:
    std::shared_ptr<InferenceInterface> inference_interface_;
    bool is_initialized_ = false;
};
