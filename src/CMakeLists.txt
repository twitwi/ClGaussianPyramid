SET(cmake_include_current_dir YES)
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIR})

#add_definitions(-DPASSIVE_GSP_FRAMEWORK)

ADD_LIBRARY(clgp SHARED ImageSource.cpp CvViewer.cpp Convolution.cpp)
TARGET_LINK_LIBRARIES(clgp ${OpenCL_LIBRARIES} ${OPENCV_LIBRARIES})

#add_library(Incubator SHARED CustomSource ImageSource CvViewer OpticalFlow RenderVectorField)
#target_link_libraries(Incubator cxcore highgui)

#add_executable(test-box test-box)
#target_link_libraries(optical-flow Incubator)

