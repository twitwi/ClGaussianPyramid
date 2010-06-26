#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

#include <cv.h>
#include <highgui.h>

#include <downscale.h>

extern const char kernel_src[];

cl_context clgp_context;
cl_command_queue clgp_queue;

cl_kernel clgp_downscale_kernel;

int 
main(int argc, char *argv[])
{
    cl_int err = 0;

    cl_platform_id platforms[1];
    cl_uint n_platforms = 0;
    int myplatform = 0;
    cl_platform_id platform;
    cl_device_id devices[1];
    cl_uint n_devs = 0;
    int mydevice = 0;
    cl_device_id device = 0;
    char *source = (char *)kernel_src;
    size_t source_size = 0;
    cl_program program = 0;
    char build_log[20000];

    IplImage *input = NULL, *output = NULL, *tmp = NULL;

    cl_image_format imageformat = {CL_RGBA, CL_UNSIGNED_INT8};
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1}; /* To update when we got image size */
    cl_mem climage_input = 0, climage_output = 0;

    /* OpenCL init, many should go in a clgp_init() function */
    /* Enumerate platforms */
    err =
        clGetPlatformIDs(
                1,
                platforms,
                &n_platforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not check for platforms\n");
        exit(1);
    }

    /* Choose a platform */
    platform = platforms[myplatform];

    /* Enumerate devices */
    err =
        clGetDeviceIDs(
                platform,
                CL_DEVICE_TYPE_GPU,
                1,
                devices,
                &n_devs);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not check for devices\n");
        exit(1);
    }
    if (n_devs == 0) {
        fprintf(stderr, "No device available on this host\n");
        exit(1);
    }

    /* Choose a device */
    device = devices[mydevice];

    /* Create a clgp_context on this device */
    clgp_context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not create a clgp_context for the device\n");
        exit(1);
    }

    /* Create command clgp_queue associated with the clgp_context */
    clgp_queue = clCreateCommandQueue(clgp_context, device, 0, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: could not create command clgp_queue\n");
        exit(1);
    }

    /* Create the program object */
    program =
        clCreateProgramWithSource(
                clgp_context,
                1,
                (char const **)&source,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr,
                "Could not create the program: openCL internal error\n");
        exit(1);
    }

    /* Try to build the program */
    err =
        clBuildProgram(program, 0, NULL, "", NULL, NULL);
    if (err != CL_SUCCESS) {
        clGetProgramBuildInfo(
                program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, "== Build errors ==\n%s\n", build_log);
        exit(1);
    }

    /* Get the downscale kernel */
    clgp_downscale_kernel = clCreateKernel(program, "downscale", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: downscale kernel not found\n");
        exit(1);
    }


    /* Load image */
    input = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
    if (input == NULL) {
        fprintf(stderr, "Could not load file %s\n", argv[1]);
        exit(1);
    }
    cvNamedWindow("input image", CV_WINDOW_AUTOSIZE);
    cvShowImage("input image", input);

    /* Convert to an acceptable OpenCL format (we don't do {RGB, UINT8} */
    tmp = 
        cvCreateImage(
                cvSize(input->width, input->height), 
                input->depth, 
                4);
    cvCvtColor(input, tmp, CV_BGR2RGBA);
    cvReleaseImage(&input);
    input = tmp;
    tmp = NULL;

    climage_input =
        clCreateImage2D(
                clgp_context,
                CL_MEM_READ_ONLY,
                &imageformat,
                input->width,
                input->height,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_input\n");
        exit(1);
    }

    climage_output =
        clCreateImage2D(
                clgp_context,
                CL_MEM_WRITE_ONLY,
                &imageformat,
                input->width/2,
                input->height/2,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_output\n");
        exit(1);
    }

    region[0] = input->width;
    region[1] = input->height;
    err =
        clEnqueueWriteImage(
                clgp_queue,
                climage_input,
                CL_TRUE,
                origin,
                region,
                input->widthStep,
                0,
                input->imageData,
                0,
                NULL,
                NULL);
    clFinish(clgp_queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy input data on device (%i)\n", err);
        exit(1);
    }


    /* At last, call our downscale function */
    clgp_downscale(
            climage_output, 
            climage_input,
            input->width,
            input->height);


    /* Retrieve image */
    output = 
        cvCreateImage(
                cvSize(input->width/2, input->height/2), 
                input->depth, 
                4);
    region[0] = output->width;
    region[1] = output->height;
    err = 
        clEnqueueReadImage(
                clgp_queue,
                climage_output,
                CL_TRUE,
                origin,
                region,
                output->widthStep,
                0,
                output->imageData,
                0,
                NULL,
                NULL);
    clFinish(clgp_queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy output data on host (%i)\n", err);
        exit(1);
    }


    /* Release device ressources */
    clReleaseMemObject(climage_input);
    clReleaseMemObject(climage_output);

    clReleaseKernel(clgp_downscale_kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(clgp_queue);
    clReleaseContext(clgp_context);


    /* Show results */
    cvNamedWindow("output image", CV_WINDOW_AUTOSIZE);
    cvShowImage("output image", output);
    cvWaitKey(0);
    cvDestroyWindow("input image");
    cvDestroyWindow("output image");

    cvReleaseImage(&input);
    cvReleaseImage(&output);

    return 0;
}

