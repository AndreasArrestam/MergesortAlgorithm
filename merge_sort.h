#include <stdbool.h>

#ifdef MEASURE
struct mergesort_timing
{
  // Monitors general algorithm start and stop time
  struct timespec start, stop;
};
#endif
struct mergesort_thread
{
	int id;
	int left;
	int right;
    int* array;
    int array_size;
    int sub_array_size;
    pthread_mutex_t array_lock;
    bool sort_completed;
    bool is_merged;
};


void sequential_merge_sort(int *array, int left, int right);
void* simple_parallel_merge_sort(void * buffer);
void* fully_parallel_merge_sort(void * buffer);
void parallel_sort(int *array, int left, int right);
void sort_array(int *array, int size);