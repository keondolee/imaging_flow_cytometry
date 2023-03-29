#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "tensorrt_window.h"
#include "mytracker_v2.h"
#include "myutils.h"
#include "configmanager.h"
#include "nicontrol.h"
#include <QtWidgets>
#include <ctime>
#include "dirent.h"
#include "tisudshl.h"		// IC Imaging Control Class Library
#include <mutex>
#include <iostream>
#include "SerialPort.hpp"
#include "smaract.h"
#include "photron_camera.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow, public DShowLib::FrameQueueSinkListener
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    typedef struct imginfo{
        cv::Mat img, img2;
        int skip_frames;
        std::chrono::system_clock::time_point curtime;
        imginfo()
        {

        }
    } ImgInfo;

    typedef struct imginfo2{
        cv::Mat img;
        std::chrono::system_clock::time_point curtime;
        imginfo2()
        {

        }
    } ImgInfo2;

    static std::queue<ImgInfo> cameraQ;          // Imaging Source camera
    static std::queue<ImgInfo> imageprocessingQ; // Imaging Source camera
    static std::queue<cv::Mat> croppedimagesQ;   // Imaging Source camera
    static std::queue<cv::Mat> cameraimagesQ;    // Imaging Source camera
    static std::deque<cv::Mat> cameraimagesQ3;    // Imaging Source camera

    static std::queue<cv::Mat> processedimagesQ; // Imaging Source camera
    static std::queue<QPixmap> screenimagesQ;    // Imaging Source camera
    static std::queue<ImgInfo> cameraQ2;         // Photron camera
    static std::queue<ImgInfo2> cameraimagesQ2;   // Photron camera

    virtual void    sinkConnected( DShowLib::FrameQueueSink& sink, const DShowLib::FrameTypeInfo& frameType );
    //////////////////////////////////////////////////////////////////////////
    /*! The framesQueued() method calls the saveImage method to save the image buffer to disk. */
    virtual void    framesQueued( DShowLib::FrameQueueSink& sink );
    void photroncameraThread();
    void imageprocessingThread(bool&, bool&, int&, int&, int&, int&, int&);
    void classificationThread();
    void sortingThread(int, ulong&, ulong&, double&, double&);
    void getcroppedimagesThread(std::string path);
    void getcameraimagesThread(std::string path);
    void getcameraimagesThread2(std::string path);
    void getprocessedimagesThread(std::string path);
    void getscreenimagesThread(std::string path);
    void debugThread(bool&, int&, int&, int, std::string, std::string);
    void generatepulseThread(double&, int&);
    void capture_screen();

private:
    Ui::MainWindow *ui;
    DShowLib::Grabber		m_cGrabber; // The instance of the Grabber class.
    DShowLib::tFrameQueueSinkPtr pSink;
    tensorrt_window *rt;
    //MyTracker *mytracker;
    mytracker_v2 *mytracker2;
    photron_camera *cam{NULL};
    NIControl *nicontrol;
    SmarAct *smaract;
    //SerialPort *arduino;
    ConfigManager cfg;
    std::mutex tracking_mutex, camera_mutex;
    std::vector<std::string> classListTensorRT;
    ulong *classListNum;
    ulong total_cell_cnt{0}, start_total_cell_cnt{0}, start_sorting_cell_cnt{0}, stop_sorting_cell_cnt{0};
    ulong sorted_cell_cnt1{0}, sorted_cell_cnt2{0};
    double eps{0}, max_eps{0};
    double processing_time_us{0};
    double system_latency_us{3000};
    int image_width{640}, image_height{480};
    int sorting_minutes{0}, sorting_seconds{0};
    double sorting_line1{500};
    cv::Rect roi{0,0,0,0}, roi_tmp{0,0,0,0};
    int click_cnt{0};
    bool setBackground{false}, setROI{false}, debugON{false};
    bool cancel_getcroppedimages{false}, cancel_getcameraimages{false}, cancel_getprocessedimages{false}, cancel_getscreenimages{false};
    bool cancel_getcameraimages2{false};
    rects_t rects;
    tracking_t trackings;
    std::vector<cv::Scalar> m_colors;
    std::chrono::system_clock::time_point global_time;
    std::chrono::system_clock::time_point trigger_time;
    std::chrono::system_clock::time_point start_eps_time, stop_eps_time;
    std::chrono::system_clock::time_point start_sorting_time, stop_sorting_time;

    const int MAX_CAMERA_IMG_SIZE = 15000;
    const int MAX_CAMERA_IMG_SIZE2 = 15000;
    const int MAX_CROPPED_IMG_SIZE = 7000;
    const int MAX_PROCESSED_IMG_SIZE = 15000;
    const int MAX_SCREEN_IMG_SIZE = 500;
    int debug_file_size{0}, debug_index{0};
    bool debug_clicked{false}, draw_clicked{true};

    // User parameters
    int filter_size_min{5};
    int filter_size_max{100};
    int threshold_value{10};
    int crop_size_width{50}, crop_size_height{50};
    int target_cell_no1{-1}, target_cell_no2{-1};
    int debug_fps{480}, gap_um{500}, pulse_repetition_interval{250};
    int sample_air_pressure{0};
    double pulse_on_voltage{1.0};
    double pulse_voltage1{1.0}, pulse_voltage2{1.0};

    // SmarAct
    int smaract_stepsize = 2; //um
    int smaract_velocity = 0; // um/ms

    char* portName = "\\\\.\\COM3";
    std::string debug_background_img_path{""};
    std::string debug_dir_path{""};

    void initCombobox();
    void initSpinbox();
    void initParameters(int, int);
    void initClassificationParameters();
    void initSortingParameters();
    void initDebugParameters();
    void updateCombobox();
    void updateCombobox2();
    void updateConfigs();
    void updateSystemLatency();

    void DrawMyTrack2(cv::Mat, int, const trackinginfo);
    void clear_queue(std::queue<cv::Mat> &q);
    void clear_queue(std::queue<QPixmap> &q);
    void clear_queue(std::queue<ImgInfo> &q);
    void clear_queue(std::queue<ImgInfo2> &q);

private slots:
    void on_cameraButton_clicked(bool checked);
    void on_cameraButton2_clicked(bool checked);
    void on_imageprocessingButton_clicked(bool checked);
    void on_classificationButton_clicked(bool checked);
    void on_sortingButton_clicked(bool checked);
    void on_pulseButton_clicked(bool checked);
    void on_drawprocessingresultsButton_clicked(bool checked);

    void on_cameraSettingButton_clicked();
    void on_camerapropertiesButton_clicked();

    void on_setbackgroundButton_clicked();
    void on_roiButton_clicked();
    void on_debugButton_clicked();
    void on_loadButton_clicked();

    void on_getcroppedimagesButton_clicked(bool checked);
    void on_getcameraimagesButton_clicked(bool checked);
    void on_getcameraimagesButton2_clicked(bool checked);
    void on_getprocessedimagesButton_clicked(bool checked);
    void on_getscreenimagesButton_clicked(bool checked);

    void on_getcroppedimagesDelButton_clicked();
    void on_getcameraimagesDelButton_clicked();
    void on_getprocessedimagesDelButton_clicked();
    void on_getscreenimagesDelButton_clicked();
    void on_getcameraimagesDelButton2_clicked();

    void on_forwardButton_clicked();
    void on_backwardButton_clicked();

    void updateDisplay();
    void updateDisplay2();
    void on_spinBox_filtersize_min_valueChanged(int);
    void on_spinBox_filtersize_max_valueChanged(int);
    void on_spinBox_cropsize_width_valueChanged(int);
    void on_spinBox_cropsize_height_valueChanged(int);
    void on_spinBox_threshold_value_valueChanged(int);
    void on_spinBox_target_cell_no1_valueChanged(int);
    void on_spinBox_target_cell_no2_valueChanged(int);
    void on_spinBox_debug_fps_valueChanged(int);
    void on_spinBox_gap_valueChanged(int);
    void on_spinBox_pulse_repetition_interval_valueChanged(int);
    void on_spinBox_air_pressure_valueChanged(int);
    void on_spinBox_system_latency_valueChanged(int);
    void on_spinBox_sortingline1_valueChanged(int);
    void on_doubleSpinBox_pulse_on_voltage_valueChanged(double);
    void on_doubleSpinBox_pulse_voltage1_valueChanged(double);
    void on_doubleSpinBox_pulse_voltage2_valueChanged(double);

    // SmarAct Control Box
    void on_spinBox_stepsize_valueChanged(int);
    void on_spinBox_velocity_valueChanged(int);


protected:
    void mousePressEvent(QMouseEvent *event);

};
#endif // MAINWINDOW_H
