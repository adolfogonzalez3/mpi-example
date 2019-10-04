/*
 *   Taken from the book. Augemented for easier reading and understanding.
 */
#include "mpi.h"    // MPI_Init, MPI_Comm_size, MPI_Comm_rank, MPI_Finalize
#include <chrono>   // std::chrono::seconds
#include <iostream> // std::std::cout, std::std::endl
#include <stdlib.h> // srandom, random
#include <thread>   // std::this_thread::sleep_for
#include <iomanip>  // std::setw
#include <time.h>


void CompareSplit(int, int*, int*, int*,int);
int IncOrder(const void*, const void*);

int main(int argc, char *argv[]) {

  int world_size, myrank, size_of_array, nlocal, oddrank, evenrank;
  int *elmnts, *relmnts, *wspace;
  bool show = false;
  MPI_Status status;

  time_t begin = time(NULL);

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  if(argc < 2){
      if(myrank == 0)
        std::cout << "This program sorts N numbers using MPI.\n"
                    << "The program expects the user to pass in N "
                    << "as a commandline argument.\n"
                    << "If the user wants to print the resulting array "
                    << "then an additional argument should be given.\n\n"
                    << "Example: mpirun -np 8 a.out 25000000\n"
                    << "Runs odd-even sorting on 25 million randomly generated "
                    << "numbers.\n\n"
                    << "Example: mpirun -np 8 a.out 250 1\n"
                    << "Runs odd-even sorting on 250 randomly generated numbers "
                    << "and then prints the resulting array.\n";
    MPI_Finalize();
    return 0;
  }

  if(myrank==0)
    std::cout << "Begin sorting..." << std::endl;

  size_of_array = atoi(argv[1]);
  show = argc > 2;
  nlocal = size_of_array / world_size;

  elmnts = new int[nlocal];
  relmnts = new int[nlocal];
  wspace = new int[nlocal];

  // Initialize the subarray with random values
  srandom(myrank);
  for (int i = 0; i < nlocal; i++)
    elmnts[i] = random() % size_of_array;

  // Sort the subarray with quick sort
  // in ascending order
  qsort(elmnts, nlocal, sizeof(int), IncOrder);

  // Determine the rank of the nodes that
  // will be communicated with during the
  // even and odd phases.
  if (myrank % 2 == 0) {
    oddrank = myrank - 1;
    evenrank = myrank + 1;
  } else {
    oddrank = myrank + 1;
    evenrank = myrank - 1;
  }

  /* Set the ranks of the processors at the end of the linear */
  if (oddrank == -1 || oddrank == world_size)
    oddrank = MPI_PROC_NULL;
  if (evenrank == -1 || evenrank == world_size)
    evenrank = MPI_PROC_NULL;

  /* Get into the main loop of the odd-even sorting algorithm */
  for (int i = 0; i < world_size - 1; i++) {
    if (i % 2 == 1) /* Odd phase */
      MPI_Sendrecv(elmnts, nlocal, MPI_INT, oddrank, 1, relmnts, nlocal,
                   MPI_INT, oddrank, 1, MPI_COMM_WORLD, &status);
    else /* Even phase */
      MPI_Sendrecv(elmnts, nlocal, MPI_INT, evenrank, 1, relmnts, nlocal,
                   MPI_INT, evenrank, 1, MPI_COMM_WORLD, &status);
    CompareSplit(nlocal, elmnts, relmnts, wspace, myrank < status.MPI_SOURCE);
  }

  if (show) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * myrank));

    std::cout << "I'm process " << myrank + 1 << " out of " << world_size << std::endl;
    for(int i = 0; i < nlocal; i++) {
      std::cout << std::setw(4) << elmnts[i] << " ";
    }
    std::cout << std::endl;
    if(myrank == 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(100 * world_size));
  }
  if(myrank == 0)
    std::cout << "Finished Sorting in: " << time(NULL) - begin << std::endl;

  delete[] elmnts;
  delete[] relmnts;
  delete[] wspace;
  MPI_Finalize();
  return 0;
}

/* This is the CompareSplit function */
void CompareSplit(int nlocal, int *elmnts, int *relmnts, int *wspace,
                  int keepsmall) {
  int i, j, k;
  for (i = 0; i < nlocal; i++)
    wspace[i] = elmnts[i]; /* Copy the elmnts array into the wspace array */
  if (keepsmall) {         /* Keep the nlocal smaller elements */
    for (i = j = k = 0; k < nlocal; k++) {
      if (j == nlocal || (i < nlocal && wspace[i] < relmnts[j]))
        elmnts[k] = wspace[i++];
      else
        elmnts[k] = relmnts[j++];
    }
  } else { /* Keep the nlocal larger elements */
    for (i = k = nlocal - 1, j = nlocal - 1; k >= 0; k--) {
      if (j == 0 || (i >= 0 && wspace[i] >= relmnts[j]))
        elmnts[k] = wspace[i--];
      else
        elmnts[k] = relmnts[j--];
    }
  }
}

/* The IncOrder function that is called by qsort is defined as follows */
int IncOrder(const void *e1, const void *e2) {
  return (*((int *)e1) - *((int *)e2));
}