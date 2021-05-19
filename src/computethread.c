// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
#include <memman.h>
#include "agentformat.h"
#include "file.h"
#include "options.h"
#include "threadpipe.h"
#include "statistics.h"
#include "computetask.h"
#include "matrix.h"
#include "phenotype.h"
#include "computethread.h"
#include "dosageencode.h"
#include "compress.h"
#include "decompress.h"
// computethread.c

void *compute_thread_start_routine(void *arg)
{

    uint32_t batch_size = 500;  // REMARK: Must be the same as other batch_size variables
    ComputeThreadArgs *args = arg;
    AgentHeader* agentHeader = args->agentHeader;
    ThreadPipe *pipe_in = args->pipe_in;
    ThreadPipe *pipe_out = args->pipe_out;
    int is_zstd = args->is_zstd;
    int is_agent = args->is_agent;
    int action = args->action;
    char *outputGwasDirectory = args->output_dir;
    t_phenotype *phenos = args->phenos;

    assert(action == ACT_BGA1 || action == ACT_DOSE || action == ACT_GWA1);

    assert(action != ACT_BGA1 || !is_agent);

    t_matrix y;
    t_matrix yt;
    t_matrix obs;
    t_matrix denom;

    char *fb_name = NULL;
    char *fs_name = NULL;
    char *ft_name = NULL;
    char *fp_name = NULL;
    t_matrix g;
    t_matrix beta;
    t_matrix se;
    t_matrix tstat;
    t_matrix pval;
    t_matrix b1;
    t_matrix w1;
    t_matrix w2;

    if (action == ACT_GWA1) {
        y = *(phenos->y);
        yt = *(phenos->yt);
        obs = *(phenos->obs);
        denom = *(phenos->denom);
        assert(outputGwasDirectory != NULL);
        char *fb_list[] = { outputGwasDirectory, "beta.bin", NULL };
        fb_name = join(fb_list, "/");
        char *fs_list[] = { outputGwasDirectory, "se.bin", NULL };
        fs_name = join(fs_list, "/");
        char *ft_list[] = { outputGwasDirectory, "tstat.bin", NULL };
        ft_name = join(ft_list, "/");
        char *fp_list[] = { outputGwasDirectory, "pval.bin", NULL };
        fp_name = join(fp_list, "/");
        int N = y.N;
        int D = y.D;
        g = create(N, batch_size);
        beta = create(D, batch_size);
        se = create(D, batch_size);
        tstat = create(D, batch_size);
        pval = create(D, batch_size);
        b1 = create(batch_size, 1);
        w1 = create(D, batch_size);
        w2 = create(N, D);
    }

    size_t *variants = create_buf(sizeof(variants[0]), batch_size);
    ComputeTask **tasks = create_buf(sizeof(tasks[0]), batch_size);
    while (1) {
        int pipe_state =
            remove_threadpipe(pipe_in, variants, (void **) tasks,
                              batch_size);
        if (pipe_state == THREADPIPE_OP_CLOSED) {
            break;
        }
        uint32_t i = 0;

        if (action == ACT_GWA1) {
            for (; i < batch_size && tasks[i] != NULL; i++) {
                ComputeTask *task = tasks[i];
                if (is_zstd) {
                    zstd_decompress(task);
                } else if (is_agent) {
                    agent_decompress(task, agentHeader);
                }
            }

            int M = i;
            int N = y.N;
            int D = y.D;
            g.D = M;
            beta.D = M;
            se.D = M;
            tstat.D = M;
            pval.D = M;
            b1.N = M;
            w1.D = M;

            for (int ii = 0; ii < M; ii++) {
                ComputeTask *task = tasks[ii];
                UncompressedProbabilityData *zz =
                    &task->prob.uncompressed;

                if (zz->pmin != 2 || zz->pmax != 2) {
                    error("Ploidy of genotype file must be 2");
                }

                uint16_t *p = (uint16_t *) (zz->data);
                for (uint32_t jj = 0; jj < N; jj++) {
                    put(g,
                        2.0 * ((double) p[jj]) / ((double) UINT16_MAX),
                        jj, ii);

                }
            }

            

            regression(g, y, yt, obs, denom, beta, se, tstat, pval, b1, w1, w2);


            for (int ii = 0; ii < M; ii++) {
                int variant = variants[ii];
                size_t length = D * sizeof(double);
                assert(variant >= 0);
                off64_t offset = (off64_t)variant * (off64_t)length;

                int fb = open(fb_name, O_RDWR, S_IRUSR | S_IWUSR);
                if (fb < 0) {
                    error("Could not open output file");
                }
                char *fb_addr =
                    mmap64(NULL, length, PROT_WRITE, MAP_SHARED, fb,
                         offset);
                if (fb_addr == MAP_FAILED) {
                    error("Could not memory map output file");
                }
                memcpy(fb_addr, (char *) (&(beta.X[D * ii])), length); //
                if (munmap(fb_addr, length) != 0) {
                    error("Error releasing mapped memory");
                }
                if (close(fb) != 0) {
                    error("Could not close file descriptor");
                }

                int fs = open(fs_name, O_RDWR, S_IRWXU | S_IRGRP);
                if (fs < 0) {
                    error("Could not open output file");
                }
                char *fs_addr =
                    mmap64(NULL, length, PROT_WRITE, MAP_SHARED, fs,
                         offset);
                if (fs_addr == MAP_FAILED) {
                    error("Could not memory map output file");
                }
                memcpy(fs_addr, (char *) (&(se.X[D * ii])), length); //
                munmap(fs_addr, length);
                if (close(fs) != 0) {
                    error("Could not close file descriptor");
                }

                int ft = open(ft_name, O_RDWR, S_IRWXU | S_IRGRP);
                if (ft < 0) {
                    error("Could not open output file");
                }
                char *ft_addr =
                    mmap64(NULL, length, PROT_WRITE, MAP_SHARED, ft,
                         offset);
                if (ft_addr == MAP_FAILED) {
                    error("Could not memory map output file");
                }
                memcpy(ft_addr, (char *) (&(tstat.X[D * ii])), length); //
                munmap(ft_addr, length);
                if (close(ft) != 0) {
                    error("Could not close file descriptor");
                }

                int fp = open(fp_name, O_RDWR, S_IRWXU | S_IRGRP);
                if (fp < 0) {
                    error("Could not open output file");
                }
                char *fp_addr =
                    mmap64(NULL, length, PROT_WRITE, MAP_SHARED, fp,
                         offset);
                if (fp_addr == MAP_FAILED) {
                    error("Could not memory map output file");
                }
                memcpy(fp_addr, (char *) (&(pval.X[D * ii])), length); //
                munmap(fp_addr, length);
                if (close(fp) != 0) {
                    error("Could not close file descriptor");
                }
            }
        } else {

            for (; i < batch_size && tasks[i] != NULL; i++) {
                size_t variant = variants[i];
                ComputeTask *task = tasks[i];
                if (is_zstd) {
                    zstd_decompress(task);
                } else if (is_agent) {
                    agent_decompress(task, agentHeader);
                }
                switch (action) {
                case ACT_BGA1:{
                        dosage_encode(&task->agentFseHeader,
                                      &task->agentProb,
                                      &task->prob.uncompressed);
                        compress(&task->agentFseHeader,
                                 &task->agentProb, &task->prob.uncompressed);
                        break;
                    }
                case ACT_DOSE:{
                        break;
                    }

                default:{
                        error("Unknown compute task");
                        break;
                    }
                }
            }
        }
        batch_size -= batch_size - i;
        append_threadpipe(pipe_out, variants, (void **) tasks, batch_size);
    }
    destroy_buf(variants);
    destroy_buf(tasks);
    if (action == ACT_GWA1) {
        free(fb_name);
        free(fs_name);
        free(ft_name);
        free(fp_name);
        destroy(g);
        destroy(beta);
        destroy(se);
        destroy(tstat);
        destroy(pval);
        destroy(b1);
        destroy(w1);
        destroy(w2);
    }
    return NULL;
}

void initializeargs_compute_thread(pthread_t * thread,
                                   ComputeThreadArgs * args)
{
    pthread_create(thread, NULL, compute_thread_start_routine, args);

#ifdef __APPLE__
#else
    pthread_setname_np(*thread, "agent.compute");
#endif
}

void destroy_compute_thread(pthread_t * thread, ComputeThreadArgs * args)
{
    pthread_join(*thread, NULL);
}
