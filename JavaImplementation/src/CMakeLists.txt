FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.class")
SET(CMAKE_Java_COMPILE_OBJECT
  "<CMAKE_Java_COMPILER> <FLAGS> <SOURCE> -d \"<CMAKE_CURRENT_BINARY_DIR>/.class\""
)
SET(CMAKE_Java_CREATE_STATIC_LIBRARY
  "<CMAKE_Java_ARCHIVE> -cf <TARGET> -C \"<CMAKE_CURRENT_BINARY_DIR>/.class\" ."
)

# Where our instruction really begins
SET(CMAKE_INCLUDE_CURRENT_DIR YES)

INCLUDE_DIRECTORIES(${JavaImplementation_SOURCE_DIR}/lib/JOCL-0.1.3-beta-linux-64.jar)
LINK_DIRECTORIES(${JavaImplementation_SOURCE_DIR}/lib)

SET(JavaImplementation_SRCS 
    com/heeere/opencl/demo/FileUtils.java
    com/heeere/opencl/demo/ImageStackViewer.java
    com/heeere/opencl/demo/JOCLSimpleConvolution.java
    com/heeere/opencl/JOCLConvolveOp.java
    com/heeere/opencl/PyramidImpl.java
    com/heeere/opencl/Pyramid.java
    com/heeere/opencl/PyramidProcessor.java
    com/heeere/opencl/TestPyramidOnLena.java)
ADD_LIBRARY(JavaImplementation ${JavaImplementation_SRCS})
TARGET_LINK_LIBRARIES(JavaImplementation JOCL-0.1.3-beta-linux-64)

