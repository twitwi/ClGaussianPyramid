#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

/* We can't use the standard getopt */
#define OPT_D 1
#define OPT_I 2
#define OPT_CL_SINGLE_PRECISION_CONSTANT 3
#define OPT_CL_DENORMS_ARE_ZERO 4
#define OPT_CL_OPT_DISABLE 5
#define OPT_CL_STRICT_ALIASING 6
#define OPT_CL_MAD_ENABLE 7
#define OPT_CL_NO_SIGNED_ZEROS 8
#define OPT_CL_UNSAFE_MATH_OPTIMIZATIONS 9
#define OPT_CL_FINITE_MATH_ONLY 10
#define OPT_CL_FAST_RELAXED_MATH 11
#define OPT_W 12
#define OPT_WERROR 13
#define OPT_UNKNOWN '?'

const char *clc_opts[] = {
    "",
    "-D",
    "-I",
    "-cl-single-precision-constant",
    "-cl-denorms-are-zero",
    "-cl-opt-disable",
    "-cl-strict-aliasing",
    "-cl-mad-enable",
    "-cl-no-signed-zeros",
    "-cl-unsafe-math-optimizations",
    "-cl-finite-math-only",
    "-cl-fast-relaxed-math",
    "-w",
    "-Werror"
};

char *clc_optarg;
int clc_optind = 1, clc_opterr, clc_optopt;

int clc_getopt(int argc, char * const argv[])
{
    int opt = OPT_UNKNOWN;
    int i = 0;

    if (clc_optind >= argc) {
        return -1; /* End of arguments */
    }

    if (argv[clc_optind][0] != '-') {
        return -1; /* End of options */
    }

    /* Search for the opt */
    for (i = 0; i < sizeof(clc_opts)/sizeof(char *); i++) {
        if (strncmp(argv[clc_optind], clc_opts[i], strlen(clc_opts[i])+1) == 0) {
            opt = i;
            break;
        }
    }

    if (opt != OPT_UNKNOWN) {
        /* Try to get optarg */
        switch (opt) {
            case OPT_D:
                clc_optarg = argv[++clc_optind];
                break;
            case OPT_I:
                clc_optarg = argv[++clc_optind];
                break;
            case OPT_CL_SINGLE_PRECISION_CONSTANT:
            case OPT_CL_DENORMS_ARE_ZERO:
            case OPT_CL_OPT_DISABLE:
            case OPT_CL_STRICT_ALIASING:
            case OPT_CL_MAD_ENABLE:
            case OPT_CL_NO_SIGNED_ZEROS:
            case OPT_CL_UNSAFE_MATH_OPTIMIZATIONS:
            case OPT_CL_FINITE_MATH_ONLY:
            case OPT_CL_FAST_RELAXED_MATH:
            case OPT_W:
            case OPT_WERROR:
            default:
                clc_optarg = NULL;
                break;
        }

        /* Go to next opt */
        clc_optind++;
    }

    return opt;
}

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

    char *clflags = NULL;
    FILE *source_file = NULL;
    char *source = NULL;
    size_t source_size = 0;
    cl_program program = 0;
    char build_log[20000];

    int c = 0;
    int i = 0;

    /* Get the clflags */
    /* Initialize with CLFLAGS env variable */
    //clflags = getenv("CLFLAGS");
    /* If there are options, take the options instead */
    if (clflags == NULL) {
        clflags = (char *)malloc(1*sizeof(char));
        clflags[0] = '\0';
    }
    while ((c = clc_getopt(argc, argv)) != -1) {
        switch (c) {
            case OPT_D:
                if (clc_optarg == NULL) {
                    fprintf(stderr, "== Option D need name ==\n");
                    exit(1);
                }
                clflags = 
                    realloc(clflags, strlen(clflags)+strlen(clc_opts[c])+strlen(clc_optarg)+3);
                strcat(clflags, clc_opts[c]);
                strcat(clflags, " ");
                strcat(clflags, clc_optarg);
                strcat(clflags, " ");
                break;
            case OPT_I:
                if (clc_optarg == NULL) {
                    fprintf(stderr, "== Option I need dir ==\n");
                    exit(1);
                }
                clflags = 
                    realloc(clflags, strlen(clflags)+strlen(clc_opts[c])+strlen(clc_optarg)+3);
                strcat(clflags, clc_opts[c]);
                strcat(clflags, " ");
                strcat(clflags, clc_optarg);
                strcat(clflags, " ");
                break;
            case '?':
                fprintf(stderr, "== Unrecognized option %s ==\n", argv[clc_optind]);
                exit(1);
                break;
            default:
                clflags = 
                    realloc(clflags, strlen(clflags)+strlen(clc_opts[c])+2);
                strcat(clflags, clc_opts[c]);
                strcat(clflags, " ");
                break;
        }
    }

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
                CL_DEVICE_TYPE_ALL,
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
            clBuildProgram(program, 0, NULL, clflags, NULL, NULL);
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

    free(clflags);

    return 0;
}

