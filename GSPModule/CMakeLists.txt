project ( gspclgp )
cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${ClGaussianPyramid_SOURCE_DIR}/clgp)
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})

ADD_LIBRARY ( gspclgp SHARED GaussPyramid.cpp PyramidLevelSelector.cpp RGBToIPL.cpp IPLToRGB.cpp)
TARGET_LINK_LIBRARIES(gspclgp clgp ${OPENCL_LIBRARIES})
