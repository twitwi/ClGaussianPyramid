#include <assert.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include <clgp/utils.h>

#include <cv.h>

/*#include "framework.h"*/
typedef void (*Framework) (const char* command, ...);

/* See globalvars.c */
extern cl_command_queue global_command_queue;

struct ipltocl_module {
    Framework framework;

    cl_command_queue command_queue;
};

struct ipltocl_module *
IplToCl__v__create(Framework f)
{
    struct ipltocl_module *module = NULL;

    module = malloc(sizeof(struct ipltocl_module));
    assert(module != NULL);
    module->framework = f;

    return module;
}

void
IplToCl__v__init(struct ipltocl_module *module)
{
    cl_int clerr = 0;
    cl_device_id device = NULL;
    cl_context context = NULL;

    clgpMaxflopsGPU(&device);
    assert(device != NULL);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &clerr);
    assert(clerr == CL_SUCCESS);

    module->command_queue = clCreateCommandQueue(context, device, 0, &clerr);
    assert(clerr == CL_SUCCESS);
}

void
IplToCl__v__stop(struct ipltocl_module *module)
{
    clReleaseCommandQueue(module->command_queue);
    free(module);
}

void 
IplToCl__v__event__v__input(
        struct ipltocl_module *module, 
        const IplImage *ipl)
{
    void *output[] = { 
        "output", 
        "char*", NULL, 
        NULL };

    cl_int clerr = 0;
    cl_context context = NULL;

    const cl_image_format clgpimageformat = {CL_RGBA, CL_UNORM_INT8};

    cl_mem climage = NULL;

    /* Fixme: With OpenCV not storing any color format, we're begging for 
     *        color conversion bugs... the best we can do is to check
     *        the number of channel/depth and pray it is BGR (the default 
     *        format used by cvLoadImage and as such, the ImageSource module).
     */
    assert(ipl->nChannels == 3);
    assert(ipl->depth == (int)IPL_DEPTH_8U);
    IplImage *tmp;

    tmp = cvCreateImage(cvSize(ipl->width, ipl->height), IPL_DEPTH_8U, 4);
    cvCvtColor(ipl, tmp, CV_BGR2RGBA);

    clerr =
        clGetCommandQueueInfo(
                module->command_queue,
                CL_QUEUE_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    climage =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                &clgpimageformat,
                ipl->width,
                ipl->height,
                tmp->widthStep,
                tmp->imageData,
                &clerr);
    assert(clerr == CL_SUCCESS);

    output[2] = (void *)&climage;
    module->framework("emit", output);

    clReleaseMemObject(climage);
    cvReleaseImage(&tmp);
}

