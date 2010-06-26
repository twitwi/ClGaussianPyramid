
#define Convolution_ _IMPL
#include "Convolution.h"

#include <opencv/cv.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

static void freeImage(IplImage* &image) {
    if (image) {
        cvReleaseImage(&image);
        image = NULL;
    }
}

static char *load_source(char const *path) {

    size_t size = 0;
    char *source = 0;
    FILE *file = NULL;

    file = fopen(path, "r");
    if (!file) {
        perror(NULL);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    source = (char*) malloc(size + 1);

    fread(source, 1, size, file);
    source[size] = 0;
    fclose(file);

    return source;
}

static void free_source(char *p) {
    free(p);
}

Convolution::Convolution() : currentImage(NULL) {
    err = 0;
    
    n_platforms = 0;
    myplatform = 0;
    
    n_devs = 0;
    mydevice = 0;
    device = 0;
    
    workgroups = 256;
    workitems_per_group = 256;
    global_work_size = 0;
    local_work_size = 0;
    
    context = 0;
    queue = 0;
    
    source = NULL;
    program = 0;
    convolution_1 = 0;
    convolution_2 = 0;
    
    h_data = NULL;
    d_data = 0;
    result = (DATA_TYPE)0;
}

void Convolution::initModule() {
    /* Enumerate platforms */
    err =
        clGetPlatformIDs(
                17,
                platforms,
                &n_platforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not check for platforms\n");
        exit(1);
    }

    /* Choose a platform */
    platform = platforms[myplatform];

    /* Enumerate devices */
    err = 
        clGetDeviceIDs(
                platform, 
                CL_DEVICE_TYPE_GPU, 
                17, 
                devices, 
                &n_devs);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not check for devices\n");
        exit(1);
    }

    /* Choose a device */
    device = devices[mydevice];

    /* Create a context on this device */
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not create context\n");
        exit(1);
    }

    /* Create command queue associated with the context */
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not create command queue\n");
        exit(1);
    }

    /* Load the source */
    source = load_source("cl/Simple.cl");
    program = 
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source, 
                0, 
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not create program\n");
        exit(1);
    }

    /* Compile */
    if(clBuildProgram(program, 0, NULL, "-cl-mad-enable", NULL, NULL) != CL_SUCCESS) {
        char build_log[20000];
        clGetProgramBuildInfo(
                program, 
                device, 
                CL_PROGRAM_BUILD_LOG, 
                sizeof(build_log), 
                build_log, 
                NULL);
        printf("Build errors.\n%s\n", build_log);
        exit(1);
    }

    /* Get a handle on the first pass kernel */
    convolution_1 = clCreateKernel(program, "BoxRowsTex", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: _1 kernel not found\n");
        exit(1);
    }

    /* Get a handle on the first pass kernel */
    convolution_2 = clCreateKernel(program, "BoxColumns", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: _2 kernel not found\n");
        exit(1);
    }
    fprintf(stderr, "INITED\n");
}

static unsigned int iDivUp(unsigned int dividend, unsigned int divisor)
{
    return (dividend % divisor == 0) ? (dividend / divisor) : (dividend / divisor + 1);
}

void Convolution::input(IplImage* in) {
    int w = in->width;
    int h = in->height;
    //int imageSize = in->widthStep * h * sizeof(unsigned int);
    int imageSize = w * h * sizeof(unsigned int)*2;
    // Allocate OpenCL 2D Image (Texture) object for the source data
    cl_image_format InputFormat;
    InputFormat.image_channel_order = CL_RGBA;
    InputFormat.image_channel_data_type = CL_UNSIGNED_INT8;
    //cl_mem cmDevTexIn = clCreateImage2D(context, CL_MEM_READ_ONLY, &InputFormat, w, h, in->widthStep * sizeof(unsigned int), in->imageData, &err);       
    cl_mem cmDevTexIn = clCreateImage2D(context, CL_MEM_READ_ONLY, &InputFormat, w, h,w * sizeof(unsigned int), in->imageData, &err);

    // Allocate the OpenCL intermediate and result buffer memory objects on the device GMEM
    cl_mem cmDevBufTemp = clCreateBuffer(context, CL_MEM_READ_WRITE, imageSize, NULL, &err);
    cl_mem cmDevBufOut = clCreateBuffer(context, CL_MEM_WRITE_ONLY, imageSize, NULL, &err);
    //TODO check err

    int r = 2;
    float fScale = 5.f;
    size_t szGlobalWorkSize[2];         // global # of work items
    size_t szLocalWorkSize[2];          // work group # of work items 

    // Set the Argument values for the row kernel
    err = clSetKernelArg(convolution_1, 0, sizeof(cl_mem), (void*)&cmDevTexIn);
    err |= clSetKernelArg(convolution_1, 1, sizeof(cl_mem), (void*)&cmDevBufTemp);
    err |= clSetKernelArg(convolution_1, 2, sizeof(unsigned int), (void*)&w);
    err |= clSetKernelArg(convolution_1, 3, sizeof(unsigned int), (void*)&h);
    err |= clSetKernelArg(convolution_1, 4, sizeof(int), (void*)&r);
    err |= clSetKernelArg(convolution_1, 5, sizeof(float), (void*)&fScale);

    // Set the Argument values for the column kernel
    err  = clSetKernelArg(convolution_2, 0, sizeof(cl_mem), (void*)&cmDevBufTemp);
    err |= clSetKernelArg(convolution_2, 1, sizeof(cl_mem), (void*)&cmDevBufOut);
    err |= clSetKernelArg(convolution_2, 2, sizeof(unsigned int), (void*)&w);
    err |= clSetKernelArg(convolution_2, 3, sizeof(unsigned int), (void*)&h);
    err |= clSetKernelArg(convolution_2, 4, sizeof(int), (void*)&r);
    err |= clSetKernelArg(convolution_2, 5, sizeof(float), (void*)&fScale);

    { // BoxFilterGPU(uiInput, uiOutput, w, h, r, fScale);
    // var for kernel timing
    double dKernelTime = 0.0;

    int NTHREADS = 64;

    // Copy input data from host to device Image (Texture)
    const size_t szTexOrigin[3] = {0, 0, 0};                // Offset of input texture origin relative to host image
    const size_t szTexRegion[3] = {w, h, 1};   // Size of texture region to operate on
    err = clEnqueueWriteImage(queue, cmDevTexIn, CL_TRUE, szTexOrigin, szTexRegion, 0, 0, in->imageData, 0, NULL, NULL);
#define ERR()     if (err != CL_SUCCESS) {        fprintf(stderr, "Error at line %d\n", __LINE__);        exit(1);    }
    ERR();
    //    oclCheckErrorEX(err, CL_SUCCESS, pCleanup);
    /*

    // Set global and local work sizes for row kernel
    szLocalWorkSize[0] = NTHREADS;
    szLocalWorkSize[1] = 1;
    szGlobalWorkSize[0] = szLocalWorkSize[0] * iDivUp(h, (unsigned int)szLocalWorkSize[0]);
    szGlobalWorkSize[1] = 1;

    // sync host and start timer
    clFinish(queue);

    // Launch row kernel
    err = clEnqueueNDRangeKernel(queue, convolution_1, 2, NULL, szGlobalWorkSize, szLocalWorkSize, 0, NULL, NULL);
    //    oclCheckErrorEX(err, CL_SUCCESS, pCleanup);

    // Set global and local work sizes for column kernel
    szLocalWorkSize[0] = NTHREADS;
    szLocalWorkSize[1] = 1;
    szGlobalWorkSize[0] = szLocalWorkSize[0] * iDivUp(w, (unsigned int)szLocalWorkSize[0]);
    szGlobalWorkSize[1] = 1;

    // Launch column kernel
    err = clEnqueueNDRangeKernel(queue, convolution_2, 2, NULL, szGlobalWorkSize, szLocalWorkSize, 0, NULL, NULL);
    //    oclCheckErrorEX(err, CL_SUCCESS, pCleanup);

    // sync host and stop timer
    clFinish(queue);


    */
    IplImage *out = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);

    // Copy results back to host, block until complete
    //    err = clEnqueueReadBuffer(queue, cmDevBufOut, CL_TRUE, 0, imageSize, out->imageData, 0, NULL, NULL);
    //    oclCheckErrorEX(err, CL_SUCCESS, pCleanup);

    emitNamedEvent("output", out);
    cvReleaseImage(&out);
 }
}

void Convolution::inputComplexWIP(IplImage* in) {
        /* Allocate memory on device and copy data */
    d_data = 
        clCreateBuffer(
                context, 
                CL_MEM_READ_WRITE,
                workgroups*workitems_per_group*sizeof(DATA_TYPE), 
                NULL, 
                &err);
    if (d_data == 0) {
        fprintf(stderr, "Error: could not alloc memory on device\n");
        exit(1);
    }
    err = 
        clEnqueueWriteBuffer(
                queue,
                d_data,
                CL_TRUE,
                0,
                workgroups*workitems_per_group*sizeof(DATA_TYPE),
                h_data,
                0,
                NULL,
                NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not write data\n");
        exit(1);
    }

    clFinish(queue);
    //for(int i = 0; i < iterations; i++) {
    {
        /* Define first pass NDRange dimensions */
        global_work_size = workgroups*workitems_per_group;
        local_work_size = workitems_per_group;
        /* Set parameters for first pass kernel */
        err = clSetKernelArg(convolution_1, 0, sizeof(cl_mem), &d_data);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Error: convolution_1 kernel not found\n");
            exit(1);
        }
        /* Run first pass kernel */
        err = 
            clEnqueueNDRangeKernel(
                    queue, 
                    convolution_1, 
                    1,
                    NULL, 
                    &global_work_size, 
                    &local_work_size,
                    0, 
                    NULL, 
                    NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Error: could not run convolution_1 kernel\n");
            exit(1);
        }

        /* Define second pass NDRange dimensions */
        global_work_size = workgroups;
        local_work_size = workgroups;
        /* Set parameters for second pass kernel */
        err = clSetKernelArg(convolution_2, 0, sizeof(cl_mem), &d_data);
        /* Run second pass kernel */
        err = 
            clEnqueueNDRangeKernel(
                    queue, 
                    convolution_2, 
                    1,
                    NULL, 
                    &global_work_size, 
                    &local_work_size,
                    0, 
                    NULL, 
                    NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Error: could not run convolution_2 kernel\n");
            exit(1);
        }

        /* Only the first time, retrieve results */
        //        if (i == 0) {
        {
            err = 
                clEnqueueReadBuffer(
                        queue, 
                        d_data, 
                        CL_TRUE, 
                        0,
                        sizeof(DATA_TYPE), 
                        &result, 
                        0, 
                        NULL, 
                        NULL);
            if (err != CL_SUCCESS) {
                fprintf(stderr, "Error: could not get results\n");
                exit(1);
            }
        }
    }

}

void Convolution::stopModule() {
    /* Free everything */
    clReleaseMemObject(d_data);

    clReleaseKernel(convolution_2);
    clReleaseKernel(convolution_1);

    clReleaseProgram(program);
    free_source(source);

    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(h_data);

    freeImage(currentImage);
}


