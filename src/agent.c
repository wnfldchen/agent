// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "threadpipe.h"
#include "writethread.h"
#include "computethread.h"
#include "readthread.h"
#include "bgenformat.h"
#include "file.h"
#include "options.h"
#include "errors.h"
#include "read.h"
#include "write.h"
#include "memman.h"
#include "matrix.h"
#include "phenotype.h"
#include "statistics.h"
// agent.c

void loop_each_variant(FILE* inputFile, FILE* outputFile, char *output_dir, t_phenotype *phenos, BgenHeader *bgenHeader, AgentHeader *agentHeader, uint16_t computeThreads, int action) {
    assert(!(bgenHeader == NULL && agentHeader == NULL));
    uint32_t m;
    int header_bgen;
    if (bgenHeader != NULL) {
        m = bgenHeader->m;
        header_bgen = 1;
    } else {
        m = agentHeader->m;
        header_bgen = 0;
    }
    int header_agent = !header_bgen;
	// I/O Threads
	pthread_t write_thread;
	pthread_t read_thread;
	// Compute Threads
	pthread_t* compute_threads = create_buf(
			sizeof(compute_threads[0]), computeThreads);
	// Thread pipes
	ThreadPipe write_pipe;
	ThreadPipe read_pipe;
	// Start thread pipes
	initialize_threadpipe(&write_pipe);
	initialize_threadpipe(&read_pipe);
	// I/O Thread arguments
	WriteThreadArgs write_args = {
	        .agentHeader = agentHeader,
	        .pipe_in = &write_pipe,
	        .outputFile = outputFile,
	        .m = m,
	        .is_agent = header_agent,
	        .action = action
	};
	ReadThreadArgs read_args = {
	        .pipe_out = &read_pipe,
	        .inputFile = inputFile,
	        .m = m,
	        .is_zstd = header_bgen,
	        .is_agent = header_agent
	};
	// Compute Threads arguments
	ComputeThreadArgs compute_args = {
	        .agentHeader = agentHeader,
		.pipe_in = &read_pipe,
		.pipe_out = &write_pipe,
		.is_zstd = header_bgen,
		.is_agent = header_agent,
		.action = action,
                .output_dir = output_dir,
                .phenos = phenos
	};
	// Start threads
	initializeargs_write_thread(&write_thread, &write_args);
	for (uint16_t i = 0; i < computeThreads; i += 1) {
		initializeargs_compute_thread(&compute_threads[i], &compute_args);
	}
	initializeargs_read_thread(&read_thread, &read_args);
	// Wait for and cleanup threads
	destroy_write_thread(&write_thread, &write_args);
	for (uint16_t i = 0; i < computeThreads; i += 1) {
		destroy_compute_thread(&compute_threads[i], &compute_args);
	}
	destroy_read_thread(&read_thread, &read_args);
	destroy_buf(compute_threads);
	// Cleanup thread pipes
	destroy_threadpipe(&write_pipe);
	destroy_threadpipe(&read_pipe);
}

void close_files(FILE* inputFile, FILE* outputFile) {
	close_file(inputFile);
	close_file(outputFile);
}

int agent_main(int argc, char** argv) {
        int action = -1;
	char* inputBgenFile = NULL;
        char* inputGenFile = NULL;
	char* inputA1File = NULL;
	char* outputA1File = NULL;
	char* inputPhenotypeFile = NULL;
	char* inputPhenotypeDirectory = NULL;
	char* outputGwasDirectory = NULL;
	char* outputDosageFile = NULL;
	uint16_t computeThreads = 1;

	read_options(
          argc,
          argv,
          &action,
          &inputBgenFile,
          &inputGenFile,
          &inputA1File,
          &outputA1File,
          &inputPhenotypeFile,
          &outputGwasDirectory,
          &outputDosageFile,
          &computeThreads);

	BgenHeader bgenHeader;
	AgentHeader* agentHeader = NULL;

        FILE *inputFile =  (FILE *)NULL;
        FILE *outputFile = (FILE *)NULL;

        switch (action) {
          case ACT_BGA1: {
            inputFile = open_file(inputBgenFile, "r");
            outputFile = open_file(outputA1File, "w");
            read_bgen_header(inputFile, &bgenHeader);
            validate_bgen_header(&bgenHeader);
            agentHeader = create_agent_header_bgen(&bgenHeader);
            write_agent_header(outputFile, agentHeader);
            loop_each_variant(inputFile, outputFile, outputGwasDirectory, NULL, &bgenHeader, agentHeader, computeThreads, action);
            write_agent_header(outputFile, agentHeader);
            destroy_agent_header(agentHeader);
            close_files(inputFile, outputFile);
            break;
          }
          
          case ACT_GWA1: {
            assert(inputA1File != NULL);
            assert(inputGenFile == NULL);
            assert(inputBgenFile == NULL);
            assert(outputGwasDirectory != NULL);
            assert(inputPhenotypeFile != NULL);
            t_matrix y;
            t_matrix yt;
            t_matrix obs;
            t_matrix denom;
            int N = numSamples(inputA1File);
            int M = numVariants(inputA1File);
            int D0 = numPhenotypes(inputPhenotypeFile);
            int D;
            if (D0 % (sysconf(_SC_PAGESIZE) / 8) == 0) {
              D = D0;
            } else {
              D = (D0 / (sysconf(_SC_PAGESIZE) / 8) + 1) * (sysconf(_SC_PAGESIZE) / 8);
            }
            load_phenotypes2(inputPhenotypeFile, &y, &obs, &denom, N, D0, D);
            t_phenotype phenos;
            yt = create(D, N);
            for (int i = 0; i < N; i++) {
              for (int j = 0; j < D; j++) {
                yt.X[D * i + j] = y.X[N * j + i];
              }
            }
            phenos.y = &y;
            phenos.yt = &yt;
            phenos.obs = &obs;
            phenos.denom = &denom;
            if (D <= 0) {
              error("No phenotypes provided");
            }
            //if ((D * 8) % sysconf(_SC_PAGESIZE) != 0) {
            //  error("Number of phenotypes must be a multiple of the page size divided by 8");
            //}
            if (N <= 2) {
              error("Phenotypes describe two or fewer subjects");
            }
            
            char *fi_list[] = { outputGwasDirectory, "info.txt", NULL };
            char *fi_name = join(fi_list, "/");
            FILE *fi_file = fopen(fi_name, "w");
            if (fi_file == NULL) {
              error("Error opening file");
            }
            free(fi_name);
            fprintf(fi_file, "D = %d\nD0 = %d\nM = %d\nN = %d\n", D, D0, M, N);
            fclose(fi_file);

            char *fu_list[] = { outputGwasDirectory, "unpack", NULL };
            char *fu_name = join(fu_list, "/");
            FILE *fu_file = fopen(fu_name, "w");
            if (fu_file == NULL) {
              error("Error opening file");
            }
            #include "unpack.h"
            if (fwrite(unpack, sizeof(unsigned char), unpack_len, fu_file) != unpack_len) {
              error("Could not write unpacking script");
            }
            fclose(fu_file);
            chmod(fu_name, S_IRWXU);
            free(fu_name);

            inputFile = open_file(inputA1File, "r");
            char *fl_list[] = { outputGwasDirectory, "log.txt", NULL };
            char *fl_name = join(fl_list, "/");
            outputFile = open_file(fl_name, "w");
            if (outputFile == NULL) {
              error("Error opening file");
            }
            free(fl_name);
            char *fb_list[] = { outputGwasDirectory, "beta.bin", NULL };
            char *fb_name = join(fb_list, "/");
            FILE *fb = fopen(fb_name, "w");
            if (fb == NULL) {
              error("Error opening file");
            }
            free(fb_name);
            int z;
            off64_t size = (off64_t)D * (off64_t)M * (off64_t)sizeof(double);
            printf("%d %d %d %lld %d\n", D, M, sizeof(double), (off64_t)D*(off64_t)M*(off64_t)sizeof(double), sizeof(off64_t));
            printf("size = %lld\n", size);
            z = posix_fallocate64(fileno(fb), 0, size);
            if (z != 0) {
              fprintf(stderr, "%d %d: %s\n", z, errno, strerror(errno));
              error("Could not resize output file");
            }
            fclose(fb);
            char *fs_list[] = { outputGwasDirectory, "se.bin", NULL };
            char *fs_name = join(fs_list, "/");
            FILE *fs = fopen(fs_name, "w");
            if (fs == NULL) {
              error("Error opening file");
            }
            free(fs_name);
            z = posix_fallocate64(fileno(fs), 0, size);
            if (z != 0) {
              error("Could not resize output file");
            }
            fclose(fs);
            char *ft_list[] = { outputGwasDirectory, "tstat.bin", NULL };
            char *ft_name = join(ft_list, "/");
            FILE *ft = fopen(ft_name, "w");
            if (ft == NULL) {
              error("Error opening file");
            }
            free(ft_name);
            z = posix_fallocate64(fileno(ft), 0, size);
            if (z != 0) {
              error("Could not resize output file");
            }
            fclose(ft);
            char *fp_list[] = { outputGwasDirectory, "pval.bin", NULL };
            char *fp_name = join(fp_list, "/");
            FILE *fp = fopen(fp_name, "w");
            if (fp == NULL) {
              error("Error opening file");
            }
            free(fp_name);
            z = posix_fallocate64(fileno(fp), 0, size);
            if (z != 0) {
              error("Could not resize output file");
            }
            fclose(fp);

            uint64_t l0 = read_l0(inputFile);
            agentHeader = create_agent_header(l0);
            read_agent_header(inputFile, agentHeader);
            validate_agent_header(agentHeader);
            loop_each_variant(inputFile, outputFile, outputGwasDirectory, &phenos, NULL, agentHeader, computeThreads, action);
            destroy_agent_header(agentHeader);
            close_files(inputFile, outputFile);
            break;
          }

          case ACT_DOSE: {
              if (outputDosageFile == NULL) {
                  error("No dosage file");
              }
              outputFile = open_file(outputDosageFile, "w");
              if (inputBgenFile) {
                  inputFile = open_file(inputBgenFile, "r");
                  read_bgen_header(inputFile, &bgenHeader);
                  validate_bgen_header(&bgenHeader);
                  loop_each_variant(inputFile, outputFile, outputGwasDirectory, NULL, &bgenHeader, agentHeader, computeThreads, action);
                  close_files(inputFile, outputFile);
              } else if (inputA1File) {
                  inputFile = open_file(inputA1File, "r");
                  uint64_t l0 = read_l0(inputFile);
                  agentHeader = create_agent_header(l0);
                  read_agent_header(inputFile, agentHeader);
                  validate_agent_header(agentHeader);
                  loop_each_variant(inputFile, outputFile, outputGwasDirectory, NULL, NULL, agentHeader, computeThreads, action);
                  destroy_agent_header(agentHeader);
                  close_files(inputFile, outputFile);
              } else if (inputGenFile) {
                  error("Unimplemented");
              } else {
                  error("No genotype file");
              }
              break;
          }

          case ACT_GWGE:
          case ACT_GWBG: {
            error("Unimplemented action specified");
            break;
          }

          default: {
            error("Could not determine action from command line");
            break;
          }
        }
	return 0;
}
