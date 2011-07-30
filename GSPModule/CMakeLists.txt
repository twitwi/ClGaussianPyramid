PROJECT(gspclgp C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(${ClGaussianPyramid_SOURCE_DIR})
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})

SET(gspclgp_SRCS
    bgrtoipl.c
    gausspyramid.c
    ipltobgr.c
    levelselector.c)
SET(gspclgp_HEADERS
    framework.h
    layoututils.h)
ADD_LIBRARY(gspclgp SHARED ${gspclgp_SRCS} ${gspclgp_HEADERS})
TARGET_LINK_LIBRARIES(gspclgp clgp ${OPENCL_LIBRARIES})
