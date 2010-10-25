# - Try to find an OpenCL library
# Once done this will define
#
#  OPENCL_FOUND - System has OPENCL
#  OPENCL_INCLUDE_DIR - The OPENCL include directory
#  OPENCL_LIBRARIES - The libraries needed to use OPENCL
#
# You can set the OPENCL_ROOT variable if the package is not installed under
# a standard prefix.

# Copyright (c) 2010 Matthieu Volat. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are
# those of the authors and should not be interpreted as representing
# official policies, either expressed or implied, of Matthieu Volat.

IF(OPENCL_LIBRARIES)
   SET(OpenCL_FIND_QUIETLY TRUE)
ENDIF(OPENCL_LIBRARIES)

FIND_PATH(OPENCL_INCLUDE_DIR CL/cl.h 
    PATHS ${OPENCL_ROOT} /usr/local/cuda/include
    PATH_SUFFIXES nvidia)

FIND_LIBRARY(OPENCL_LIBRARIES NAMES OpenCL PATH_SUFFIXES nvidia nvidia-current)

IF(OPENCL_INCLUDE_DIR AND OPENCL_LIBRARIES)
   SET(OPENCL_FOUND TRUE)
ELSE(OPENCL_INCLUDE_DIR AND OPENCL_LIBRARIES)
   SET(OPENCL_FOUND FALSE)
ENDIF (OPENCL_INCLUDE_DIR AND OPENCL_LIBRARIES)

IF(OPENCL_FOUND)
   IF(NOT OpenCL_FIND_QUIETLY)
      MESSAGE(STATUS "Found OpenCL: ${OPENCL_LIBRARIES}")
   ENDIF(NOT OpenCL_FIND_QUIETLY)
ELSE(OPENCL_FOUND)
   IF(OpenCL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find OpenCL")
   ENDIF(OpenCL_FIND_REQUIRED)
ENDIF(OPENCL_FOUND)

MARK_AS_ADVANCED(OPENCL_INCLUDE_DIR OPENCL_LIBRARIES)

