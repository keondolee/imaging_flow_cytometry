#pragma once
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <chrono>

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
typedef float track_t;
typedef cv::Point_<track_t> Point_t;
#define Mat_t CV_32FC

///
/// \brief The CRegion class
///
class CRegion
{
public:
    CRegion()
        : m_type(""), m_confidence(-1)
    {
    }

    CRegion(const cv::Rect& rect)
        : m_rect(rect)
    {

    }

    CRegion(const cv::Rect& rect, const std::string& type, float confidence)
        : m_rect(rect), m_type(type), m_confidence(confidence)
    {

    }

    cv::Rect m_rect;
    std::vector<cv::Point2f> m_points;

    std::string m_type;
    float m_confidence;
};

///
/// \brief The CRegion class
///
class CRect
{
public:
    CRect(const cv::Rect& rect)
        : m_rect(rect), is_used(false)
    {

    }

    cv::Rect m_rect;
    cv::Mat cropped_img;
    bool is_used;
    double area, intensity;
};

typedef struct trackinginfo{
    cv::Mat cropped_img;
    cv::Rect rect;
    int track_id;
    int class_id;
    int skip_frames;
    double area;
    double center[2], next_center[2];
    std::vector<double> centerPositions[2];  // 0: x, 1: y
    bool is_used_tracking, is_classified, is_sorted, is_not_tracked, is_time_to_remove, is_new_image ;
    double speed_x, speed_y;
    double sorting_nxt_pos;
    std::chrono::system_clock::time_point curtime;
    std::vector<std::chrono::system_clock::time_point> times;
    trackinginfo()
    {
        is_classified = false;
        is_sorted = false;
        is_not_tracked = false;
        is_time_to_remove = false;
        is_used_tracking = false;
        class_id = -1;
        speed_x = 0;
        speed_y = 0;
        skip_frames = 0;
        sorting_nxt_pos = 0;
    }
} Trackinginfo;

struct SortStruct {
    int idx, x_pos;
};

typedef std::vector<CRegion> regions_t;
typedef std::vector<CRect> rects_t;
typedef std::vector<Trackinginfo> tracking_t;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

typedef struct mytrack{
    int trackID;
    int m_clsNum;
    bool is_used, is_sorted;
    bool blnCurrentMatchFoundOrNewBlob, blnStillBeingTracked, remove_now;
    double speed, speed_x, speed_y;
    double intensity, area;
    double deltaX, deltaY;
    cv::Rect rect;
    double center[2], next_center[2];
    double initial_center[2]; //20201017, test
    double next_center_after_60ms[2], predicted_center[2];
    std::vector<double> centerPositions[2];  // 0: x, 1: y
    std::vector<int> skip_frame_cnt, skip_frame_camera_cnt;
    double diagonal;
    int skip_frame;
    int centerPositionsSize;
    //std::chrono::steady_clock::time_point begin;
    std::chrono::system_clock::time_point begin;
    std::chrono::system_clock::time_point initial_time;  //20201017, test
    std::vector<std::chrono::system_clock::time_point> times;

    mytrack(int ID, CRect crect, std::chrono::system_clock::time_point cur_time)
        : trackID(ID), m_clsNum(-1), is_used(false), is_sorted(false), speed(0.0), speed_x(0.0), speed_y(0.0), rect(crect.m_rect), begin(cur_time), deltaX(0), deltaY(0)
    {
        center[0] = (rect.x+rect.x+rect.width)/2;
        center[1] = (rect.y+rect.y+rect.height)/2;
        //center[0] = crect.pts.x;
        //center[1] = crect.pts.y;

        next_center[0] = center[0];
        next_center[1] = center[1];
        predicted_center[0] = center[0];
        predicted_center[1] = center[1];
        //next_center_after_60ms[0] = center[0];
        diagonal = sqrt(rect.width*rect.width + rect.height*rect.height);
        skip_frame = 0;
        blnCurrentMatchFoundOrNewBlob = true;
        blnStillBeingTracked = true;
        remove_now = false;
        centerPositions[0].push_back(center[0]); // x
        centerPositions[1].push_back(center[1]); // y
        skip_frame_cnt.push_back(skip_frame);
        skip_frame_camera_cnt.push_back(0);
        times.push_back(cur_time);
        centerPositionsSize = 0;
        area = crect.area;
        intensity = crect.intensity;
        initial_center[0] = (rect.x+rect.x+rect.width)/2;
        initial_center[1] = (rect.y+rect.y+rect.height)/2;
        initial_time = cur_time;
    }
} MyTrack;

typedef std::vector<std::unique_ptr<MyTrack>> mytracks_t;

///
///
///
namespace tracking
{
///
enum Detectors
{
    Motion_VIBE,
    Motion_MOG,
    Motion_GMG,
    Motion_CNT,
    Motion_SuBSENSE,
    Motion_LOBSTER,
    Motion_MOG2,
    Face_HAAR,
    Pedestrian_HOG,
    Pedestrian_C4,
    SSD_MobileNet,
    Yolo
};

///
/// \brief The DistType enum
///
enum DistType
{
    DistCenters = 0,
    DistRects = 1,
    DistJaccard = 2
    //DistLines = 3
};

///
/// \brief The FilterGoal enum
///
enum FilterGoal
{
    FilterCenter = 0,
    FilterRect = 1
};

///
/// \brief The KalmanType enum
///
enum KalmanType
{
    KalmanLinear = 0,
    KalmanUnscented = 1,
    KalmanAugmentedUnscented
};

///
/// \brief The MatchType enum
///
enum MatchType
{
    MatchHungrian = 0,
    MatchBipart = 1
};

///
/// \brief The LostTrackType enum
///
enum LostTrackType
{
    TrackNone = 0,
    TrackKCF = 1,
    TrackMIL,
    TrackMedianFlow,
    TrackGOTURN,
    TrackMOSSE
};
}
