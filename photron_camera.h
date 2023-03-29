#ifndef PHOTRON_CAMERA_H
#define PHOTRON_CAMERA_H
#include "windows.h"
// Include windows.h before "PDCLIB.h"(vfw.h)
#include "PDCLIB.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class photron_camera
{
public:
    photron_camera();
    ~photron_camera();
    cv::Mat GetLiveImage();

    void PrintCameraSetting();
    std::vector<std::string> GetFramerateList();
    std::vector<std::string> GetShutterspeedList();
    std::vector<std::string> GetResolutionList();

    int GetFramerateListNum();
    int GetFramerate();
    int GetShutterspeedListNum();
    int GetResolutionListNum();
    void SetFramerateListNum(int);
    void SetShutterspeedListNum(int);
    void SetResolutionListNum(int);
    int GetImageWidth();
    int GetImageHeight();

private:
    unsigned long nRet;
    unsigned long nErrorCode;
    unsigned long pDetectNoDummy{0};
    unsigned long nDetectNum{256};
    unsigned long nDeviceNo{0};    //device number
    unsigned long nCount{0};
    unsigned long nChildNo;
    unsigned long nSize;
    unsigned long m_Width; //image width
    unsigned long m_Height;  //image height
    unsigned long m_nRecordRate; //current frame rate
    unsigned long nShutterList[PDC_MAX_LIST_NUMBER]; //list of shutter speed
    unsigned long ChildNo[PDC_MAX_DEVICE]; //the number of child devices
    unsigned long RateList[PDC_MAX_LIST_NUMBER];
    unsigned long nResolutionList[PDC_MAX_LIST_NUMBER]; //list of resolution
    PDC_DETECT_NUM_INFO DetectNumInfo;

    unsigned long m_nShutterSpeed; //current shutter speed
    int	m_dwResolution;	//current resolution
    BOOL m_Execute_Live; //execute live
    BOOL print_on;
    char nMode;
    unsigned char *livedata;

    int FramerateListNum{0};
    int ShutterspeedListNum{0};
    int ResolutionListNum{0};

    void UpdateParam();
    void SetLiveMode();

};

#endif // PHOTRON_CAMERA_H
