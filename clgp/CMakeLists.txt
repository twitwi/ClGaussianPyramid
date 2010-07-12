SET(cmake_include_current_dir YES)
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR})

SET(CL_SRCS downscaledconvolution.cl)
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
  SET(CL_C_SRCS ${CL_C_SRCS} "${CL_SRC}.c")
ENDFOREACH(CL_SRC ${CL_SRCS})

SET(clgp_SRCS 
    clgp.c
    clgp.h
    downscaledconvolution.c
    downscaledconvolution.h
    error.c
    error.h
    ${CL_C_SRCS})
ADD_LIBRARY(clgp SHARED ${clgp_SRCS})
TARGET_LINK_LIBRARIES(clgp ${OPENCL_LIBRARIES})

