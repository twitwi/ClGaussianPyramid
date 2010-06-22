
project(ClGaussianPyramid)

cmake_minimum_required(VERSION 2.6)

#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

#find_package(OpenCV REQUIRED)

include_directories(.)
include_directories(/home/twilight/app/NVIDIA_GPU_Computing_SDK/OpenCL/common/inc)

#add_definitions(-DPASSIVE_GSP_FRAMEWORK)

add_library(clgp SHARED ImageSource CvViewer Convolution)
target_link_libraries(clgp cxcore highgui OpenCL)

#add_library(Incubator SHARED CustomSource ImageSource CvViewer OpticalFlow RenderVectorField)
#target_link_libraries(Incubator cxcore highgui)

#add_executable(test-box test-box)
#target_link_libraries(optical-flow Incubator)

