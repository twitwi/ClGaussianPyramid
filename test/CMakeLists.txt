SET(cmake_include_current_dir YES)
INCLUDE_DIRECTORIES(${ClGaussianPyramid_SOURCE_DIR}/clgp)
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

ADD_EXECUTABLE(test-downscale test-downscale.c)
TARGET_LINK_LIBRARIES(test-downscale clgp ${OpenCV_LIBRARIES})

#add_executable(test-box test-box)
#target_link_libraries(optical-flow Incubator)

