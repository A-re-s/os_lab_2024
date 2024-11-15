#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>     
#include <stdbool.h> 

long long result  = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int start;
    int end;
    int mod;
} ThreadData;

void *calculate_factorial(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long local_result = 1;

    for (int i = data->start; i <= data->end; i++) {
        local_result = (local_result * i) % data->mod;
    }

    pthread_mutex_lock(&mut);
    result = (result * local_result) % data->mod;
    pthread_mutex_unlock(&mut);
}

int main(int argc, char **argv) {
    int pnum = 0, mod = 0, k = 0;

          
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
        {"pnum", required_argument, 0, 0},
        {"mod", required_argument, 0, 1},
        {"k", required_argument, 0, 2},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1) {
        switch (c) {
            case 0: 
                pnum = atoi(optarg);
                if (pnum <= 0) {
                    fprintf(stderr, "pnum <= 0\n");
                    return 1;
                }
                break;
            case 1: 
                mod = atoi(optarg);
                if (mod <= 0) {
                    fprintf(stderr, "pnum <= 0\n");
                    return 1;
                }
                break;
            case 2:
                k = atoi(optarg);
                if (k <= 0) {
                    fprintf(stderr, "k <= 0\n");
                    return 1;
                }
                break;
            default:
                printf("Index %d is out of options\n", option_index);
                return 1;
        }
    }
    

    pthread_t threads[pnum];
    ThreadData thread_data[pnum];

    int chunk_size = k / pnum;
    int remainder = k % pnum;

    int start = 1;
    for (int i = 0; i < pnum; i++) {
        int end = start + chunk_size - 1;
        if (i == pnum - 1) {
            end = k;
        }
        thread_data[i].start = start;
        thread_data[i].end = end;
        thread_data[i].mod = mod;
        start = end + 1;

        pthread_create(&threads[i], NULL, calculate_factorial, &thread_data[i]);
    }

     for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("result: %lld \n", result);
    return 0;
}
