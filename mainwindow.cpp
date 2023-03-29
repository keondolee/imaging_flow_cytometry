#include "mainwindow.h"
#include "./ui_mainwindow.h"

std::queue<MainWindow::ImgInfo> MainWindow::cameraQ;          // Imaging Source camera
std::queue<MainWindow::ImgInfo> MainWindow::imageprocessingQ; // Imaging Source camera
std::queue<cv::Mat> MainWindow::croppedimagesQ;               // Imaging Source camera
std::queue<cv::Mat> MainWindow::cameraimagesQ;                // Imaging Source camera
std::deque<cv::Mat> MainWindow::cameraimagesQ3;                // Imaging Source camera

std::queue<cv::Mat> MainWindow::processedimagesQ;             // Imaging Source camera
std::queue<QPixmap> MainWindow::screenimagesQ;                // Imaging Source camera
std::queue<MainWindow::ImgInfo> MainWindow::cameraQ2;         // Photron camera
std::queue<MainWindow::ImgInfo2> MainWindow::cameraimagesQ2;   // Photron camera

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    DShowLib::InitLibrary();

    atexit( DShowLib::ExitLibrary );

    DShowLib::Grabber::tVidCapDevListPtr pVidCapDevList = m_cGrabber.getAvailableVideoCaptureDevices();
    if( pVidCapDevList == 0 || pVidCapDevList->empty() )
    {
        std::cerr << " No camera available." << std::endl;
        return; // No device available.
    }

    m_cGrabber.openDev( "DFK 37BUX287" );
    m_cGrabber.loadDeviceStateFromFile("device.xml");

    // Create a FrameTypeInfoArray data structure describing the allowed color formats.
    DShowLib::FrameTypeInfoArray acceptedTypes = DShowLib::FrameTypeInfoArray::createRGBArray();

    // Create the frame sink
    pSink = DShowLib::FrameQueueSink::create(*this, acceptedTypes );

    // Apply the sink to the grabber.
    m_cGrabber.setSinkType( pSink );

    ui->setupUi(this);

    //arduino = new SerialPort(portName);
    //Checking if arduino is connected or not
    //if (!arduino->isConnected()) {
    //    std::cerr  << "Arduino is not connected at port " << portName << std::endl;
    //}

    nicontrol = new NIControl();
    mytracker2 = new mytracker_v2();
    //cam = new photron_camera();
    //cam->PrintCameraSetting();

    initCombobox();
    initDebugParameters();
    initSpinbox();

    rt = new tensorrt_window();
    classListTensorRT = rt->get_class_names();
    classListNum = new ulong[classListTensorRT.size()];
    for(int i=0; i< int(classListTensorRT.size()); i++){
        classListNum[i] = 0;
    }

    // Smaract linear stage
    //smaract = new SmarAct();


    //mytracker = new MyTracker(image_width, image_height);
    mytracker2->update_frame_interval(1000.0*1000.0/m_cGrabber.getFPS());  //?

    m_colors.push_back(cv::Scalar(255, 0, 0));
    m_colors.push_back(cv::Scalar(0, 255, 0));
    m_colors.push_back(cv::Scalar(255, 255, 255));
    m_colors.push_back(cv::Scalar(0, 0, 255));
    m_colors.push_back(cv::Scalar(255, 255, 0));
    m_colors.push_back(cv::Scalar(0, 255, 255));
    m_colors.push_back(cv::Scalar(255, 0, 255));
    m_colors.push_back(cv::Scalar(255, 127, 255));
    m_colors.push_back(cv::Scalar(127, 0, 255));
    m_colors.push_back(cv::Scalar(127, 0, 127));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
    timer->start(50); // every 40ms, 25fps

    // [ERROR] Image processing takes long time. img1C.w: 0, h: 0


    QTimer *timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(updateDisplay2()));
    timer2->start(50); // every 50ms, 20fps
}

MainWindow::~MainWindow()
{
    rt->teardown();
    delete rt;
    delete mytracker2;
    delete nicontrol;
    //delete arduino;
    delete ui;
}


void MainWindow::sinkConnected( DShowLib::FrameQueueSink& sink, const DShowLib::FrameTypeInfo& frameType )
{
    UNREFERENCED_PARAMETER( frameType );

    sink.allocAndQueueBuffers( 1 );
}


//////////////////////////////////////////////////////////////////////////
/*! The framesQueued() method calls the saveImage method to save the image buffer to disk.
*/
void MainWindow::framesQueued( DShowLib::FrameQueueSink& sink )
{
    DShowLib::tFrameQueueBufferPtr buffer = sink.popOutputQueueBuffer();
    // This function creates a number of buffers and puts them into the input queue.
    sink.allocAndQueueBuffers(1);

    //unsigned int frame_number = buffer->getFrameMetaData().mediaSampleDesc.FrameNumber;

    //std::cout << "Buffer " << frame_number << " processed in sink_listener::framesQueued()." << std::endl;
    //std::cout << "getInputQueueSize " << sink.getInputQueueSize() << std::endl;
    //std::cout << "getOutputQueueSize " << sink.getOutputQueueSize() << std::endl;

    unsigned char *livedata = buffer->getPtr();

    cv::Mat img(cv::Size(image_width, image_height), CV_8UC3, livedata, cv::Mat::AUTO_STEP);
    //cv::flip(img, img, 0); // 0.050ms, up-down

    if(cameraQ.size()<2 && img.rows > 1){
        ImgInfo camera;
        camera.img = img.clone();
        camera.curtime = std::chrono::system_clock::now();
        cameraQ.push(camera);
    }
    if(ui->getcameraimagesButton->isChecked() && int(cameraimagesQ.size()) < MAX_CAMERA_IMG_SIZE){
        cameraimagesQ.push(img.clone());
    }
}

void MainWindow::updateDisplay(){
    if(!cameraQ.empty()&&ui->cameraButton->isChecked()&&!ui->imageprocessingButton->isChecked()){
        ImgInfo camera = cameraQ.front();
        cv::Mat img = camera.img;
        cv::rectangle(img, cv::Point(roi.x,roi.y), cv::Point(roi.x+roi.width, roi.y+roi.height), cv::Scalar(255, 125, 0), 2, 8, 0);
        cv::line(img, cv::Point(sorting_line1,0), cv::Point(sorting_line1,image_height),   cv::Scalar(255, 125, 0), 2, cv::LINE_AA);
        QImage *qimg;
        if(img.channels() == 4){
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, QImage::Format_ARGB32);
        }else if(img.channels() == 3){
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA); // 4 channels -> 3 channels, RGB -> CNN model, BGR -> cv::imwrite -> RGB
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, QImage::Format_ARGB32);
        }else {
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_Grayscale8); //Format_Grayscale8
        }

        QPixmap temp = QPixmap::fromImage(*qimg);

        ui->camera_view->setPixmap(temp.scaled(ui->camera_view->width(),ui->camera_view->height(),Qt::KeepAspectRatio));
        delete qimg;

        cameraQ.pop();
    }else if(!imageprocessingQ.empty()&&ui->imageprocessingButton->isChecked()){
        ImgInfo imageprocessing = imageprocessingQ.front();
        cv::Mat img = imageprocessing.img.clone();
        cv::rectangle(img, cv::Point(roi.x,roi.y), cv::Point(roi.x+roi.width, roi.y+roi.height), cv::Scalar(255, 125, 0), 2, 8, 0);
        cv::line(img, cv::Point(sorting_line1,0), cv::Point(sorting_line1,image_height),   cv::Scalar(255, 125, 0), 2, cv::LINE_AA);
        QImage *qimg;
        if(img.channels() == 4){
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, QImage::Format_ARGB32);
        }else if(img.channels() == 3){
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA); // 4 channels -> 3 channels, RGB -> CNN model, BGR -> cv::imwrite -> RGB
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, QImage::Format_ARGB32);
        }else {
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_Grayscale8); //Format_Grayscale8
        }
        QPixmap temp = QPixmap::fromImage(*qimg);
        ui->camera_view->setPixmap(temp.scaled(ui->camera_view->width(),ui->camera_view->height(),Qt::KeepAspectRatio));
        delete qimg;

        cv::Mat img2 = imageprocessing.img2.clone();
        //cv::cvtColor(img2, img2, cv::COLOR_GRAY2BGRA); // 0.942055ms
        QImage *qimg2;
        if(img2.channels() == 4){
            qimg2 = new QImage((unsigned char*) img2.data, img2.cols, img2.rows, QImage::Format_ARGB32);
        }else if(img2.channels() == 3){
            cv::cvtColor(img2, img2, cv::COLOR_BGR2BGRA); // 4 channels -> 3 channels, RGB -> CNN model, BGR -> cv::imwrite -> RGB
            qimg2 = new QImage((unsigned char*) img2.data, img2.cols, img2.rows, QImage::Format_ARGB32);
        }else {
            qimg2 = new QImage((unsigned char*) img2.data, img2.cols, img2.rows, static_cast<int>(img2.step), QImage::Format_Grayscale8); //Format_Grayscale8
        }

        QPixmap temp2 = QPixmap::fromImage(*qimg2);
        ui->imageprocessing_view->setPixmap(temp2.scaled(ui->imageprocessing_view->width(),ui->imageprocessing_view->height(),Qt::KeepAspectRatio));
        delete qimg2;
        imageprocessingQ.pop();
    }

    if(ui->getscreenimagesButton->isChecked() && int(screenimagesQ.size()) < MAX_SCREEN_IMG_SIZE){
        capture_screen();
    }

    if(ui->getcroppedimagesButton->isChecked()){
        char status[40];
        sprintf(status, "%d/%d", int(croppedimagesQ.size()), MAX_CROPPED_IMG_SIZE);
        QString qstr = QString::fromStdString(std::string(status));
        ui->getcroppedimagesButton->setText(qstr);
    }
    if(ui->getcameraimagesButton->isChecked()){
        char status[40];
        sprintf(status, "%d/%d", int(cameraimagesQ.size()), MAX_CAMERA_IMG_SIZE);
        QString qstr = QString::fromStdString(std::string(status));
        ui->getcameraimagesButton->setText(qstr);
    }
    if(ui->getprocessedimagesButton->isChecked()){
        char status[40];
        sprintf(status, "%d/%d", int(processedimagesQ.size()), MAX_PROCESSED_IMG_SIZE);
        QString qstr = QString::fromStdString(std::string(status));
        ui->getprocessedimagesButton->setText(qstr);
    }
    if(ui->getscreenimagesButton->isChecked()){
        char status[40];
        sprintf(status, "%d/%d", int(screenimagesQ.size()), MAX_SCREEN_IMG_SIZE);
        QString qstr = QString::fromStdString(std::string(status));
        ui->getscreenimagesButton->setText(qstr);
    }

    if(debug_clicked){
        char status[40];
        sprintf(status, "Loading (%d/%d)", debug_index, debug_file_size);
        QString qstr = QString::fromStdString(std::string(status));
        ui->debugButton->setText(qstr);
    }else{
        ui->debugButton->setText("Debug");
    }

    // Update status
    std::string textbrowsers_text = "";

    // Update running time - sorting, sorted num
    if(ui->sortingButton->isChecked()){
        stop_sorting_cell_cnt = total_cell_cnt;
        stop_sorting_time = std::chrono::system_clock::now();
        double lapsed_time_s =  std::chrono::duration_cast<std::chrono::seconds>(stop_sorting_time - start_sorting_time).count();
        sorting_minutes = int(lapsed_time_s)/60;
        sorting_seconds = int(lapsed_time_s)%60;
        //textbrowsers_text += "Running time: " + std::to_string(minutes)+ " minutes "+std::to_string(seconds)+" seconds\n";
        //textbrowsers_text += "Sorted beads/cells NO1: " + std::to_string(sorted_cell_cnt1)+ ", NO2: "+std::to_string(sorted_cell_cnt2)+"\n";
    }


    if(ui->classificationButton->isChecked()){
        for(int i=0; i < int(classListTensorRT.size()); i++){
            textbrowsers_text += classListTensorRT.at(i);
            if(i == target_cell_no1){ textbrowsers_text += "[T1]"; }
            if(i == target_cell_no2){ textbrowsers_text += "[T2]"; }
            // else if (target_cell_no2
            textbrowsers_text += ": " + std::to_string(classListNum[i]) +" ("+ to_string_with_precision(classListNum[i]*100.0/total_cell_cnt, 1)+ "%)";
            if(i<int(classListTensorRT.size())-1){
                textbrowsers_text += ", ";
            }else{
                textbrowsers_text += "\n";
            }
        }
        stop_eps_time = std::chrono::system_clock::now();
        double lapsed_time_ms =  std::chrono::duration_cast<std::chrono::milliseconds>(stop_eps_time-start_eps_time).count();
        double throughput_update_interval_ms = 5000;
        if(lapsed_time_ms > throughput_update_interval_ms){
            start_eps_time = std::chrono::system_clock::now();
            eps = (total_cell_cnt-start_total_cell_cnt)*1000.0/throughput_update_interval_ms; //s
            start_total_cell_cnt = total_cell_cnt;
            if(eps > max_eps){max_eps = eps;}
        }
        double lapsed_time_s =  std::chrono::duration_cast<std::chrono::seconds>(stop_sorting_time - start_sorting_time).count();
        double sorting_eps = (stop_sorting_cell_cnt - start_sorting_cell_cnt)*1.0/lapsed_time_s; //s
        textbrowsers_text += "Throughput: " + to_string_with_precision(eps, 1)+ " eps, Max: "+to_string_with_precision(max_eps, 1)+" eps";
        textbrowsers_text += ", Sorting: "+to_string_with_precision(sorting_eps, 1)+" eps\n";
        textbrowsers_text += "Sorted target beads/cells: [T1] " + std::to_string(sorted_cell_cnt1) + ", [T2] " + std::to_string(sorted_cell_cnt2) +"\n";
        textbrowsers_text += "Running time: " + std::to_string(sorting_minutes)+ " minutes "+std::to_string(sorting_seconds)+" seconds\n";
    }

    if(ui->imageprocessingButton->isChecked()){
        textbrowsers_text += "Processing time: " + to_string_with_precision(processing_time_us/1000.0, 2)+ " ms\n";
    }

    ui->textBrowser->setText(textbrowsers_text.c_str());

}

void MainWindow::updateDisplay2(){
    if(!cameraQ2.empty()&&ui->cameraButton2->isChecked()){
        ImgInfo camera = cameraQ2.front();
        cv::Mat img = camera.img;

        QImage *qimg;
        if(img.channels() == 4){
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, QImage::Format_ARGB32);
        }else if(img.channels() == 3){
            cv::cvtColor(img, img, cv::COLOR_BGR2BGRA); // 4 channels -> 3 channels, RGB -> CNN model, BGR -> cv::imwrite -> RGB
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, QImage::Format_ARGB32);
        }else {
            qimg = new QImage((unsigned char*) img.data, img.cols, img.rows, static_cast<int>(img.step), QImage::Format_Grayscale8); //Format_Grayscale8
        }

        QPixmap temp = QPixmap::fromImage(*qimg);

        ui->camera_view2->setPixmap(temp.scaled(ui->camera_view2->width(),ui->camera_view2->height(),Qt::KeepAspectRatio));
        delete qimg;

        cameraQ2.pop();
    }
    if(ui->getcameraimagesButton2->isChecked()){
        char status[40];
        sprintf(status, "%d/%d", cameraimagesQ2.size(), MAX_CAMERA_IMG_SIZE2);
        QString qstr = QString::fromStdString(std::string(status));
        ui->getcameraimagesButton2->setText(qstr);
    }

}


void MainWindow::initCombobox(){
    if(cam != NULL){
    std::vector<std::string> framerate_list = cam->GetFramerateList();
    for(int i=0; i<(int)framerate_list.size(); i++){
        ui->comboBox_framerate->addItem(QString::fromStdString(framerate_list.at(i)));
    }
    std::vector<std::string> shutterspeed_list = cam->GetShutterspeedList();
    for(int i=0; i<(int)shutterspeed_list.size(); i++){
        ui->comboBox_shutterspeed->addItem(QString::fromStdString(shutterspeed_list.at(i)));
    }
    std::vector<std::string> resolution_list = cam->GetResolutionList();
    for(int i=0; i<(int)resolution_list.size(); i++){
        ui->comboBox_resolution->addItem(QString::fromStdString(resolution_list.at(i)));
    }

    ui->comboBox_framerate->setCurrentIndex(cam->GetFramerateListNum());
    ui->comboBox_shutterspeed->setCurrentIndex(cam->GetShutterspeedListNum());
    ui->comboBox_resolution->setCurrentIndex(cam->GetResolutionListNum());
    }
}

void MainWindow::initSpinbox(){
    // Get values from a config file
    filter_size_min = cfg.StringtoInt(cfg.search_conf("FILTER_SIZE_MIN"));
    filter_size_max = cfg.StringtoInt(cfg.search_conf("FILTER_SIZE_MAX"));
    threshold_value = cfg.StringtoInt(cfg.search_conf("THRESHOLD_VALUE"));
    crop_size_width = cfg.StringtoInt(cfg.search_conf("CROP_SIZE_WIDTH"));
    crop_size_height = cfg.StringtoInt(cfg.search_conf("CROP_SIZE_HEIGTH"));
    target_cell_no1 = cfg.StringtoInt(cfg.search_conf("TARGET_CELL_NO1"));
    target_cell_no2 = cfg.StringtoInt(cfg.search_conf("TARGET_CELL_NO2"));
    debug_fps = cfg.StringtoInt(cfg.search_conf("DEBUG_FPS"));
    gap_um = cfg.StringtoInt(cfg.search_conf("GAP_UM"));
    system_latency_us = cfg.StringtoInt(cfg.search_conf("SYSTEM_LATENCY_US"));
    sorting_line1 = cfg.StringtoDouble(cfg.search_conf("SORTING_LINE1"));
    pulse_repetition_interval = cfg.StringtoInt(cfg.search_conf("PULSE_REPETITION_INTERVAL"));
    sample_air_pressure = cfg.StringtoInt(cfg.search_conf("SAMPLE_AIR_PRESSURE"));
    pulse_on_voltage = cfg.StringtoDouble(cfg.search_conf("PULSE_ON_VOLTAGE"));
    pulse_voltage1 = cfg.StringtoDouble(cfg.search_conf("PULSE_VOLTAGE1"));
    pulse_voltage2 = cfg.StringtoDouble(cfg.search_conf("PULSE_VOLTAGE2"));

    ui->spinBox_filtersize_min->setValue(filter_size_min);
    ui->spinBox_filtersize_max->setValue(filter_size_max);
    ui->spinBox_threshold_value->setValue(threshold_value);
    ui->spinBox_cropsize_width->setValue(crop_size_width);
    ui->spinBox_cropsize_height->setValue(crop_size_height);
    ui->spinBox_target_cell_no1->setValue(target_cell_no1);
    ui->spinBox_target_cell_no2->setValue(target_cell_no2);
    ui->spinBox_debug_fps->setValue(debug_fps);
    ui->spinBox_gap->setValue(gap_um);
    ui->spinBox_system_latency->setValue(system_latency_us);
    ui->spinBox_sortingline1->setValue(sorting_line1);
    ui->spinBox_pulse_repetition_interval->setValue(pulse_repetition_interval);
    ui->spinBox_air_pressure->setValue(sample_air_pressure);
    ui->doubleSpinBox_pulse_on_voltage->setValue(pulse_on_voltage);
    ui->doubleSpinBox_pulse_voltage1->setValue(pulse_voltage1);
    ui->doubleSpinBox_pulse_voltage2->setValue(pulse_voltage2);
}

void MainWindow::initDebugParameters(){
    debug_background_img_path = cfg.search_conf("DEBUG_BACKGROUND_IMG_PATH");
    debug_dir_path = cfg.search_conf("DEBUG_DIR_PATH");
}

void MainWindow::updateCombobox(){

    int fps = m_cGrabber.getFPS();
    mytracker2->update_frame_interval(1000.0*1000.0/fps);  //?
    //updateSystemLatency();
}


void MainWindow::updateCombobox2(){
    // Frame rate error handling, if image size is higher, it takes much longer time to receive images.
    if(ui->comboBox_framerate->currentIndex() == 6 && ui->comboBox_resolution->currentIndex() < 1){
        // If fps is 2000fps(6), image size must be equal to or lower than 512x352-pixels
        ui->comboBox_resolution->setCurrentIndex(1);
    }else if(ui->comboBox_framerate->currentIndex() == 7 && ui->comboBox_resolution->currentIndex() < 2){
        // If fps is 3000fps(7), image size must be equal to or lower than 512x256-pixels
        ui->comboBox_resolution->setCurrentIndex(2);
    }else if(ui->comboBox_framerate->currentIndex() == 8 && ui->comboBox_resolution->currentIndex() < 3){
        // If fps is 4000fps(8), image size must be equal to or lower than 512x128-pixels
        ui->comboBox_resolution->setCurrentIndex(3);
    }else if(ui->comboBox_framerate->currentIndex() == 9 && ui->comboBox_resolution->currentIndex() < 3){
        // If fps is 5000fps(9), image size must be equal to or lower than 512x128-pixels
        ui->comboBox_resolution->setCurrentIndex(3);
    }else if(ui->comboBox_framerate->currentIndex() == 10 && ui->comboBox_resolution->currentIndex() < 3){
        // If fps is 6000fps(10), image size must be equal to or lower than 512x128-pixels
        ui->comboBox_resolution->setCurrentIndex(3);
    }else if(ui->comboBox_framerate->currentIndex() == 11 && ui->comboBox_resolution->currentIndex() < 4){
        // If fps is 7000fps(11), image size must be equal to or lower than 512x96-pixels
        ui->comboBox_resolution->setCurrentIndex(4);
    }else if(ui->comboBox_framerate->currentIndex() == 12 && ui->comboBox_resolution->currentIndex() < 4){
        // If fps is 8000fps(12), image size must be equal to or lower than 512x96-pixels
        ui->comboBox_resolution->setCurrentIndex(4);
    }else if(ui->comboBox_framerate->currentIndex() == 13 && ui->comboBox_resolution->currentIndex() < 4){
        // If fps is 9000fps(13), image size must be equal to or lower than 512x96-pixels
        ui->comboBox_resolution->setCurrentIndex(4);
    }else if(ui->comboBox_framerate->currentIndex() == 14 && ui->comboBox_resolution->currentIndex() < 5){
        // If fps is 10000fps(14), image size must be equal to or lower than 512x64-pixels
        ui->comboBox_resolution->setCurrentIndex(5);
    }

    // Shutter speed error handling
    if(ui->comboBox_framerate->currentIndex() == 6 && ui->comboBox_shutterspeed->currentIndex() < 3){
        // If fps is 2000fps, shutter speed should be lower than 1/2000s.
        ui->comboBox_shutterspeed->setCurrentIndex(3);
    }else if(ui->comboBox_framerate->currentIndex() >= 7 && ui->comboBox_framerate->currentIndex() < 9 && ui->comboBox_shutterspeed->currentIndex() < 6){
        // If fps is higher than 3000fps and lower than 5000fps, shutter speed should be lower than 1/4000s.
        ui->comboBox_shutterspeed->setCurrentIndex(6);
    }else if(ui->comboBox_framerate->currentIndex() == 9 && ui->comboBox_shutterspeed->currentIndex() < 7){
        // If fps is 5000fps, shutter speed should be lower than 1/5000s.
        ui->comboBox_shutterspeed->setCurrentIndex(7);
    }else if(ui->comboBox_framerate->currentIndex() == 10 && ui->comboBox_shutterspeed->currentIndex() < 9){
        // If fps is 6000fps, shutter speed should be lower than 1/6400s.
        ui->comboBox_shutterspeed->setCurrentIndex(9);
    }else if(ui->comboBox_framerate->currentIndex() >= 11 && ui->comboBox_framerate->currentIndex() < 13 && ui->comboBox_shutterspeed->currentIndex() < 10){
        // If fps is higher than 7000fps and lower than 9000fps, shutter speed should be lower than 1/8000s.
        ui->comboBox_shutterspeed->setCurrentIndex(10);
    }else if(ui->comboBox_framerate->currentIndex() >= 13 && ui->comboBox_shutterspeed->currentIndex() < 11){
        // If fps is higher than 9000fps, shutter speed should be lower than 1/10000s.
        ui->comboBox_shutterspeed->setCurrentIndex(11);
    }
    if(cam != NULL){
    cam->SetFramerateListNum(ui->comboBox_framerate->currentIndex());
    cam->SetShutterspeedListNum(ui->comboBox_shutterspeed->currentIndex());
    cam->SetResolutionListNum(ui->comboBox_resolution->currentIndex());
    }

}


void MainWindow::updateSystemLatency(){
    // Set system latency
    // 512x512x1px => 1800us
    // 512x256x1px => 1600us
    // 512x128x1px => 1480us
    // 512x96x1px => 1450us
    // Image size =====> system_latency
    //system_latency_us = 3000;
    ui->spinBox_system_latency->setValue(system_latency_us);
}

void MainWindow::initParameters(int w, int h){
    // Set background image, to prevent error when image size was changed.
    setBackground = true;
    roi = {0,0,0,0};
    // Update image width and height
    image_width = w;
    image_height = h;
    initClassificationParameters();
    initSortingParameters();
}

void MainWindow::initClassificationParameters(){
    total_cell_cnt = 0;
    start_total_cell_cnt = 0;
    for(int i=0; i< int(classListTensorRT.size()); i++){
        classListNum[i] = 0;
    }
    start_eps_time = std::chrono::system_clock::now();
    eps = 0;
    max_eps = 0;
}


void MainWindow::initSortingParameters(){
    start_sorting_cell_cnt = 0;
    stop_sorting_cell_cnt = 0;
    sorted_cell_cnt1 = 0;
    sorted_cell_cnt2 = 0;
}


void MainWindow::updateConfigs(){
    cfg.update_conf(system_latency_us, filter_size_min, filter_size_max, crop_size_width, crop_size_height, threshold_value, target_cell_no1, target_cell_no2, debug_fps, gap_um, pulse_repetition_interval, sorting_line1, sample_air_pressure, pulse_on_voltage, pulse_voltage1, pulse_voltage2, debug_background_img_path, debug_dir_path);
}

void MainWindow::capture_screen()
{
    auto active_window = qApp->activeWindow();
    if (active_window) //could be null if your app doesn't have focus
    {
        QPixmap pixmap(active_window->size());
        active_window->render(&pixmap);
        screenimagesQ.push(pixmap);
    }
}

void MainWindow::on_spinBox_filtersize_min_valueChanged(int arg1){
    filter_size_min = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_filtersize_max_valueChanged(int arg1){
    filter_size_max = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_cropsize_width_valueChanged(int arg1){
    crop_size_width = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_cropsize_height_valueChanged(int arg1){
    crop_size_height = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_threshold_value_valueChanged(int arg1){
    threshold_value = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_target_cell_no1_valueChanged(int arg1){
    target_cell_no1 = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_target_cell_no2_valueChanged(int arg1){
    target_cell_no2 = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_debug_fps_valueChanged(int arg1){
    debug_fps = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_gap_valueChanged(int arg1){
    gap_um = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_pulse_repetition_interval_valueChanged(int arg1){
    pulse_repetition_interval = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_air_pressure_valueChanged(int arg1){
    sample_air_pressure = arg1;
    updateConfigs();
    double input_voltage = sample_air_pressure/100.0*10.0;
    //std::cerr << input_voltage << std::endl;
    nicontrol->NI_WRITE2(input_voltage);
}

void MainWindow::on_spinBox_sortingline1_valueChanged(int arg1){
    sorting_line1 = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_system_latency_valueChanged(int arg1){
    system_latency_us = arg1;
    updateConfigs();
}

void MainWindow::on_spinBox_stepsize_valueChanged(int arg1){
    smaract_stepsize = arg1;
}

void MainWindow::on_spinBox_velocity_valueChanged(int arg1){
    smaract_velocity = arg1;
}

void MainWindow::on_doubleSpinBox_pulse_on_voltage_valueChanged(double arg1){
    pulse_on_voltage = arg1;
    updateConfigs();
}

void MainWindow::on_doubleSpinBox_pulse_voltage1_valueChanged(double arg1){
    pulse_voltage1 = arg1;
    updateConfigs();
}

void MainWindow::on_doubleSpinBox_pulse_voltage2_valueChanged(double arg1){
    pulse_voltage2 = arg1;
    updateConfigs();
}

void MainWindow::on_cameraButton_clicked(bool checked)
{
    if(checked){
        if( m_cGrabber.isDevValid()){
            updateCombobox();
            initParameters(m_cGrabber.getVideoFormat().getSize().cx, m_cGrabber.getVideoFormat().getSize().cy);
            m_cGrabber.startLive(false); // framesQueued starts to receive images. false: do not display img, true: display it
            // Camera info
            //std::cout << m_cGrabber.getFPS() <<"fps" << std::endl;
        }
        ui->cameraButton->setText("Camera Off");
    }else{
        // If live video is running, stop it.
        if(m_cGrabber.isDevValid() && m_cGrabber.isLive())
        {
            m_cGrabber.stopLive();
        }
        ui->cameraButton->setText("Camera On");
    }
}

void MainWindow::on_cameraButton2_clicked(bool checked)
{
    if(checked){
        updateCombobox2();
        //initParameters(cam->GetImageWidth(), cam->GetImageHeight());
        std::thread cameraTh(&MainWindow::photroncameraThread, this);
        cameraTh.detach();
        ui->cameraButton2->setText("Camera Off");
    }else{
        ui->cameraButton2->setText("Camera On");

    }
}

void MainWindow::on_imageprocessingButton_clicked(bool checked)
{
    if(checked){
        // Reset tracker
        mytracker2->reset_previous_tracking();
        std::thread imageprocessingTh(&MainWindow::imageprocessingThread, this, std::ref(draw_clicked), std::ref(setBackground), std::ref(filter_size_min), std::ref(filter_size_max), std::ref(threshold_value), std::ref(crop_size_width), std::ref(crop_size_height));
        imageprocessingTh.detach();
        ui->imageprocessingButton->setText("Image Processing Off");
    }else{
        ui->imageprocessingButton->setText("Image Processing On");
    }
}

void MainWindow::on_classificationButton_clicked(bool checked)
{
    if(checked){
        std::thread classificationTh(&MainWindow::classificationThread, this);
        classificationTh.detach();
        ui->classificationButton->setText("Classification Off");
    }else{
        ui->classificationButton->setText("Classification On");
    }
}

void MainWindow::on_sortingButton_clicked(bool checked)
{
    if(checked){
        start_sorting_time = std::chrono::system_clock::now();
        start_sorting_cell_cnt = total_cell_cnt;
        sorted_cell_cnt1 = 0;
        sorted_cell_cnt2 = 0;
        std::thread sortingTh(&MainWindow::sortingThread, this, gap_um, std::ref(sorted_cell_cnt1), std::ref(sorted_cell_cnt2), std::ref(sorting_line1), std::ref(system_latency_us));
        sortingTh.detach();
        ui->sortingButton->setText("Sorting Off");
    }else{
        ui->sortingButton->setText("Sorting On");
    }
}

void MainWindow::on_cameraSettingButton_clicked(){
    // Camera Setting
    m_cGrabber.showDevicePage(); // Select a video capture device
    if( m_cGrabber.isDevValid())
    {
        m_cGrabber.saveDeviceStateToFile("device.xml");
    }
}

void MainWindow::on_camerapropertiesButton_clicked(){
    // Camera Setting
    m_cGrabber.showVCDPropertyPage();
    if( m_cGrabber.isDevValid())
    {
        m_cGrabber.saveDeviceStateToFile("device.xml");
    }
}

void MainWindow::photroncameraThread()
{
    while(ui->cameraButton2->isChecked()){
        cv::Mat img = cam->GetLiveImage();
        if(img.rows <= 1) continue;
        if(cameraQ2.size()<2){
            ImgInfo camera;
            camera.img = img.clone();
            camera.curtime = std::chrono::system_clock::now();
            cameraQ2.push(camera);
        }
        if(ui->getcameraimagesButton2->isChecked() && cameraimagesQ2.size() < MAX_CAMERA_IMG_SIZE2){
            ImgInfo2 camera;
            camera.img = img.clone();
            camera.curtime = std::chrono::system_clock::now();
            cameraimagesQ2.push(camera); // problem
            //cameraimagesQ3.push(img.clone());
            //cameraimagesQ3.push_back(img.clone());
        }
    }
}


void MainWindow::imageprocessingThread(bool& draw_clicked, bool& setBackground, int& filter_size_min, int& filter_size_max, int& threshold_value, int& crop_size_width, int& crop_size_height)
{
    int boundx1, boundy1, boundx2, boundy2;
    std::chrono::system_clock::time_point cur_time;
    cv::Mat img3C, img1C, threshImg, subImg, roiImg, backgroundImg, morImg, outImg;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    setBackground = true;
    //std::ofstream outfile;
    //outfile.open("processing_time.txt", std::ios_base::app);

    while(ui->imageprocessingButton->isChecked()){
        if(!cameraQ.empty()){
            std::chrono::system_clock::time_point start_processing_time = std::chrono::system_clock::now();
            ImgInfo camera = cameraQ.front();
            cameraQ.pop();

            if(camera.img.rows == 0 || camera.img.cols == 0){
                std::cerr << "[ERROR] Image processing takes long time. img1C.w: " << camera.img.cols << ", h: " << camera.img.rows << std::endl;
                continue;
            }
            img3C = camera.img.clone(); //?
            // If img channel is one channel.
            cv::cvtColor(img3C, img1C, cv::COLOR_RGB2GRAY); // it is necessary. Bead : same results, Cancer : different results
            outImg = camera.img.clone();
            cur_time = camera.curtime;

            if(setBackground){
                backgroundImg = img1C.clone();
                setBackground = false;
            }

            cv::absdiff(img1C, backgroundImg, subImg);

            if(roi.width > 0 && roi.height > 0){
                roiImg = subImg(roi);
                boundx1 = roi.x;
                boundy1 = roi.y;
                boundx2 = roi.x + roi.width;
                boundy2 = roi.y + roi.height;
                mytracker2->update_boundingbox(boundx1, boundy1, boundx2, boundy2);
            }else{
                roiImg = subImg;
                boundx1 = 0;
                boundy1 = 0;
                boundx2 = image_width;
                boundy2 = image_height;
                mytracker2->update_boundingbox(boundx1, boundy1, boundx2, boundy2);
            }

            cv::GaussianBlur(roiImg, roiImg, cv::Size(3,3), 0);  // he didn't use it.

            cv::threshold(roiImg, threshImg, threshold_value, 255, cv::THRESH_BINARY); // Setting  blurImg->subImg

            cv::morphologyEx(threshImg, morImg, 1, element);  // 1:Opening, 2:Closing, Remove noises
            //cv::erode(morImg,morImg, element, cv::Point(-1, -1), 1); // cell

            std::vector<std::vector<cv::Point>> contours;

            cv::findContours(morImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            for( unsigned int i = 0; i < contours.size(); i++ ){
                cv::Rect rect = cv::boundingRect( contours[i] );  // I removed cv:Mat(contours_poly)

                if(rect.width < filter_size_min || rect.height < filter_size_min || rect.width > filter_size_max || rect.height > filter_size_max){
                    continue;
                }
                //std::cout << rect.width << std::endl;
                if(roi.width > 0 && roi.height >0){
                    rect.x += roi.x;
                    rect.y += roi.y;
                }

                Trackinginfo trackinginfo;
                trackinginfo.rect = rect;

                int cx = (int)(rect.x+rect.width/2);
                int cy = (int)(rect.y+rect.height/2);

                if( cx - crop_size_width/2 >= boundx1 && cx - crop_size_width/2 + crop_size_width <= boundx2 && cy - crop_size_height/2 >= boundy1 && cy - crop_size_height/2 + crop_size_height <= boundy2){
                    cv::Rect acqRect = cv::Rect(cx - crop_size_width/2, cy - crop_size_height/2, crop_size_width, crop_size_height);
                    cv::Mat croppedimage = img3C(acqRect);
                    trackinginfo.cropped_img = croppedimage.clone();
                    if(ui->getcroppedimagesButton->isChecked() && int(croppedimagesQ.size()) < MAX_CROPPED_IMG_SIZE){
                        croppedimagesQ.push(croppedimage);
                    }
                }

                //Test
                //cv::rectangle(outImg, rect, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
                double area = cv::contourArea(contours[i]);
                trackinginfo.area = area;
                trackinginfo.curtime = camera.curtime;
                trackings.push_back(trackinginfo);
            }
            tracking_mutex.lock();
            mytracker2->update_previous_trackings(trackings);
            tracking_mutex.unlock();

            contours.clear();
            trackings.clear();

            if(draw_clicked){
                for (const auto& track2 : mytracker2->get_previous_trackings()){
                    if(track2.is_not_tracked) continue;
                    if(track2.skip_frames > 0) continue;
                    DrawMyTrack2(outImg, 1, track2); //Time consuming
                }
            }

            if(ui->getprocessedimagesButton->isChecked() && int(processedimagesQ.size()) < MAX_PROCESSED_IMG_SIZE){
                // display current time
                double lapsed_time =  int(std::chrono::duration_cast<std::chrono::microseconds>(camera.curtime-global_time).count());
                std::stringstream stream;
                stream << std::fixed << std::setprecision(2) << lapsed_time/1000.0;
                std::string s = stream.str();
                cv::putText(outImg, //target image
                            s+" ms", //text
                            cv::Point(10, 30),
                            cv::FONT_HERSHEY_DUPLEX,
                            0.7,
                            m_colors[3], //font color
                        1);
                if(ui->pulseButton->isChecked()){
                    double lapsed_time2 =  int(std::chrono::duration_cast<std::chrono::microseconds>(trigger_time-global_time).count());
                    std::stringstream stream2;
                    stream2 << std::fixed << std::setprecision(2) << lapsed_time2/1000.0;
                    cv::putText(outImg, //target image
                                "[T] "+stream2.str()+" ms", //text
                                cv::Point(10, 60),
                                cv::FONT_HERSHEY_DUPLEX,
                                0.7,
                                m_colors[3], //font color
                            1);
                }
                processedimagesQ.push(outImg);
            }

            if(imageprocessingQ.size()<2){
                ImgInfo imageprocessing;
                imageprocessing.img = outImg.clone(); // processed image
                imageprocessing.img2 = morImg.clone();
                imageprocessingQ.push(imageprocessing);
            }

            std::chrono::system_clock::time_point stop_processing_time = std::chrono::system_clock::now();
            processing_time_us =  std::chrono::duration_cast<std::chrono::microseconds>(stop_processing_time - cur_time).count();
            //outfile << processing_time_us << "\n"; // test
        }
    }
    //outfile.close();

}

void MainWindow::classificationThread()
{
   //std::ofstream outfile;
   //outfile.open("processing_inference_time3.txt", std::ios_base::app);
   // Update continously if threshold < 0.8 (Unnecessary computation)
    while(ui->classificationButton->isChecked()){
        tracking_mutex.lock();
        tracking_t tmp(mytracker2->get_previous_trackings());
        tracking_mutex.unlock();
        for (auto track2 : tmp){
            if(track2.is_not_tracked) continue;
            if(!track2.is_classified && !track2.cropped_img.empty()){
                //auto t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                int class_id = rt->predict(track2.cropped_img.clone());
                //auto t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                //double t_comp = (t_check2- t_check1);///1000.0;
                //outfile << t_comp << "\n";
                //tracking_mutex.lock();
                //mytracker2->update_class_id(track2.track_id, class_id);
                //tracking_mutex.unlock();
                if(class_id != -1){
                    tracking_mutex.lock();
                    mytracker2->update_class_id(track2.track_id, class_id);
                    tracking_mutex.unlock();
                    //auto classified_time = std::chrono::system_clock::now(); // test
                    //double t_comp = std::chrono::duration_cast<std::chrono::microseconds>(classified_time - track2.curtime).count(); // test
                    //outfile << t_comp << "\n"; // test
                    //std::cout << t_comp << std::endl;
                    classListNum[class_id]++; total_cell_cnt++;
                }
            }

        }
    }
   //outfile.close();
}

void MainWindow::sortingThread(int gap_um, ulong& sorted_cell_cnt1, ulong& sorted_cell_cnt2, double& sorting_line1, double& system_latency_us)
{
    // 512x512x1px => 1800us
    // 512x256x1px => 1600us
    // 512x128x1px => 1480us
    // 512x96x1px => 1450us
    //std::ofstream outfile;
    //outfile.open("processing_time.txt", std::ios_base::app);

    //double sum_um_per_ms = 0.0; int cnt_um_per_ms = 0;  // measure_speed code
    // Inverted_microscope : um_to_px_10X 1.472504 - gap 970um
    // Cell sorter v2 : um_to_px_10X 1.5662 - gap 984um
    // Align the sorting line with the right end of the square.
    // gap 2000um - 1968um
    //     1500um - 1476um
    //     1000um - 984um
    //      500um - 492um

    const double um_to_px_10X = 1.5662;
    //double system_latency = system_latency_us; //us
    double gap_px = gap_um*um_to_px_10X; //px, 490.7px = 500um
    double sorting_width_px = 10; //px
    double line1 = sorting_line1+gap_px;
    double line2 = sorting_line1+sorting_width_px+gap_px;
    //nicontrol->NI_INIT_SQUARE_PULSE2(1.0);
    nicontrol->NI_INIT_SAWTOOTH_PULSE2(1.0);
    while(ui->sortingButton->isChecked()){
        tracking_mutex.lock();
        tracking_t tmp(mytracker2->get_previous_trackings());
        tracking_mutex.unlock();
        for (auto track2 : tmp){
            //double um_per_ms = track2.speed_x*1000.0/um_to_px_10X;  // measure_speed code
            //sum_um_per_ms += um_per_ms;  // measure_speed code
            //cnt_um_per_ms++;  // measure_speed code
            if(track2.is_sorted) continue;
            //if(!track2.is_classified) continue;
            if(track2.class_id == -1) continue;
            if(track2.class_id != target_cell_no1 && track2.class_id != target_cell_no2) continue;
            // calculate current position & next position
            double travel_distance = 0;
            auto end = std::chrono::system_clock::now();
            travel_distance = track2.speed_x*std::chrono::duration_cast<std::chrono::microseconds>(end - track2.curtime).count();
            double cur_pos = track2.center[0] + travel_distance;
            double nxt_pos = cur_pos + track2.speed_x*system_latency_us;

            // Test
            //tracking_mutex.lock();
            //mytracker2->update_nxt_pos(track2.track_id, nxt_pos);
            //tracking_mutex.unlock();

            if(nxt_pos > line1 && nxt_pos < line2){
                if(track2.class_id == target_cell_no1){
                    //nicontrol->NI_GENERATE_SQUARE_PULSE2(pulse_voltage1);
                    nicontrol->NI_GENERATE_SAWTOOTH_PULSE2(pulse_voltage1);
                    //arduino->writeSerialPort("1", 1);
                    sorted_cell_cnt1++;
                }
                if(track2.class_id == target_cell_no2){
                    nicontrol->NI_GENERATE_SAWTOOTH_PULSE2(pulse_voltage2);
                    sorted_cell_cnt2++;
                }
                tracking_mutex.lock();
                mytracker2->update_sort_status(track2.track_id);
                tracking_mutex.unlock();
            }
        }
    }
    //std::cout << "Avg speed_x (um/ms): " << sum_um_per_ms/cnt_um_per_ms << std::endl; //measure_speed code

}

void MainWindow::generatepulseThread(double& voltage, int& pulse_repetition_interval){
    nicontrol->NI_INIT_SAWTOOTH_PULSE2(1.0);
    //int cnt = 1;
    while(ui->pulseButton->isChecked()){
        trigger_time = std::chrono::system_clock::now();
        nicontrol->NI_GENERATE_SAWTOOTH_PULSE2(voltage);
        //double voltage2 =  voltage*(cnt++%2);
        //nicontrol->NI_WRITE3(voltage2);
        //arduino->writeSerialPort("1", 1);
        Sleep(pulse_repetition_interval); //ms
    }
}

void MainWindow::debugThread(bool& debug_clicked, int& debug_index, int& debug_file_size, int debug_fps, std::string dir_path, std::string background_img_path){
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files;
    if ((dir = opendir(dir_path.c_str())) != NULL) {
      debug_clicked = true;

      //ui->debugButton->setText("Loading...");
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
        std::string str = ent->d_name;
        std::size_t found=str.find(".bmp"); //bmp
        std::size_t found2=str.find(".jpg"); //bmp
        if (found!=std::string::npos||found2!=std::string::npos){
            files.push_back(dir_path+"/"+ent->d_name);
        }
      }
      closedir (dir);
      debug_file_size = int(files.size());
      std::sort(files.begin(),files.end());
      clear_queue(cameraQ);
      // Set a background image
      cv::Mat background_img = cv::imread(background_img_path, cv::IMREAD_COLOR);
      initParameters(background_img.cols, background_img.rows);
      //Set frame rate
      double framerate = debug_fps; // fps
      double frame_interval = 1000.0*1000.0/framerate;
      mytracker2->update_frame_interval(frame_interval);  //for tracking

      double t_check1, t_check2;
      std::queue<ImgInfo> tmpQ;   // Image processing

      //t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
      for(int i=0; i<(int)files.size(); i++){
         cv::Mat img = cv::imread(files[i], cv::IMREAD_COLOR);
         ImgInfo debug;
         debug.img = img;
         //debug.curtime = std::chrono::system_clock::now();
         tmpQ.push(debug);
         //cameraQ.push(debug);
         //debug_index = i;
         //if(i%50 == 0){
         //    char status[40];
         //    sprintf(status, "Loading (%d/%d)", i, int(files.size()));
         //    QString qstr = QString::fromStdString(std::string(status));
         //    ui->debugButton->setText(qstr);
         //}
         //t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
         //while(t_check2 - t_check1 < frame_interval){
         //    t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
         //}
         //t_check1 = t_check2;
      }
      ImgInfo backgroundInfo;
      backgroundInfo.img = background_img;
      backgroundInfo.curtime = std::chrono::system_clock::now();
      cameraQ.push(backgroundInfo);
      global_time = std::chrono::system_clock::now();

      int idx = 1;
      t_check1 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
      while(!tmpQ.empty()){
          ImgInfo debug;
          debug.img = tmpQ.front().img;
          debug.curtime = std::chrono::system_clock::now();
          cameraQ.push(debug);
          tmpQ.pop();
          debug_index = idx++;
          t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
          while(t_check2 - t_check1 < frame_interval){
              t_check2 =  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
          }
          t_check1 = t_check2;
      }
      //ui->debugButton->setText("Waiting...");
      while(!cameraQ.empty()){
          Sleep(10);
      }
      //ui->debugButton->setText("Debug");
      debug_clicked = false;
      debug_index = 0;
    } else {
      /* could not open directory */
      std::cerr << "Could not open directory" << std::endl;
    }
}

void MainWindow::getcroppedimagesThread(std::string path){
    ui->getcroppedimagesButton->setText("Saving ...");
    int i=1, size = int(croppedimagesQ.size());
    while(croppedimagesQ.size()>0 && !cancel_getcroppedimages){
        cv::Mat img = croppedimagesQ.front();
        char filename[40];
        sprintf(filename, "/img%08d.bmp", i);
        std::string fullPath = path+std::string(filename);
        // imread() will internally convert from rgb to bgr, and imwrite() will do the opposite,
        cv::imwrite(fullPath, img );  // img should be in BGR order, BGR->RGB
        croppedimagesQ.pop();
        if( i%100 == 0){
            char status[40];
            sprintf(status, "%d/%d", i, size);
            QString qstr = QString::fromStdString(std::string(status));
            ui->getcroppedimagesButton->setText(qstr);
        }
        i+=1;
    }
    clear_queue(croppedimagesQ);
    ui->getcroppedimagesButton->setText("Get cropped images");
}

void MainWindow::getcameraimagesThread(std::string path){
    ui->getcameraimagesButton->setText("Saving ...");
    int i=1, size = int(cameraimagesQ.size());
    while(cameraimagesQ.size()>0 && !cancel_getcameraimages){
        cv::Mat img = cameraimagesQ.front();
        char filename[40];
        sprintf(filename, "/img%08d.bmp", i);
        std::string fullPath = path+std::string(filename);
        // imread() will internally convert from rgb to bgr, and imwrite() will do the opposite,
        cv::imwrite(fullPath, img );  // img should be in BGR order, BGR->RGB
        cameraimagesQ.pop();
        if( i%100 == 0){
            char status[40];
            sprintf(status, "%d/%d", i, size);
            QString qstr = QString::fromStdString(std::string(status));
            ui->getcameraimagesButton->setText(qstr);
        }
        i+=1;
    }
    clear_queue(cameraimagesQ);
    ui->getcameraimagesButton->setText("Get camera images");
}

void MainWindow::getcameraimagesThread2(std::string path){
    ui->getcameraimagesButton2->setText("Saving ...");
    int i=1, size = int(cameraimagesQ2.size());
    while(cameraimagesQ2.size()>0 && !cancel_getcameraimages2){
        ImgInfo2 camera = cameraimagesQ2.front();
        cv::Mat img = camera.img.clone();
        if(camera.img.rows == 0 || camera.img.cols == 0){
            std::cerr << "[ERROR] Image processing takes long time. img1C.w: " << camera.img.cols << ", h: " << camera.img.rows << std::endl;
            continue;
        }
        char filename[40];
        sprintf(filename, "/img%08d.jpg", i);
        std::string fullPath = path+std::string(filename);


        // display current time
        double lapsed_time =  int(std::chrono::duration_cast<std::chrono::microseconds>(camera.curtime-global_time).count());
        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << lapsed_time/1000.0;
        std::string s = stream.str();
        cv::putText(img, //target image
                    s+" ms", //text
                    cv::Point(10, 30),
                    cv::FONT_HERSHEY_DUPLEX,
                    0.7,
                    m_colors[3], //font color
                1);


        // imread() will internally convert from rgb to bgr, and imwrite() will do the opposite,
        cv::imwrite(fullPath, img );  // img should be in BGR order, BGR->RGB
        cameraimagesQ2.pop();
        if( i%100 == 0){
            char status[40];
            sprintf(status, "%d/%d", i, size);
            QString qstr = QString::fromStdString(std::string(status));
            ui->getcameraimagesButton2->setText(qstr);
        }
        i+=1;
    }
    clear_queue(cameraimagesQ2);
    ui->getcameraimagesButton2->setText("Get camera images");
}


void MainWindow::getprocessedimagesThread(std::string path){
    ui->getprocessedimagesButton->setText("Saving ...");
    int i=1, size = int(processedimagesQ.size());
    while(processedimagesQ.size()>0 && !cancel_getprocessedimages){
        cv::Mat img = processedimagesQ.front();
        char filename[40];
        sprintf(filename, "/img%08d.jpg", i);
        std::string fullPath = path+std::string(filename);
        // imread() will internally convert from rgb to bgr, and imwrite() will do the opposite,
        cv::imwrite(fullPath, img );  // img should be in BGR order, BGR->RGB
        processedimagesQ.pop();
        if( i%100 == 0){
            char status[40];
            sprintf(status, "%d/%d", i, size);
            QString qstr = QString::fromStdString(std::string(status));
            ui->getprocessedimagesButton->setText(qstr);
        }
        i+=1;
    }
    clear_queue(processedimagesQ);
    ui->getprocessedimagesButton->setText("Get processed images");
}

void MainWindow::getscreenimagesThread(std::string path){
    ui->getscreenimagesButton->setText("Saving ...");
    int i=1, size = int(screenimagesQ.size());
    while(screenimagesQ.size()>0 && !cancel_getscreenimages){
        QPixmap pixmap = screenimagesQ.front();
        char filename[40];
        sprintf(filename, "/img%08d.png", i);
        std::string fullPath = path+std::string(filename);
        QFile file(fullPath.c_str());
        file.open(QIODevice::WriteOnly);
        pixmap.save(&file,"PNG");
        screenimagesQ.pop();
        if(i%100 == 0){
            char status[40];
            sprintf(status, "%d/%d", i, size);
            QString qstr = QString::fromStdString(std::string(status));
            ui->getscreenimagesButton->setText(qstr);
        }
        i+=1;
    }
    clear_queue(screenimagesQ);
    ui->getscreenimagesButton->setText("Get screen images");
}

void MainWindow::DrawMyTrack2(cv::Mat frame,
               int resizeCoeff, const trackinginfo track)
{
    auto ResizeRect = [&](const cv::Rect& r) -> cv::Rect
    {
        return cv::Rect(resizeCoeff * r.x, resizeCoeff * r.y, resizeCoeff * r.width, resizeCoeff * r.height);
    };
    cv::Rect rect = ResizeRect(track.rect);
    int cls_num;

    if(track.class_id == -1){
        cls_num = 7;
    }else{
        cls_num = track.class_id;
    }

    std::string name = "";
    if(track.class_id != -1){
       name = classListTensorRT[track.class_id]+"-"+std::to_string(track.track_id); //text
    }else{
       name = "unknown-"+std::to_string(track.track_id);
    }
    cv::rectangle(frame, rect, m_colors[cls_num], 1, cv::LINE_AA);
    cv::putText(frame, //target image
                name, //text
                cv::Point(rect.x, rect.y-3), //bottom-left position
                cv::FONT_HERSHEY_DUPLEX,
                0.3,
                m_colors[cls_num], //font color
            1);


    // trajectory
    for (int j = 0; j < int(track.centerPositions[0].size())-1; j++)
    {
        cv::Point pt1 = cv::Point(int(track.centerPositions[0][j]*resizeCoeff), int(track.centerPositions[1][j]*resizeCoeff));
        cv::Point pt2 = cv::Point(int(track.centerPositions[0][j+1]*resizeCoeff), int(track.centerPositions[1][j+1]*resizeCoeff));
        cv::line(frame, pt1, pt2, cv::Scalar(255,0,0), 1, cv::LINE_AA); // pyramid down
    }

    // Next position
    //cv::Point pt1 = cv::Point(int(track.next_center[0]*resizeCoeff), int((track.next_center[1]+5)*resizeCoeff));
    //cv::Point pt2 = cv::Point(int(track.next_center[0]*resizeCoeff), int((track.next_center[1]-5)*resizeCoeff));
    //cv::line(frame, pt1, pt2, cv::Scalar(0,0,0), 1, cv::LINE_AA); // pyramid down

    // Next sorting position
    //cv::Point pt1 = cv::Point(int(track.sorting_nxt_pos*resizeCoeff), int((track.centerPositions[1][0]+5)*resizeCoeff));
    //cv::Point pt2 = cv::Point(int(track.sorting_nxt_pos*resizeCoeff), int((track.centerPositions[1][0]-5)*resizeCoeff));
    //cv::line(frame, pt1, pt2, cv::Scalar(0,0,0), 1, cv::LINE_AA); // pyramid down


}

void MainWindow::clear_queue( std::queue<cv::Mat> &q )
{
   std::queue<cv::Mat> empty;
   std::swap( q, empty );
}

void MainWindow::clear_queue( std::queue<QPixmap> &q )
{
   std::queue<QPixmap> empty;
   std::swap( q, empty );
}

void MainWindow::clear_queue( std::queue<ImgInfo> &q )
{
   std::queue<ImgInfo> empty;
   std::swap( q, empty );
}

void MainWindow::clear_queue( std::queue<ImgInfo2> &q )
{
   std::queue<ImgInfo2> empty;
   std::swap( q, empty );
}


void MainWindow::on_getcroppedimagesDelButton_clicked()
{
    if(ui->getcroppedimagesButton->isChecked()){
       ui->getcroppedimagesButton->setChecked(false);
       clear_queue(croppedimagesQ);
       ui->getcroppedimagesButton->setText("Get cropped images");
    }else{
        cancel_getcroppedimages = true;
    }
}

void MainWindow::on_getcameraimagesDelButton_clicked()
{
    if(ui->getcameraimagesButton->isChecked()){
       ui->getcameraimagesButton->setChecked(false);
       clear_queue(cameraimagesQ);
       ui->getcameraimagesButton->setText("Get camera images");
    }else{
        cancel_getcameraimages = true;
    }
}

void MainWindow::on_getcameraimagesDelButton2_clicked()
{
    if(ui->getcameraimagesButton2->isChecked()){
       ui->getcameraimagesButton2->setChecked(false);
       clear_queue(cameraimagesQ2);
       ui->getcameraimagesButton2->setText("Get camera images");
    }else{
        cancel_getcameraimages2 = true;
    }
}


void MainWindow::on_getprocessedimagesDelButton_clicked()
{
    if(ui->getprocessedimagesButton->isChecked()){
       ui->getprocessedimagesButton->setChecked(false);
       clear_queue(processedimagesQ);
       ui->getprocessedimagesButton->setText("Get processed images");
    }else{
        cancel_getprocessedimages = true;
    }
}

void MainWindow::on_getscreenimagesDelButton_clicked()
{
    if(ui->getscreenimagesButton->isChecked()){
       ui->getscreenimagesButton->setChecked(false);
       clear_queue(screenimagesQ);
       ui->getscreenimagesButton->setText("Get screen images");
    }else{
        cancel_getscreenimages = true;
    }
}

void MainWindow::on_getcroppedimagesButton_clicked(bool checked){
    if(checked){

    }else{
        cancel_getcroppedimages = false;
        std::string OutputFolder = "cropped_images";
        if (CreateDirectory(OutputFolder.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            int year = 1900 + ltm->tm_year;
            int month = 1 + ltm->tm_mon;
            int day = ltm->tm_mday;
            int hour = ltm->tm_hour;
            int min = ltm->tm_min;
            int sec = ltm->tm_sec;
            int cnt = int(croppedimagesQ.size());
            std::string path = OutputFolder+"/"+std::to_string(year)+"-"+std::to_string(month)+"-"+std::to_string(day)+"_"+std::to_string(hour)+"h"+std::to_string(min)+"m"+std::to_string(sec)+"s_"+std::to_string(cnt)+"imgs";
            if (CreateDirectory(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
            {
                std::thread getcroppedimagesTh(&MainWindow::getcroppedimagesThread, this, path);
                getcroppedimagesTh.detach();
            }
        }
        else
        {
             // Failed to create directory.
            std::cerr << "Failed to create directory." << std::endl;
        }
    }
}

void MainWindow::on_getcameraimagesButton_clicked(bool checked){
    if(checked){

    }else{
        cancel_getcameraimages = false;
        std::string OutputFolder = "camera_images";
        if (CreateDirectory(OutputFolder.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            int year = 1900 + ltm->tm_year;
            int month = 1 + ltm->tm_mon;
            int day = ltm->tm_mday;
            int hour = ltm->tm_hour;
            int min = ltm->tm_min;
            int sec = ltm->tm_sec;
            int cnt = int(cameraimagesQ.size());
            std::string path = OutputFolder+"/"+std::to_string(year)+"-"+std::to_string(month)+"-"+std::to_string(day)+"_"+std::to_string(hour)+"h"+std::to_string(min)+"m"+std::to_string(sec)+"s_"+std::to_string(cnt)+"imgs";
            if (CreateDirectory(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
            {
                std::thread getcameraimagesTh(&MainWindow::getcameraimagesThread, this, path);
                getcameraimagesTh.detach();
            }
        }
        else
        {
             // Failed to create directory.
            std::cerr << "Failed to create directory." << std::endl;
        }
    }
}

void MainWindow::on_getcameraimagesButton2_clicked(bool checked){
    if(checked){
        global_time = std::chrono::system_clock::now();
    }else{
        cancel_getcameraimages2 = false;
        std::string OutputFolder = "photron_camera_images";
        if (CreateDirectory(OutputFolder.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            int year = 1900 + ltm->tm_year;
            int month = 1 + ltm->tm_mon;
            int day = ltm->tm_mday;
            int hour = ltm->tm_hour;
            int min = ltm->tm_min;
            int sec = ltm->tm_sec;
            int cnt = int(cameraimagesQ2.size());
            std::string path = OutputFolder+"/"+std::to_string(year)+"-"+std::to_string(month)+"-"+std::to_string(day)+"_"+std::to_string(hour)+"h"+std::to_string(min)+"m"+std::to_string(sec)+"s_"+std::to_string(cnt)+"imgs";
            if (CreateDirectory(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
            {
                std::thread getcameraimagesTh2(&MainWindow::getcameraimagesThread2, this, path);
                getcameraimagesTh2.detach();
            }
        }
        else
        {
             // Failed to create directory.
            std::cerr << "Failed to create directory." << std::endl;
        }
    }
}


void MainWindow::on_getprocessedimagesButton_clicked(bool checked){
    if(checked){
        global_time = std::chrono::system_clock::now();
    }else{
        cancel_getprocessedimages = false;
        std::string OutputFolder = "processed_images";
        if (CreateDirectory(OutputFolder.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            int year = 1900 + ltm->tm_year;
            int month = 1 + ltm->tm_mon;
            int day = ltm->tm_mday;
            int hour = ltm->tm_hour;
            int min = ltm->tm_min;
            int sec = ltm->tm_sec;
            int cnt = int(processedimagesQ.size());
            std::string path = OutputFolder+"/"+std::to_string(year)+"-"+std::to_string(month)+"-"+std::to_string(day)+"_"+std::to_string(hour)+"h"+std::to_string(min)+"m"+std::to_string(sec)+"s_"+std::to_string(cnt)+"imgs";
            if (CreateDirectory(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
            {
                std::thread getprocessedimagesTh(&MainWindow::getprocessedimagesThread, this, path);
                getprocessedimagesTh.detach();
            }
        }
        else
        {
             // Failed to create directory.
            std::cerr << "Failed to create directory." << std::endl;
        }
    }
}

void MainWindow::on_getscreenimagesButton_clicked(bool checked){
    if(checked){

    }else{
        cancel_getscreenimages = false;
        std::string OutputFolder = "screen_images";
        if (CreateDirectory(OutputFolder.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            int year = 1900 + ltm->tm_year;
            int month = 1 + ltm->tm_mon;
            int day = ltm->tm_mday;
            int hour = ltm->tm_hour;
            int min = ltm->tm_min;
            int sec = ltm->tm_sec;
            int cnt = int(screenimagesQ.size());
            std::string path = OutputFolder+"/"+std::to_string(year)+"-"+std::to_string(month)+"-"+std::to_string(day)+"_"+std::to_string(hour)+"h"+std::to_string(min)+"m"+std::to_string(sec)+"s_"+std::to_string(cnt)+"imgs";
            if (CreateDirectory(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
            {
                std::thread getscreenimagesTh(&MainWindow::getscreenimagesThread, this, path);
                getscreenimagesTh.detach();
            }
        }
        else
        {
             // Failed to create directory.
            std::cerr << "Failed to create directory." << std::endl;
        }
    }
}

void MainWindow::on_pulseButton_clicked(bool checked)
{
    if(checked){
        std::thread generatePulseTh(&MainWindow::generatepulseThread, this, std::ref(pulse_on_voltage), std::ref(pulse_repetition_interval));
        generatePulseTh.detach();
        ui->pulseButton->setText("Pulse Off");
    }else{
        ui->pulseButton->setText("Pulse On");
    }
}

void MainWindow::on_drawprocessingresultsButton_clicked(bool checked)
{
    if(checked){
        draw_clicked = true;
    }else{
        draw_clicked = false;
    }
}

void MainWindow::on_setbackgroundButton_clicked(){
    setBackground = true;
}


void MainWindow::on_roiButton_clicked()
{
    setROI = true;
    setCursor(Qt::PointingHandCursor);

}
void MainWindow::on_debugButton_clicked(){
    if(!ui->cameraButton->isChecked() && debug_dir_path!=""){
        // -------------- Order -----------------//
        // Image processing, sorting, classification, Get process image Button --> Debug button
        std::thread debugTh(&MainWindow::debugThread, this, std::ref(debug_clicked), std::ref(debug_index), std::ref(debug_file_size), debug_fps, debug_dir_path, debug_background_img_path);
        debugTh.detach();
    }else{
        std::cerr << "[Error] Empty dir path! Please load a directory." << std::endl;
    }
}

void MainWindow::on_loadButton_clicked()
{
    QString default_path = "C:/Users/rnb/Documents/build-cell_sorter_imagingsource-Desktop_Qt_5_15_2_MSVC2019_64bit-Release";
    QString QStringpath = QFileDialog::getOpenFileName(this,
         tr("Open Image"), default_path, tr("Image Files (*.png *.jpg *.bmp)"));
    std::string file_path = QStringpath.toStdString();
    std::string dir_path = "";
    std::string file_name = "";
    auto pos = file_path.rfind("/");
    if (pos!= std::string::npos) {
        std::size_t botDirPos = file_path.find_last_of("/");
        // get directory
        dir_path = file_path.substr(0, botDirPos);
        // get file
        file_name = file_path.substr(botDirPos+1, file_path.length());

        //std::cout << dir_path << std::endl;
        //std::cout << file_name << std::endl;
    } else {
        //do something;
    }

    debug_background_img_path = file_path;
    debug_dir_path = dir_path;
    updateConfigs();
}


void MainWindow::mousePressEvent(QMouseEvent *event){
    if (event->button()==Qt::LeftButton && setROI){
        double scale_factor;
        if(image_width > image_height){
            scale_factor = image_width*1.0/ui->camera_view->width();
        }else{
            scale_factor = image_height*1.0/ui->camera_view->height();
        }
        double scale_factor_x = scale_factor;
        double scale_factor_y = scale_factor;
        //double scale_factor_x = std::max(image_width,image_height)*1.0/ui->camera_view->width();
        //double scale_factor_y = std::max(image_width,image_height)*1.0/ui->camera_view->height();
        if(click_cnt == 0){
            //roi = {0,0,0,0};
            roi_tmp.x = (event->pos().x() - ui->camera_view->x())*scale_factor_x;
            roi_tmp.y = (event->pos().y() - ui->camera_view->y())*scale_factor_y - (ui->camera_view->height()*scale_factor_y - image_height)/2;
            click_cnt++;
        }else{
            roi_tmp.width = (event->pos().x() - ui->camera_view->x())*scale_factor_x - roi_tmp.x;
            roi_tmp.height = (event->pos().y() - ui->camera_view->y())*scale_factor_y - (ui->camera_view->height()*scale_factor_y - image_height)/2 - roi_tmp.y;
            click_cnt = 0;
            setCursor(Qt::ArrowCursor);
            setROI = false;

            // Error handling
            int pt1_x, pt1_y;
            int pt2_x, pt2_y;
            pt1_x = roi_tmp.x;
            pt1_y = roi_tmp.y;
            pt2_x = roi_tmp.x + roi_tmp.width;
            pt2_y = roi_tmp.y + roi_tmp.height;

            if(pt1_x < 0 || pt1_y < 0 || pt2_x < 0 || pt2_y < 0){
                return;
            }
            if(pt1_x > image_width || pt1_y > image_height || pt2_x > image_width || pt2_y > image_height){
                return;
            }
            if(roi_tmp.width == 0 || roi_tmp.height == 0){
                return;
            }

            if(roi_tmp.width < 0 && roi_tmp.height > 0){
                // case 2
                roi_tmp.x = pt2_x;
                roi_tmp.y = pt1_y;
                roi_tmp.width = roi_tmp.width*-1;
            }else if(roi_tmp.width > 0 && roi_tmp.height < 0){
                // case 3
                roi_tmp.x = pt1_x;
                roi_tmp.y = pt2_y;
                roi_tmp.height = roi_tmp.height*-1;
            }else if(roi_tmp.width < 0 && roi_tmp.height < 0){
                // case 4
                roi_tmp.x = pt2_x;
                roi_tmp.y = pt2_y;
                roi_tmp.width = roi_tmp.width*-1;
                roi_tmp.height = roi_tmp.height*-1;
            }
            roi = roi_tmp;
            std::cout << "ROI: " << roi << std::endl;
        }
    }
}


void MainWindow::on_forwardButton_clicked(){
    smaract->move(0, smaract_stepsize, smaract_velocity, 0);
}

void MainWindow::on_backwardButton_clicked(){
    smaract->move(0, smaract_stepsize, smaract_velocity, 1);
}
