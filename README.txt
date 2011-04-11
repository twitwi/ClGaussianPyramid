ClGaussianPyramid
================================================================================
Experiments on Gaussian Pyramid implemented using OpenCL

Authors: Rem, Mazhe


--------------------------------------------------------------------------------
clgp:
$ mkdir build 
$ cd build 
$ cmake .. 
$ make 
$ ./test/test-pyramid ../media/lena512color.png

you might need to tell to cmake where to find opencv includes, e.g.:
$ cmake -D OPENCL_ROOT=/.../NVIDIA_GPU_Computing_SDK/OpenCL/common/inc ..

JavaImplementation:
$ cd JavaImplementation 
$ ant run

