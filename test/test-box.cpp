#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <CL/cl.h> 

#define DATA_TYPE float

// Déclaration des fonctions/macros auxilliaires
double timer();

/* Util functions */
char *
load_source(char const *path) {

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

    source = malloc(size + 1);

    fread(source, 1, size, file);
    source[size] = 0;
    fclose(file);

    return source;
}

void
free_source(char *p) {
    free(p);
}

double
timenow(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.;
}

/* Main procedure */
int main() {
    cl_int err = 0;

    cl_platform_id platforms[17];
    cl_uint n_platforms = 0;
    int myplatform = 0;
    cl_platform_id platform;

    cl_device_id devices[17];
    cl_uint n_devs = 0;
    int mydevice = 0;
    cl_device_id device = 0;

    int workgroups = 256;
    int workitems_per_group = 256;
    size_t global_work_size = 0;
    size_t local_work_size = 0;

    cl_context context = 0;
    cl_command_queue queue = 0;

    char *source = NULL;
    cl_program program = 0;
    cl_kernel reduction_1 = 0;
    cl_kernel reduction_2 = 0;

    DATA_TYPE *h_data = NULL;
    cl_mem d_data = 0;
    DATA_TYPE result = (DATA_TYPE)0;

    int iterations = 100;
    double tstart = 0., tend = 0.;

    /* Create data */
    h_data = 
        (DATA_TYPE *)malloc(workgroups*workitems_per_group*sizeof(DATA_TYPE));
    for (int i = 0; i < workgroups*workitems_per_group; i++) {
        h_data[i] = rand() % 255;
    }

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
    source = load_source("BoxFilter.cl");
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
    reduction_1 = clCreateKernel(program, "reduction_1", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: reduction_1 kernel not found\n");
        exit(1);
    }

    /* Get a handle on the first pass kernel */
    reduction_2 = clCreateKernel(program, "reduction_2", &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Error: reduction_2 kernel not found\n");
        exit(1);
    }

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
    tstart = timenow();
    for(int i = 0; i < iterations; i++) {
        /* Define first pass NDRange dimensions */
        global_work_size = workgroups*workitems_per_group;
        local_work_size = workitems_per_group;
        /* Set parameters for first pass kernel */
        err = clSetKernelArg(reduction_1, 0, sizeof(cl_mem), &d_data);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Error: reduction_1 kernel not found\n");
            exit(1);
        }
        /* Run first pass kernel */
        err = 
            clEnqueueNDRangeKernel(
                    queue, 
                    reduction_1, 
                    1,
                    NULL, 
                    &global_work_size, 
                    &local_work_size,
                    0, 
                    NULL, 
                    NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Error: could not run reduction_1 kernel\n");
            exit(1);
        }

        /* Define second pass NDRange dimensions */
        global_work_size = workgroups;
        local_work_size = workgroups;
        /* Set parameters for second pass kernel */
        err = clSetKernelArg(reduction_2, 0, sizeof(cl_mem), &d_data);
        /* Run second pass kernel */
        err = 
            clEnqueueNDRangeKernel(
                    queue, 
                    reduction_2, 
                    1,
                    NULL, 
                    &global_work_size, 
                    &local_work_size,
                    0, 
                    NULL, 
                    NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Error: could not run reduction_2 kernel\n");
            exit(1);
        }

        /* Only the first time, retrieve results */
        if (i == 0) {
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
    tend = timenow();

    printf("GPU result: %f\n", result);
    printf("Done in %g ms\n", (tend - tstart)*1000);

    /* CPU version now */
    tstart = timenow();
    for (int i = 0; i < 100; i++) {
        result = reduction_cpu(h_data, workgroups*workitems_per_group);
    }
    tend = timenow();
    printf("CPU result: %f\n", result);
    printf("Done in %g ms\n", (tend - tstart)*1000);

    /* Free everything */
    clReleaseMemObject(d_data);

    clReleaseKernel(reduction_2);
    clReleaseKernel(reduction_1);

    clReleaseProgram(program);
    free_source(source);

    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(h_data);

    return 0;
}

