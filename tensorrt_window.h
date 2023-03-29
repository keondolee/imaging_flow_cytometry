#ifndef TENSORRT_WINDOW_H
#define TENSORRT_WINDOW_H

#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"

#include "NvInfer.h"
#include "NvUffParser.h"
#include <cuda_runtime_api.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class tensorrt_window
{
    template <typename T>
    using SampleUniquePtr = std::unique_ptr<T, samplesCommon::InferDeleter>;
public:
    tensorrt_window();
    ~tensorrt_window();

    //!
    //! \brief Runs the TensorRT inference engine for this sample
    //!
    int predict(cv::Mat image);

    //!
    //! \brief Used to clean up any state created in the sample class
    //!
    bool teardown();

    std::vector<std::string> get_class_names();


private:
    std::vector<std::string> meta;
    int INPUT_H;
    int INPUT_W;
    int INPUT_C;
    int CLASS_NUM;

    SampleUniquePtr<nvinfer1::IExecutionContext> my_context;
    samplesCommon::BufferManager* my_buffers;
    std::shared_ptr<nvinfer1::ICudaEngine> mEngine{nullptr}; //!< The TensorRT engine used to run the network
    samplesCommon::UffSampleParams mParams;
    nvinfer1::Dims mInputDims;

    //!
    //! \brief Builds the network engine
    //!
    bool build();

    //!
    //! \brief Parses a Uff model for MNIST and creates a TensorRT network
    //!
    void constructNetwork(
        SampleUniquePtr<nvuffparser::IUffParser>& parser, SampleUniquePtr<nvinfer1::INetworkDefinition>& network);

    //!
    //! \brief Reads the input and mean data, preprocesses, and stores the result
    //!        in a managed buffer
    //!
    bool processInput2(
        const samplesCommon::BufferManager& buffers, const std::string& inputTensorName, cv::Mat img) const;

    int MaximumIdx(
            const samplesCommon::BufferManager& buffers, const std::string& outputTensorName) const;

    void initializeSampleParams();

};

#endif // TENSORRT_WINDOW_H
