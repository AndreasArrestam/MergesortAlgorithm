# MergesortAlgoritm

POSIX thread is required to run the algoritms.
To install on linux computers run the command: sudo apt install libc6-dev

NB_THREADS is the number of threads to run the algorithm on.
ALGORITHM decides what type of sorting algorithm to be used. 0: Sequential mergesort, 1:simple parallel mergesort, 2:fully parallel mergesort

To make the run-file type: 
make NB_THREADS={} ALGORITHM={} 

The makefile will make a executable that can be run with a argument for setting the size of the array. This need to be divisible with 2^n.

To run type ./mergesort-x-x -s {size of array}

To get more understanding of the mergesort algorithm: https://www.geeksforgeeks.org/dsa/merge-sort/

This code was developed for the course TDDD56 Multicore and GPU Programming
