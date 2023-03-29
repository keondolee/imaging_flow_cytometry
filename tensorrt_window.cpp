#include "tensorrt_window.h"


tensorrt_window::tensorrt_window()
{
    initializeSampleParams();

    if (!build())
    {
        std::cout << "TensorRT build failed" << std::endl;
    }
}

tensorrt_window::~tensorrt_window()
{
    teardown();
    delete my_buffers;
}
//!
//! \brief Creates the network, configures the builder and creates the network engine
//!
//! \details This function creates the MNIST network by parsing the Uff model
//!          and builds the engine that will be used to run MNIST (mEngine)
//!
//! \return Returns true if the engine was created successfully and false otherwise
//!
bool tensorrt_window::build()
{
    std::cout << "build tensorRT model" << std::endl;
    auto builder = SampleUniquePtr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(sample::gLogger.getTRTLogger()));
    if (!builder)
    {
        return false;
    }

    auto network = SampleUniquePtr<nvinfer1::INetworkDefinition>(builder->createNetwork());
    if (!network)
    {
        return false;
    }

    auto config = SampleUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
    if (!config)
    {
        return false;
    }

    auto parser = SampleUniquePtr<nvuffparser::IUffParser>(nvuffparser::createUffParser());
    if (!parser)
    {
        return false;
    }

    constructNetwork(parser, network);

    builder->setMaxBatchSize(mParams.batchSize);

    config->setMaxWorkspaceSize(64_MiB); //?

    if (mParams.fp16)
    {
        config->setFlag(BuilderFlag::kFP16);
    }
    if (mParams.int8)
    {
        config->setFlag(BuilderFlag::kINT8);
    }
    samplesCommon::enableDLA(builder.get(), config.get(), mParams.dlaCore);

    mEngine = std::shared_ptr<nvinfer1::ICudaEngine>(
        builder->buildEngineWithConfig(*network, *config), samplesCommon::InferDeleter());

    if (!mEngine)
    {
        return false;
    }
    assert(network->getNbInputs() == 1);
    mInputDims = network->getInput(0)->getDimensions();
    assert(mInputDims.nbDims == 3);

    // Add my code
    my_context = SampleUniquePtr<nvinfer1::IExecutionContext>(mEngine->createExecutionContext());
    my_buffers = new samplesCommon::BufferManager(mEngine, mParams.batchSize);

    return true;
}

//!
//! \brief Uses a Uff parser to create the MNIST Network and marks the output layers
//!
//! \param network Pointer to the network that will be populated with the MNIST network
//!
//! \param builder Pointer to the engine builder
//!
void tensorrt_window::constructNetwork(
    SampleUniquePtr<nvuffparser::IUffParser>& parser, SampleUniquePtr<nvinfer1::INetworkDefinition>& network)
{
    // There should only be one input and one output tensor
    assert(mParams.inputTensorNames.size() == 1);
    assert(mParams.outputTensorNames.size() == 1);


    // Register tensorflow input
    parser->registerInput(
        mParams.inputTensorNames[0].c_str(), nvinfer1::Dims3(INPUT_C, INPUT_H, INPUT_W), nvuffparser::UffInputOrder::kNCHW);

    parser->registerOutput(mParams.outputTensorNames[0].c_str());

    parser->parse(mParams.uffFileName.c_str(), *network, nvinfer1::DataType::kFLOAT);

    if (mParams.int8)
    {
        samplesCommon::setAllTensorScales(network.get(), 127.0f, 127.0f);
    }
}

//!
//! \brief Reads the input data, preprocesses, and stores the result in a managed buffer
//!
bool tensorrt_window::processInput2(const samplesCommon::BufferManager& buffers, const std::string& inputTensorName, cv::Mat image) const
{
    uint8_t *HWC_Mat = image.data;

    // Copy cv::Mat to hostInputBuffer
    float* hostInputBuffer = static_cast<float*>(buffers.getHostBuffer(inputTensorName));

    int width = image.cols;
    int height = image.rows;
    int channels = image.channels();

    for(int h=0; h < height; h++){
        for(int w=0; w<width; w++){
            for(int c=0; c<channels; c++){
                uint8_t val = HWC_Mat[h*width*channels + w*channels + c];
                int CHW_idx = c*width*height + h*width + w;
                hostInputBuffer[CHW_idx] = val/255.0;
            }
        }
    }

    return true;
}

int tensorrt_window::MaximumIdx(
        const samplesCommon::BufferManager& buffers, const std::string& outputTensorName) const
{
    const float* prob = static_cast<const float*>(buffers.getHostBuffer(outputTensorName));

    float val{0.0f};
    int idx{0};

    // Determine index with highest output value
    for (int i = 0; i < CLASS_NUM; i++){
        if (val < prob[i])
        {
            val = prob[i];
            idx = i;
        }
    }
    // set threshold 0.8;
    if(val<0.8){
        return -1;
    }else{
        return idx;
    }
}


//!
//! \brief Runs the TensorRT inference engine for this sample
//!
//! \details This function is the main execution function of the sample.
//!  It allocates the buffer, sets inputs, executes the engine, and verifies the output.
//!
//!

int tensorrt_window::predict(cv::Mat img)
{

    if(INPUT_C == 3 && img.channels() == 1){
        // If img channel is one channel.
        cv::cvtColor(img, img, cv::COLOR_GRAY2RGB);
    }else if(INPUT_C == 1 && img.channels() == 3){
        cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
    }else if(INPUT_C == 3 && img.channels() == 3){
        // Important!, without this line of code, 10um clusters would be classified as 15um.
        // Low prediction accuracy
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB); // it is necessary. Bead : same results, Cancer : different results
    }

    // Process input image
    if (!processInput2(*my_buffers, mParams.inputTensorNames[0], img))
    {
        return false;
    }

    // Copy data from host input buffers to device input buffers
    my_buffers->copyInputToDevice();

    // Execute the inference work
    if (!my_context->execute(mParams.batchSize, my_buffers->getDeviceBindings().data()))
    {
        return false;
    }

    // Copy data from device output buffers to host output buffers
    my_buffers->copyOutputToHost();

    // Check and print the output of the inference
    int class_id = MaximumIdx(*my_buffers, mParams.outputTensorNames[0]);

    return class_id;
}

//!
//! \brief Used to clean up any state created in the sample class
//!
bool tensorrt_window::teardown()
{
    nvuffparser::shutdownProtobufLibrary();
    return true;
}


std::vector<std::string> tensorrt_window::get_class_names(){
    return meta;
}


//!
//! \brief Initializes members of the params struct
//!        using the command line args
//!
void tensorrt_window::initializeSampleParams()
{    
    // resnet18-W50xH50xC3-input_1-fc1000_Softmax-15um_blue(0)+10um_green(1)_two15um.uff
    // resnet18-W50xH50xC3-input_1-fc1000_Softmax-HL60(0)+Jurkat(1).uff
    // resnet18-W50xH50xC3-input_1-fc1000_Softmax-HL60(0)+K562(1).uff
    // resnet18-W50xH50xC3-inputs-Identity-15um(0)+10um(1).uff
    // resnet18-W50xH50xC3-inputs-Identity-15um(0)+10um(1)+6um(2).uff

    std::cout << "initialize tensorRT" << std::endl;
    INPUT_H = 50;
    INPUT_W = 50;
    INPUT_C = 3;
    CLASS_NUM = 2;

    meta.push_back("Raji"); //0
    meta.push_back("Jurkat");  //1
    //meta.push_back("Raji");  //2

    mParams.dataDirs.push_back("tensorRT/"); // relase directory
    mParams.uffFileName = locateFile("resnet18-W50xH50xC3-inputs-Identity-Raji(0)+Jurkat(1).uff", mParams.dataDirs);
    mParams.inputTensorNames.push_back("inputs");
    mParams.batchSize = 1;
    mParams.outputTensorNames.push_back("Identity");
    mParams.dlaCore = -1;
    mParams.int8 = false;
    /*FP16 is supported but at a low rate. So performance won’t be interesting. The driver version you have should be fine. I would recommend using CUDA 8.0.61 (CUDA 8 GA2) which is what is currently publicly available.
        The only GPUs with full-rate FP16 performance are Tesla P100, Quadro GP100, and Jetson TX1/TX2.
        All GPUs with compute capability 6.1 (e.g. GTX 1050, 1060, 1070, 1080, Pascal Titan X, Titan Xp, Tesla P40, etc.) have low-rate FP16 performance. It’s not the fast path on these GPUs. All of these GPUs should support “full rate” INT8 performance, however.*/
    mParams.fp16 = true;

}

