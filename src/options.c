// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include "options.h"
#include "file.h"
#include "errors.h"
// options.c

#ifndef GIT_VERSION
#  define GIT_VERSION "?"
#endif

#ifndef BUILD_DATE
#  define BUILD_DATE "?"
#endif

void print_version() {
  printf("agent 001 build " GIT_VERSION " " BUILD_DATE "\n");
  printf("  https://agent.engineering/\n");
  printf("  Copyright 2021, Winfield Chen and Lloyd T. Elliott.\n");
  printf("\n");
}

void read_options(
		int argc,
		char** argv,
                int *action,
		char** inputBgenFile,
                char** inputGenFile,
                char** inputA1File,
		char** outputA1File,
                char** inputPhenotypeFile,
		char** outputGwasDirectory,
		char** outputDosageFile,
		uint16_t* computeThreads
          ) {

        if (argc <= 1) {
          print_version();
          exit(0);
        }

	enum {
		VERSION = 1,
		GENOTYPES = 2,
		CONVERT = 3,
		THREADS = 4,
		PHENOTYPES = 5,
		DOSAGE = 6,
	};

        int vflag = 0;
        int fflag = 0;
        int gflag = 0;
        char *garg = NULL;
        int cflag = 0;
        int tflag = 0;
        int pflag = 0;
        int dflag = 0;

	struct option const options[] = {
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = VERSION
		},
		{
			.name = "genotypes",
			.has_arg = required_argument,
			.flag = NULL,
			.val = GENOTYPES
		},
		{
			.name = "convert",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CONVERT
		},
		{
			.name = "threads",
			.has_arg = required_argument,
			.flag = NULL,
			.val = THREADS
		},
		{
			.name = "phenotypes",
			.has_arg = required_argument,
			.flag = NULL,
			.val = PHENOTYPES
		},
		{
			.name = "dosage",
			.has_arg = required_argument,
			.flag = NULL,
			.val = DOSAGE
		},
		{0}
	};

	int opt_code;

	do {
		opt_code = getopt_long(argc, argv, "", options, NULL);
		switch (opt_code) {

                        case VERSION: {
                          vflag += 1;
                          break;
                        }

                        case 'f': {
                          fflag += 1;
                          break;
                        }
			case GENOTYPES: {
                          gflag += 1;
                          garg = optarg;
			  break;
			}
			case CONVERT: {
                          cflag += 1;
                          *outputA1File = optarg;
			  break;
			}
			case THREADS: {
                                tflag += 1;
				long int arg;
				char* endptr;
				arg = strtol(optarg, &endptr, 10);
				if (
					*optarg != '\0' && *endptr == '\0'
					&& 0 < arg && arg <= UINT16_MAX) {
					*computeThreads = arg;
				} else {
                                  error("Could not parse number of threads");
                                }
				break;
			}
			case PHENOTYPES: {
                                pflag += 1;
                                if (!can_read(optarg)) {
                                  error("Cannot read phenotype file");
                                }
				*inputPhenotypeFile = optarg;
				break;
			}
			case DOSAGE: {
                                dflag += 1;
				*outputDosageFile = optarg;
				break;
			}
		}
                if (opt_code == '?') {
                  error("Unrecognised argument");
                  exit(-1);
                }
	} while (opt_code != -1);

        int trailing = argc - optind;

        if (fflag > 0) {
          error("The flag -f is unimplemented");
        }

        if (vflag > 1 || gflag > 1 || cflag > 1 || tflag > 1 || pflag > 1
            || dflag > 1 || fflag > 1) {

          error("Some flags provided more than once");
        }

        if (vflag == 1) {
          if (gflag != 0 ||
            cflag != 0 ||
            tflag != 0 ||
            pflag != 0 ||
            dflag != 0 || trailing > 0) {

            error("Version flag must be supplied alone");
          }

          print_version();
          exit(0);
        }

        if (gflag == 1) {
          if (!(ends_with(garg, ".a1")
            || ends_with(garg, ".bgen")
            || ends_with(garg, ".gen"))) {

            error("Must read .a1 or .bgen file");
          }
          if (!can_read(garg)) {
            error("Cannot read genotype file");
          }
          if (ends_with(garg, ".bgen")) {
            *inputBgenFile = garg;
          } else if (ends_with(garg, ".a1")) {
            *inputA1File = garg;
          } else if (ends_with(garg, ".gen")) {
            *inputGenFile = garg;
          } else {
            error("Logic error");
          }
        }

        if (cflag == 1) {
          if (pflag == 1) {
            error("If convert flag provided, phenotypes must not be provided");
          }
          if (gflag != 1) {
            error("If convert flag provided, genotype flag must be provided");
          }
          if (trailing > 0) {
            error("Argument error");
          }
          if (!ends_with(*outputA1File, ".a1")) {
            error("Must convert to .a1 file");
          }
          if (*inputBgenFile == NULL) {
            error("Input .bgen file must be provided");
          }
          if (*inputA1File != NULL) {
            error("Cannot convert from .a1 file");
          }
          *action = ACT_BGA1;
        }

        if (pflag) {
          if (cflag == 1) {
            error("If phenotypes provided, convert flag must not be provided");
          }
          if (dflag == 1) {
            error("If phenotypes provided, dosage flag must not be provided");
          }
          if (trailing == 0) {
            error("Output GWAS directory must be provided");
          }
          if (trailing > 1) {
            error("Argument error");
          }
          if (gflag != 1) {
            error("If phenotype flag provided, genotype flag must be provided");
          }
          if (*inputBgenFile != NULL) {
            error("Must perform GWAS on .a1 file");
          }
          if (*inputA1File == NULL && *inputGenFile == NULL) {
            error("Must perform GWAS on .a1 file");
          }
          if (*outputDosageFile != NULL) {
            error("Cannot output dosages and do GWAS all at once");
          }
          *outputGwasDirectory = argv[optind];
          char *strings[] = {
            "/bin/mkdir",
            "-p",
            *outputGwasDirectory,
            NULL
          };

          char *command = join(strings, " ");
          system(command);
          free(command);
          if (*inputA1File != NULL) {
            *action = ACT_GWA1;
          } else if (*inputGenFile != NULL) {
            *action = ACT_GWGE;
          } else {
            error("Logic"); // Should have already crashed.
          }
        }

        if (dflag == 1) {
          if (trailing > 0) {
            error("Argument error");
          }
          if (gflag != 1) {
            error("If dosage flag provided, genotype flag must be provided");
          }
          if (cflag == 1) {
            error("If dosage flag provided, convert flag must not be provided");
          }
          if (pflag == 1) {
            error("If dosage flag provided, phenotypes must not be provided");
          }
          if (*outputDosageFile == NULL) {
            error("Logic error");
          }
          *action = ACT_DOSE;
        } 
        
        if (*action == -1) {
          error("Could not determine action");
        }

        return;
}
