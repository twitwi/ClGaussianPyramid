#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CL/cl.h>

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

    cl_context context = 0;

    FILE *source_file = NULL;
    char *source = NULL;
    size_t source_size = 0;
    cl_program program = 0;
    char build_log[20000];

    int i = 0;

    /* Initialize device, play dumb for now, take the first GPU */
    /* Enumerate platforms */
    err =
        clGetPlatformIDs(
                1,
                platforms,
                &n_platforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "== Could not check for platforms ==\n");
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
        fprintf(stderr, "== Could not check for devices ==\n");
        exit(1);
    }
    if (n_devs == 0) {
        fprintf(stderr, "== No device available on this host ==\n");
        exit(1);
    }
    /* Choose a device */
    device = devices[mydevice];
    /* Create a context on this device */
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "== Could not create a context for the device ==\n");
        exit(1);
    }

    /* Test each file path provided in input */
    for (i = 1; i < argc; i++) {
        /* Open the source file */
        source_file = fopen(argv[i], "r");
        if (source_file == NULL) {
            fprintf(stderr, 
                    "== Could not read %s: %s ==\n", argv[i], strerror(errno));
            exit(1);
        }

        fseek(source_file, 0, SEEK_END);
        source_size = ftell(source_file);
        rewind(source_file);

        source = (char *)malloc(source_size + sizeof(char));
        fread(source, 1, source_size, source_file);
        source[source_size] = 0;
        fclose(source_file);

        /* Create the program object */
        program = 
            clCreateProgramWithSource(
                    context,
                    1,
                    (char const **)&source,
                    NULL,
                    &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr, 
                    "== Could not create the program: openCL internal error ==\n");
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

        free(source);
        clReleaseProgram(program);
    }

    clReleaseContext(context);

    return 0;
}

