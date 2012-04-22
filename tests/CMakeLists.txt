SET(cmake_include_current_dir YES)
INCLUDE_DIRECTORIES(${ClGaussianPyramid_SOURCE_DIR}/clgp)
INCLUDE_DIRECTORIES(${ImageMagick_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})

ADD_EXECUTABLE(test-pyramid test-pyramid.c)
TARGET_LINK_LIBRARIES(test-pyramid clgp m ${ImageMagick_LIBRARIES})

ADD_EXECUTABLE(test-pyramidhalfoctave test-pyramidhalfoctave.c)
TARGET_LINK_LIBRARIES(test-pyramidhalfoctave clgp m ${ImageMagick_LIBRARIES})

ADD_EXECUTABLE(test-pyramidsqrt2 test-pyramidsqrt2.c)
TARGET_LINK_LIBRARIES(test-pyramidsqrt2 clgp m ${ImageMagick_LIBRARIES})

