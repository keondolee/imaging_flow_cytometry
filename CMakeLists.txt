cmake_minimum_required(VERSION 3.5)

project(cell_sorter_imagingsource VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(CUDA_PATH C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1)
set(OPENCV_PATH C:/opencv4/build)
set(PHOTRON_PATH C:/fastcam3/SDK)
set(NIDAQ_PATH C:/NIDAQmx)
set(IMAGINGSOURCE_PATH C:/IC\ Imaging\ Control\ 3.5)
set(SMARACT_PATH C:/SmarAct/MCS2/SDK)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(cell_sorter_imagingsource
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(cell_sorter_imagingsource SHARED
            ${PROJECT_SOURCES}
            main.cpp
            mainwindow.cpp
            mainwindow.h
            configmanager.cpp
            configmanager.h
            defines.h
            dirent.h
            mytracker_v2.cpp
            mytracker_v2.h
            myutils.h
            nicontrol.cpp
            nicontrol.h
            NIControlFunc.h
            tensorrt_window.cpp
            tensorrt_window.h
            SerialPort.cpp
            SerialPort.hpp
            smaract.cpp
            smaract.h
            photron_camera.cpp
            photron_camera.h
            mainwindow.ui
            C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1/samples/common/logger.cpp
        )
    else()
        add_executable(cell_sorter_imagingsource
            ${PROJECT_SOURCES}
            main.cpp
            mainwindow.cpp
            mainwindow.h
            configmanager.cpp
            configmanager.h
            defines.h
            dirent.h
            mytracker_v2.cpp
            mytracker_v2.h
            myutils.h
            nicontrol.cpp
            nicontrol.h
            NIControlFunc.h
            tensorrt_window.cpp
            tensorrt_window.h
            SerialPort.cpp
            SerialPort.hpp
            smaract.cpp
            smaract.h
            photron_camera.cpp
            photron_camera.h
            mainwindow.ui
            C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1/samples/common/logger.cpp
        )
    endif()
endif()

target_link_libraries(cell_sorter_imagingsource PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(cell_sorter_imagingsource PUBLIC
    ${CUDA_PATH}/include
    ${CUDA_PATH}/samples/common
    ${CUDA_PATH}/samples/common/windows
    ${OPENCV_PATH}/include
    ${PHOTRON_PATH}/Include
    ${NIDAQ_PATH}/include
    ${IMAGINGSOURCE_PATH}/classlib/include
    ${SMARACT_PATH}/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(cell_sorter_imagingsource PUBLIC
        ${CUDA_PATH}/lib/x64/myelin64_1.lib
        ${CUDA_PATH}/lib/x64/nvinfer.lib
        ${CUDA_PATH}/lib/x64/nvinfer_plugin.lib
        ${CUDA_PATH}/lib/x64/nvonnxparser.lib
        ${CUDA_PATH}/lib/x64/nvparsers.lib
        ${CUDA_PATH}/lib/x64/cudart.lib
        ${CUDA_PATH}/lib/x64/cuda.lib
        ${OPENCV_PATH}/x64/vc15/lib/opencv_world450d.lib
        ${PHOTRON_PATH}/Lib/64bit\(x64\)/PDCLIB.lib
        ${NIDAQ_PATH}/lib/NIDAQmx.lib
        ${IMAGINGSOURCE_PATH}/classlib/x64/debug/tis_udshl12d_x64.lib
        ${SMARACT_PATH}/lib64/SmarActCTL.lib
    )
else()
    target_link_libraries(cell_sorter_imagingsource PUBLIC
        ${CUDA_PATH}/lib/x64/myelin64_1.lib
        ${CUDA_PATH}/lib/x64/nvinfer.lib
        ${CUDA_PATH}/lib/x64/nvinfer_plugin.lib
        ${CUDA_PATH}/lib/x64/nvonnxparser.lib
        ${CUDA_PATH}/lib/x64/nvparsers.lib
        ${CUDA_PATH}/lib/x64/cudart.lib
        ${CUDA_PATH}/lib/x64/cuda.lib
        ${OPENCV_PATH}/x64/vc15/lib/opencv_world450.lib
        ${PHOTRON_PATH}/Lib/64bit\(x64\)/PDCLIB.lib
        ${NIDAQ_PATH}/lib/NIDAQmx.lib
        ${IMAGINGSOURCE_PATH}/classlib/x64/release/tis_udshl12_x64.lib
        ${SMARACT_PATH}/lib64/SmarActCTL.lib
    )
endif()


set_target_properties(cell_sorter_imagingsource PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(cell_sorter_imagingsource)
endif()

