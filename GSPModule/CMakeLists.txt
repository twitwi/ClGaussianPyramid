PROJECT(gspclgp C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(
    ${ClGaussianPyramid_SOURCE_DIR}
    ${OPENCL_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS})

LINK_DIRECTORIES(
    ${OpenCV_LIB_DIR})

SET(gspclgp_SRCS
    bgrtoipl.c
    gausspyramid.c
    globalvars.c
    ipltocl.c
    ipltobgr.c
    levelselector.c)
SET(gspclgp_HEADERS
    framework.h)
ADD_LIBRARY(gspclgp SHARED ${gspclgp_SRCS} ${gspclgp_HEADERS})
TARGET_LINK_LIBRARIES(gspclgp clgp ${OPENCL_LIBRARIES} ${OpenCV_LIBS})

