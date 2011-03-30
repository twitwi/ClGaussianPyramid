SET(cmake_include_current_dir YES)
INCLUDE_DIRECTORIES(${ClGaussianPyramid_SOURCE_DIR}/clgp)
INCLUDE_DIRECTORIES(${ImageMagick_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})

ADD_EXECUTABLE(test-pyramidhalfoctave test-pyramidhalfoctave.c)
TARGET_LINK_LIBRARIES(test-pyramidhalfoctave clgp m ${ImageMagick_LIBRARIES})
