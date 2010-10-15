SET(cmake_include_current_dir YES)
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})

SET(CL_SRCS convolution.cl downscaledconvolution.cl)
FOREACH(CL_SRC ${CL_SRCS})
  # Check CL_SRC syntax a first time at compilation time, then build a C source
  # file that will contains a string with the program source from CL_SRC
  STRING(REGEX REPLACE ".cl" "" PROGRAM_NAME ${CL_SRC})
  ADD_CUSTOM_COMMAND(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CL_SRC}.c
      COMMAND clc ${CMAKE_CURRENT_SOURCE_DIR}/${CL_SRC} 
      COMMAND cl2c -n clgp_${PROGRAM_NAME}_kernel_src -o ${CMAKE_CURRENT_BINARY_DIR}/${CL_SRC}.c ${CMAKE_CURRENT_SOURCE_DIR}/${CL_SRC}
      DEPENDS clc cl2c ${CMAKE_CURRENT_SOURCE_DIR}/${CL_SRC}
      COMMENT "Generating ressource C file for OpenCL program ${PROGRAM_NAME}")
  SET(CL_C_SRCS "${CL_C_SRCS};${CL_SRC}.c")
ENDFOREACH(CL_SRC ${CL_SRCS})
ADD_CUSTOM_TARGET(generate_CL_C_SRCS DEPENDS ${CL_C_SRCS})

SET(clgp_SRCS 
    clgp.c
    clgp.h
    convolution.c
    convolution.h
    downscaledconvolution.c
    downscaledconvolution.h
    error.c
    error.h
    utils.c
    utils.h
    ${CL_C_SRCS})
ADD_LIBRARY(clgp SHARED ${clgp_SRCS})
ADD_DEPENDENCIES(clgp generate_CL_C_SRCS)
TARGET_LINK_LIBRARIES(clgp ${OPENCL_LIBRARIES})

