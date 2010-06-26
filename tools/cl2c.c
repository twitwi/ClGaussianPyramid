#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char *optarg;
extern int optind;
extern int optopt;

const char *outfile_start = 
    "#ifdef __cplusplus\n" \
    "extern \"C\" {\n" \
    "#endif\n\n";

const char *outfile_end = 
    "#ifdef __cplusplus\n" \
    "}\n"
    "#endif\n\n";

void
usage()
{
    fprintf(stderr, 
            "usage: cl2c [-n <array name>] [-o <output file>] <input file>\n");
}

int
main(int argc, char *argv[])
{
    char *infile_path = NULL;
    char *outfile_path = "out.c";
    char *var_name = "kernel_src";

    FILE *infile = NULL;
    FILE *outfile = NULL;

    int c = 0;
    int cptr = 1;

    while ((c = getopt(argc, argv, ":n:ho:")) != -1) {
        switch (c) {
            case 'h':
                usage();
                exit(0);
            case 'n':
                outfile_path = optarg;
                break;
            case 'o':
                outfile_path = optarg;
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: -%c\n", optopt);
                usage();
                exit(1);
        }
    }

    infile_path = argv[optind];
    if (infile_path == NULL) {
        fprintf(stderr, "No input file\n");
        usage();
        exit(1);
    }

    infile = fopen(infile_path, "r");
    if (infile == NULL) {
        fprintf(stderr, 
                "Could not open input file %s: %s\n", 
                infile_path, strerror(errno));
        exit(1);
    }

    outfile = fopen(outfile_path, "w");
    if (outfile == NULL) {
        fprintf(stderr, 
                "Could not open output file %s: %s\n", 
                outfile_path, strerror(errno));
        exit(1);
    }

    fprintf(outfile, "%s", outfile_start);
    fprintf(outfile, "const char %s[] = {\n\t", var_name);
    while ((c = fgetc(infile)) != EOF) {
        fprintf(outfile, "0x%02x, ", c);
        if (cptr++ % 10 == 0) {
            fprintf(outfile, "\n\t");
        }
    }
    fprintf(outfile, "\n};\n\n");
    fprintf(outfile, "%s", outfile_end);

    fclose(infile);
    fclose(outfile);

    return 0;
}

