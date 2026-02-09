#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "merge_sort.h"
#include <stdbool.h>
#if NB_THREADS > 0
#include <pthread.h>
#include <string.h>
#endif

int *tmp_arr = NULL;

#if NB_THREADS > 0
pthread_barrier_t thread_pool_barrier;
pthread_t thread[NB_THREADS];
struct mergesort_thread thread_data[NB_THREADS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif


void print_array(int *arr, int start, int size) {
    pthread_mutex_lock(&mutex);
    printf("\nArray: ");
    putchar('{');
    int i;
    for (i = start; i < start + size - 1; i++)
        printf("%d, ", arr[i]);

    printf("%d}\n", arr[i]);
    pthread_mutex_unlock(&mutex);
}

void print_array_wo_mutex(int *arr, int start, int size) {
    printf("\nArray: ");
    putchar('{');
    int i;
    for (i = start; i < start + size - 1; i++)
        printf("%d, ", arr[i]);
    printf("%d}\n", arr[i]);
}


void sequential_merge_sort(int *array, int left, int right)
{   

    if (left < right)
    {
        int mid = left + (right - left) / 2;
        

        sequential_merge_sort(array, left, mid);
        sequential_merge_sort(array, mid + 1, right);

        // Merge the sorted halves
        int n1 = mid - left + 1; // compute number of elements
        int n2 = right - mid;

        int *L = (int *)malloc(n1 * sizeof(int)); //allocate memory of size n1 and n2
        int *R = (int *)malloc(n2 * sizeof(int));

        for (int i = 0; i < n1; i++) //populate the arrays L and R
            L[i] = array[left + i];
        for (int j = 0; j < n2; j++)
            R[j] = array[mid + 1 + j];

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2)
        {
            if (L[i] <= R[j])
            {
                array[k] = L[i];
                i++;
            }
            else
            {
                array[k] = R[j];
                j++;
            }
            k++;
        } // perform the swapping, add the lowest value of L or R and increase appropriate index

        while (i < n1)
        {
            array[k] = L[i];
            i++;
            k++;
        }

        while (j < n2)
        {
            array[k] = R[j];
            j++;
            k++;
        } // if we have added all values of one subset, add the rest of the other subset

        free(L);
        free(R);
    }
}

void* simple_parallel_merge(int* array, int left, int right)
{
    
    // Merge the sorted halves
    int mid = left + (right - left) / 2;

    int n1 = mid - left + 1; // compute number of elements
    int n2 = right - mid;

    int *L = (int *)malloc(n1 * sizeof(int)); //allocate memory of size n1 and n2
    int *R = (int *)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) //populate the arrays L and R
        L[i] = array[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = array[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            array[k] = L[i];
            i++;
        }
        else
        {
            array[k] = R[j];
            j++;
        }
        k++;
    } // perform the swapping, add the lowest value of L or R and increase appropriate index

    while (i < n1)
    {
        array[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        array[k] = R[j];
        j++;
        k++;
    } // if we have added all values of one subset, add the rest of the other subset
    free(L);
    free(R);
}

void* simple_parallel_merge_sort(void * buffer)
{
    struct mergesort_thread *args = (struct mergesort_thread*) buffer;
   
    sequential_merge_sort(args->array, args->left, args->right);
    
    pthread_barrier_wait(&thread_pool_barrier);
    

    int chunk_size = args->right - args->left + 1;
    int step_size = 1; // number of chunks to merge together
    int num_chunks = NB_THREADS;

    while (step_size < num_chunks) {
        int merge_start = chunk_size * args->id;
        int merge_end = merge_start + 2 * step_size * chunk_size - 1;

        if (merge_start < args->array_size) {
            if (merge_end >= args->array_size)
                merge_end = args->array_size - 1; //check basically

            // Only "first thread in pair" does the merge
            if (args->id % (2 * step_size) == 0) {
                simple_parallel_merge(args->array, merge_start, merge_end);
            }
        }

        pthread_barrier_wait(&thread_pool_barrier);
        step_size *= 2;
    } 
}

/* Compute rank of element from array A in array B
Returns the index in array B where 'value' should be inserted
(number of elements in B that are <= value) */
int compute_rank_a(int value, int *array, int b_start, int b_end)
{
    int left = b_start;
    int right = b_end + 1;
    
    // Binary search for upper bound (rightmost position)
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (array[mid] <= value) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return left;
}

/* Compute rank of element from array B in array A
Returns the index in array A where 'value' should be inserted
(number of elements in A that are < value) */
int compute_rank_b(int value, int *array, int a_start, int a_end)
{
    int left = a_start;
    int right = a_end + 1;
    
    // binary search for lower bound (leftmost position)
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (array[mid] < value) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return left;
}

void* fully_parallel_merge_sort(void * buffer)
{
    struct mergesort_thread *args = (struct mergesort_thread*) buffer;
#if NB_THREADS > 0
    int *array = args->array;
    int id = args->id;
    int size = args->array_size;
    int chunk = (size + NB_THREADS - 1)/ NB_THREADS; // to get residual
    int start = id * chunk;
    int end = start + chunk;
    

    
    // sorti ti all sequenital
    sequential_merge_sort(array, start, end - 1);
    
    static int *rank_ab = NULL;
    static int *rank_ba = NULL;
    static int *temp = NULL;
    
    if(args->id == 0){
        // initialize temporary arrays
        rank_ab = malloc(args->array_size * sizeof(int));
        rank_ba = malloc(args->array_size * sizeof(int));
        temp = malloc(args->array_size * sizeof(int));
    }
    
    pthread_barrier_wait(&thread_pool_barrier);
    // merge level loop with i
    for(int i = 1; i < NB_THREADS; i *= 2){
        //merge block loop with j
        for (int j = 0; j * i * chunk < size; j += 2){
            
            int block = i * chunk;
            int a_start = j * block;
            int a_end   = a_start + block - 1;
            int b_start = a_end + 1;
            int b_end   = b_start + block - 1;

            int size_a = a_end - a_start + 1;
            int size_b = b_end - b_start + 1;
            int size_temp = size_a + size_b;
            
            // thread processes a portion of array A
            int chunk_a = (size_a + NB_THREADS - 1) / NB_THREADS; //the size of chunk for each thread
            int start_a = args->id * chunk_a; //the starting index for each thread within array A
            int end_a = start_a + chunk_a; //the ending index for each thread within array A
            if(end_a > size_a) end_a = size_a;
            
            for(int k = start_a; k < end_a; k++){
                rank_ab[k] = compute_rank_a(args->array[a_start + k], args->array, b_start, b_end) - b_start;
            }
            
            //thread processes a portion of array B
            int chunk_b = (size_b + NB_THREADS - 1) / NB_THREADS; //the size of chunk for each thread
            int start_b = args->id * chunk_b; //the starting index for each thread within array B
            int end_b = start_b + chunk_b; //the ending index for each thread within array B
            if(end_b > size_b) end_b = size_b;
            
            for(int k = start_b; k < end_b; k++){
                rank_ba[k] = compute_rank_b(args->array[b_start + k], args->array, a_start, a_end) - a_start;
            }
            
            pthread_barrier_wait(&thread_pool_barrier);
        
            // Merge results into temp array
            for(int k = start_a; k < end_a; k++){
                temp[k + rank_ab[k]] = args->array[a_start + k];
            }
            for(int k = start_b; k < end_b; k++){
                temp[k + rank_ba[k]] = args->array[b_start + k];
            }
            
            pthread_barrier_wait(&thread_pool_barrier);
            
            if(args->id == 0){
                // copy back to original array
                for(int k = 0; k < size_temp; k++){
                    args->array[a_start + k] = temp[k];
                }
            }
            pthread_barrier_wait(&thread_pool_barrier);
        }
    }
    
    if(args->id == 0){
        free(temp);
        free(rank_ab);
        free(rank_ba);
    }
#endif
    return NULL;   
}

void print_thread_data(struct mergesort_thread thread_data){
    printf("\n***********THREAD DATA PRINT***************");
    printf("\nPrinting thread data: %d", thread_data.left);
    printf("\nid %d", thread_data.id);
    printf("\nleft %d", thread_data.left);
    printf("\nright %d", thread_data.right);
    printf("\narray size %d", thread_data.sub_array_size);
    printf("\n*******************************");
    printf("\n");
}

void parallel_sort(int *array, int left, int right)
{
#if NB_THREADS > 0
    pthread_attr_t thread_attr;
    int i;

    // Initialize attributes
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
    pthread_barrier_init(&thread_pool_barrier, NULL, NB_THREADS);

    // int num_elem = sizeof(array)/sizeof(array[0]);
    
    int num_elem = right - left + 1;

    // Create a thread pool
    for (i = 0; i < NB_THREADS; i++)
    {
        thread_data[i].id = i;
        thread_data[i].array = array;
        thread_data[i].array_size = num_elem;
        thread_data[i].sort_completed = false;
        
        pthread_mutex_init(&thread_data[i].array_lock, NULL);
        
        
        int chunk_size = num_elem / NB_THREADS;
        int residual = num_elem % NB_THREADS;
        
        thread_data[i].left = chunk_size * i;
#if ALGORITHM == 1
        if(i < NB_THREADS - 1){
            thread_data[i].right = chunk_size * (i+1) - 1;
            thread_data[i].sub_array_size = thread_data[i].right - thread_data[i].left + 1;
            pthread_create(&thread[i], &thread_attr, &simple_parallel_merge_sort, &thread_data[i]);
        }

        else{
            thread_data[i].right = chunk_size * (i+1) + residual - 1;
            thread_data[i].sub_array_size = thread_data[i].right - thread_data[i].left + 1;
            pthread_create(&thread[i], &thread_attr, &simple_parallel_merge_sort, &thread_data[i]);
        }

#endif

#if ALGORITHM == 2
    tmp_arr = malloc(thread_data[i].array_size * sizeof(int));
    
    // Set the boundaries for this thread's chunk
    if(i < NB_THREADS - 1){
        thread_data[i].right = chunk_size * (i+1) - 1;
    } else {
        thread_data[i].right = chunk_size * (i+1) + residual - 1;
    }
    
    pthread_create(&thread[i], &thread_attr, &fully_parallel_merge_sort, &thread_data[i]);
#endif

    }
#endif

    for (i = 0; i < NB_THREADS; i++)
	{
        pthread_join(thread[i], NULL);
	}
}

void sort_array(int *array, int size)
{
#if ALGORITHM == 0
    sequential_merge_sort(array, 0, size - 1);
#else
    parallel_sort(array, 0, size - 1);
#endif
}