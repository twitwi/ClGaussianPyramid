
#include <framework.h>
#include <opencv/cxcore.h>
#include <CL/cl.h> 
#define DATA_TYPE float

class Convolution
{
private:
    cl_int err;
    
    cl_platform_id platforms[17];
    cl_uint n_platforms;
    int myplatform;
    cl_platform_id platform;
    
    cl_device_id devices[17];
    cl_uint n_devs;
    int mydevice;
    cl_device_id device;
    
    int workgroups;
    int workitems_per_group;
    size_t global_work_size;
    size_t local_work_size;
    
    cl_context context;
    cl_command_queue queue;
    
    char *source;
    cl_program program;
    cl_kernel convolution_1;
    cl_kernel convolution_2;
    
    DATA_TYPE *h_data;
    cl_mem d_data;
    DATA_TYPE result;


public:
    IplImage *currentImage;
    
public:
    Framework _framework;
    Convolution();
    void initModule();
    void stopModule();
    void input(IplImage* in);
    void inputComplexWIP(IplImage* in);
};

CLASS_AS_MODULE(Convolution);
