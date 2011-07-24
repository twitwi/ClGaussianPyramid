PROJECT(ClGaussianPyramid C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo 
      CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
IF(NOT CMAKE_C_FLAGS)
  SET(CMAKE_C_FLAGS "-Wall -Wextra -Wshadow"
      CACHE STRING
      "Flags used by the compiler during all build types."
      FORCE)
ENDIF(NOT CMAKE_C_FLAGS)
SET(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG"
    CACHE STRING 
    "Flags used by the compiler during debug builds." 
    FORCE)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
FIND_PACKAGE(ImageMagick COMPONENTS MagickWand REQUIRED)
FIND_PACKAGE(OpenCL REQUIRED)
FIND_PACKAGE(OpenCV QUIET)

ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(clgp)
ADD_SUBDIRECTORY(tests)
IF(OpenCV_FOUND)
  ADD_SUBDIRECTORY(GSPModule)
ENDIF(OpenCV_FOUND)

