#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "merge_sort.h"

#ifndef ALGORITHM
#define ALGORITHM 0
#endif

#ifndef NUM_THREADS
#define NUM_THREADS 0
#endif


double timediff(struct timespec *begin, struct timespec *end)
{
    double sec = 0.0, nsec = 0.0;
    if ((end->tv_nsec - begin->tv_nsec) < 0)
    {
        sec = (double)(end->tv_sec - begin->tv_sec - 1);
        nsec = (double)(end->tv_nsec - begin->tv_nsec + 1000000000);
    }
    else
    {
        sec = (double)(end->tv_sec - begin->tv_sec);
        nsec = (double)(end->tv_nsec - begin->tv_nsec);
    }
    return sec + nsec / 1E9;
}

void print_help() {
    printf("Usage: ./mergesort [options]\n");
    printf("Options:\n");
    printf("  -h, --help     Display this help message\n");
    printf("  -s, --size N   Set array size (N > 0)\n");
}

int parse_args(int argc, char **argv, int *size) {
    if (argc < 2) {
        print_help();
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } else if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--size") == 0) && i + 1 < argc) {
            *size = atoi(argv[i + 1]);
            if (*size <= 0) {
                fprintf(stderr, "Error: Size must be positive\n");
                return 0;
            }
            i++;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_help();
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv)
{
    int size=1;
    parse_args(argc, argv, &size);

    printf("Array size: %d\n", size);

    int *array = (int *)malloc(sizeof(int) * size);
    if (array == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        return EXIT_FAILURE;
    }
    // Initialize array with random values
    for (int i = 0; i < size; i++)
    {
        array[i] = rand() % (10 * size);
    }
#ifdef MEASURE
    struct mergesort_timing global;
    clock_gettime(CLOCK_MONOTONIC, &global.start);
#endif 
    sort_array(array, size);
#ifdef MEASURE
    clock_gettime(CLOCK_MONOTONIC, &global.stop);
    printf("Time: %f seconds ALGORITHM: %d NB_THREADS: %d\n", timediff(&global.start, &global.stop), ALGORITHM, NB_THREADS);
#else
    int *temp = malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        temp[i] = array[i];
    }
    sequential_merge_sort(temp, 0, size - 1);
    
    for (int i = 0; i < size; i++)
    {
        if (array[i] != temp[i]) {
            printf("Error: Array is not sorted correctly at index %d (expected %d, got %d)\n", i, temp[i], array[i]);
            free(array);
            free(temp);
            return EXIT_FAILURE;
        }
    }
    printf("Array is sorted correctly.\n");
    free(array);
    free(temp);
#endif
    return EXIT_SUCCESS;
}
