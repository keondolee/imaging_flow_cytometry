#include "photron_camera.h"

photron_camera::photron_camera()
{
    // init parameters
    m_Execute_Live = FALSE;
    print_on = FALSE;
    FramerateListNum = 5;
    ShutterspeedListNum = 22;
    ResolutionListNum = 0; //2

    nRet = PDC_Init(&nErrorCode);

    // device search
    nRet = PDC_DetectDevice(PDC_INTTYPE_PCI,
                            &pDetectNoDummy,
                            nDetectNum,
                            PDC_DETECT_AUTO,
                            &DetectNumInfo,
                            &nErrorCode);

    std::cout << "Device Num : "<< DetectNumInfo.m_nDeviceNum << std::endl;
    std::cout << "Device Code : "<< DetectNumInfo.m_DetectInfo->m_nDeviceCode << std::endl;
    std::cout << "Device No : "<< DetectNumInfo.m_DetectInfo->m_nTmpDeviceNo << std::endl;
    std::cout << "Interface Code : "<< DetectNumInfo.m_DetectInfo->m_nInterfaceCode << std::endl;

    // opening the device
    nRet = PDC_OpenDevice(&(DetectNumInfo.m_DetectInfo[0]),
                            &nDeviceNo,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_OpenDevice" << std::endl;
        return;
    }

    // get the child device
    nRet = PDC_GetExistChildDeviceList(nDeviceNo,
                                       &nCount,
                                       ChildNo,
                                       &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetExistChildDeviceList" << std::endl;
        return;
    }

    // display the list of child device
    for(unsigned int i=0; i<nCount&&print_on; i++){
       std::cout << "ChildNo["<<i<<"]: "<<ChildNo[i] << std::endl;
    }

    nChildNo = ChildNo[0];

    // get the device name
    TCHAR StrName[50];
    nRet = PDC_GetDeviceName(nDeviceNo,
                            0,
                            StrName,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetDeviceName" << std::endl;
        return;
    }

    std::cout << StrName << " is connected." << std::endl;

    // get the record rate list
    nRet = PDC_GetRecordRateList(nDeviceNo,
                            nChildNo,
                            &nCount,
                            RateList,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetRecordRateList" << std::endl;
        return;
    }

    // display the list of record rate
    for(unsigned int i=0; i<nCount&&print_on; i++){
       std::cout << "RateList["<<i<<"]: "<<RateList[i] <<"fps" << std::endl;
    }

    m_nRecordRate = RateList[FramerateListNum];

    //std::cout << "m_nRecordRate: " << m_nRecordRate << "fps" << std::endl;

    // set the record rate
    nRet = PDC_SetRecordRate(nDeviceNo,
                            nChildNo,
                            m_nRecordRate,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetRecordRate" << std::endl;
        return;
    }

    // update parameters(shutter speed, resolution, trigger mode)
    UpdateParam();

    // set live mode
    SetLiveMode();

}

photron_camera::~photron_camera()
{

    if(livedata){
        delete livedata;
    }
    PDC_CloseDevice(nDeviceNo, &nErrorCode);
}

void photron_camera::UpdateParam()
{
    int	nWidth;
    int	nHeight;

    // get a list of shutterspeed
    nRet = PDC_GetShutterSpeedFpsList(nDeviceNo,
                                        nChildNo,
                                        &nSize,
                                        nShutterList,
                                        &nErrorCode);
    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetShutterSpeedFpsList" << std::endl;
        return;
    }

    // display the list of shutterspeed
    for(unsigned int i=0; i<nSize&&print_on; i++){
       std::cout << "nShutterList["<<i<<"]: 1/"<<nShutterList[i] <<"s"<< std::endl;
    }

    m_nShutterSpeed = nShutterList[ShutterspeedListNum];

    // set the shutterspeed
    nRet = PDC_SetShutterSpeedFps(nDeviceNo,
                                    nChildNo,
                                    m_nShutterSpeed,
                                    &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetShutterSpeedFps" << std::endl;
        return;
    }

    // get a list of resolution
    nRet = PDC_GetResolutionList(nDeviceNo,
                                        nChildNo,
                                        &nSize,
                                        nResolutionList,
                                        &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetResolutionList" << std::endl;
        return;
    }

    // display the list
    for(unsigned int i=0; i<nSize&&print_on; i++){
       nWidth	= nResolutionList[i] & 0xffff0000;
       nWidth	= nWidth>>16;
       nHeight = nResolutionList[i] & 0x0000ffff;
       std::cout << "nResolutionList["<<i<<"]: "<<nWidth <<"x"<< nHeight<< "-pixels (WxH)" << std::endl;
    }

    // set the resolution as the current resolution
    m_dwResolution = nResolutionList[ResolutionListNum]; //2
    // set the resolution
    nWidth	= m_dwResolution & 0xffff0000;
    nWidth	= nWidth>>16;
    nHeight = m_dwResolution & 0x0000ffff;

    nRet = PDC_SetResolution(nDeviceNo,
                            nChildNo,
                            nWidth,
                            nHeight,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetResolution" << std::endl;
        return;
    }
    m_Width  = nWidth;
    m_Height = nHeight;

    // set the trigger mode?????????????????????????
    nRet = PDC_SetTriggerMode(nDeviceNo,
                                PDC_TRIGGER_START,
                                0,
                                0,
                                0,
                                &nErrorCode);
    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetTriggerMode" << std::endl;
        return;
    }

}

void photron_camera::SetLiveMode(){
    int	nInterval;
    if(m_Execute_Live==TRUE){
        return;
    }

    // set live mode
    nRet = PDC_SetStatus(nDeviceNo,
                         PDC_STATUS_LIVE,
                         &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetStatus" << std::endl;
        return;
    }

    // get the resolution of live image
    nRet = PDC_GetResolution(nDeviceNo,
                     nChildNo,
                     &m_Width,
                     &m_Height,
                     &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetResolution" << std::endl;
        return;
    }

    // get color type information
    nRet = PDC_GetColorType(nDeviceNo,
                         nChildNo,
                         &nMode,
                         &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetColorType" << std::endl;
        return;
    }

    if(nMode == PDC_COLORTYPE_COLOR){
        if(m_Width<=512){
            nInterval = 100;	//100ms
        }else if(m_Width<=640){
            nInterval = 200;	//200ms
        }else if(m_Width<=1280){
            nInterval = 500;	//500ms
        }else{
            nInterval = 1000;	//1000ms
        }
    }else{
        if(m_Width<=512){
            nInterval = 100;	//100ms
        }else if(m_Width<=640){
            nInterval = 200;	//200ms
        }else if(m_Width<=1280){
            nInterval = 500;	//500ms
        }else{
            nInterval = 1000;	//1000ms
        }
    }

    m_Execute_Live = TRUE;

}

cv::Mat photron_camera::GetLiveImage(){
    // PDC_GetLiveImageData is too slow, 512x512x1 takes 2ms.
    // PDC_GetLiveImageAddress is fast, 512x512x1 takes 1ms.
    nRet = PDC_GetLiveImageAddress(nDeviceNo,
                                   nChildNo,
                                   (void**)&livedata,
                                   &nErrorCode);
    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetLiveImageAddress" << std::endl;
        cv::Mat img1C(cv::Size(1, 1), CV_8UC1);
        return img1C;
    }else{
        livedata = livedata + 8;
        cv::Mat img1C(cv::Size(m_Width, m_Height), CV_8UC1, livedata, cv::Mat::AUTO_STEP);
        return img1C;
    }
}

void photron_camera::PrintCameraSetting(){
    int nInterval;
    // get the resolution of live image
    nRet = PDC_GetResolution(nDeviceNo,
                     nChildNo,
                     &m_Width,
                     &m_Height,
                     &nErrorCode);
    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetResolution" << std::endl;
        return;
    }
    // get color type information
    nRet = PDC_GetColorType(nDeviceNo,
                         nChildNo,
                         &nMode,
                         &nErrorCode);
    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetColorType" << std::endl;
        return;
    }

    if(nMode == PDC_COLORTYPE_COLOR){
        if(m_Width<=512){
            nInterval = 100;	//100ms
        }else if(m_Width<=640){
            nInterval = 200;	//200ms
        }else if(m_Width<=1280){
            nInterval = 500;	//500ms
        }else{
            nInterval = 1000;	//1000ms
        }
    }else{
        if(m_Width<=512){
            nInterval = 100;	//100ms
        }else if(m_Width<=640){
            nInterval = 200;	//200ms
        }else if(m_Width<=1280){
            nInterval = 500;	//500ms
        }else{
            nInterval = 1000;	//1000ms
        }
    }

    nRet = PDC_GetRecordRate(nDeviceNo,
                      nChildNo,
                      &m_nRecordRate,
                      &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetRecordRate" << std::endl;
        return;
    }

    nRet = PDC_GetShutterSpeedFps(nDeviceNo,
                                  nChildNo,
                                  &m_nShutterSpeed,
                                  &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetShutterSpeedFps" << std::endl;
        return;
    }
    std::cout << "Frame rate: " << m_nRecordRate << "fps" << std::endl;
    std::cout << "Shutter speed: 1/" << m_nShutterSpeed << "s" << std::endl;
    std::cout << "Resolution: " << m_Width << "x" << m_Height <<"-pixels (WxH)"<< std::endl;

}

std::vector<std::string> photron_camera::GetFramerateList(){
    std::vector<std::string> tmp;
    // get the record rate list
    nRet = PDC_GetRecordRateList(nDeviceNo,
                            nChildNo,
                            &nCount,
                            RateList,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetRecordRateList" << std::endl;
    }

    // display the list of record rate
    for(unsigned int i=0; i<nCount; i++){
       tmp.push_back(std::to_string(RateList[i]));
    }
    return tmp;
}

std::vector<std::string> photron_camera::GetShutterspeedList(){
    std::vector<std::string> tmp;
    // get a list of shutterspeed
    nRet = PDC_GetShutterSpeedFpsList(nDeviceNo,
                                        nChildNo,
                                        &nSize,
                                        nShutterList,
                                        &nErrorCode);
    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetShutterSpeedFpsList" << std::endl;
    }

    // display the list of shutterspeed
    for(unsigned int i=0; i<nSize; i++){
       tmp.push_back("1/"+std::to_string(nShutterList[i]));
    }
    return tmp;
}

std::vector<std::string> photron_camera::GetResolutionList(){
    int nWidth, nHeight;
    std::vector<std::string> tmp;
    // get a list of resolution
    nRet = PDC_GetResolutionList(nDeviceNo,
                                        nChildNo,
                                        &nSize,
                                        nResolutionList,
                                        &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_GetResolutionList" << std::endl;
    }

    // display the list
    for(unsigned int i=0; i<nSize; i++){
       nWidth	= nResolutionList[i] & 0xffff0000;
       nWidth	= nWidth>>16;
       nHeight = nResolutionList[i] & 0x0000ffff;
       tmp.push_back(std::to_string(nWidth)+"x"+std::to_string(nHeight)+" (WxH)");
    }
    return tmp;
}
int photron_camera::GetFramerateListNum(){
    return FramerateListNum;
}
int photron_camera::GetShutterspeedListNum(){
    return ShutterspeedListNum;
}
int photron_camera::GetResolutionListNum(){
    return ResolutionListNum;
}

int photron_camera::GetFramerate(){
    return m_nRecordRate;
}

void photron_camera::SetFramerateListNum(int tmp){
    FramerateListNum = tmp;
    m_nRecordRate = RateList[FramerateListNum];

    // set the record rate
    nRet = PDC_SetRecordRate(nDeviceNo,
                            nChildNo,
                            m_nRecordRate,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetRecordRate" << std::endl;
        return;
    }
}
void photron_camera::SetShutterspeedListNum(int tmp){
    ShutterspeedListNum = tmp;
    m_nShutterSpeed = nShutterList[ShutterspeedListNum];

    // set the shutterspeed
    nRet = PDC_SetShutterSpeedFps(nDeviceNo,
                                    nChildNo,
                                    m_nShutterSpeed,
                                    &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetShutterSpeedFps" << std::endl;
        return;
    }
}
void photron_camera::SetResolutionListNum(int tmp){
    ResolutionListNum = tmp;
    // update function
    int nWidth, nHeight;
    // set the resolution as the current resolution
    m_dwResolution = nResolutionList[ResolutionListNum]; //2

    // set the resolution
    nWidth	= m_dwResolution & 0xffff0000;
    nWidth	= nWidth>>16;
    nHeight = m_dwResolution & 0x0000ffff;

    nRet = PDC_SetResolution(nDeviceNo,
                            nChildNo,
                            nWidth,
                            nHeight,
                            &nErrorCode);

    if(nRet == PDC_FAILED) {
        std::cerr << "[ERROR] PDC_SetResolution" << std::endl;
        return;
    }
    m_Width  = nWidth;
    m_Height = nHeight;
}

int photron_camera::GetImageWidth(){
    return (int)m_Width;
}
int photron_camera::GetImageHeight(){
    return (int)m_Height;
}
