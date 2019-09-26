#include <mpi.h>
#include <stdlib.h>

void CompareSplit(int nlocal, int *elmnts, int *relmnts, int *wspace,
                  int keepsmall);

int IncOrder(const void *e1, const void *e2);

int main(int argc, char *argv[]) {

  int n;
  int npes;
  int myrank;
  int nlocal;
  int *elmnts;
  int *relmnts;
  int oddrank;
  int evenrank;
  int *wspace;
  int i;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &npes);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  n = atoi(argv[1]);
  nlocal = n / npes;

  elmnts = new int[nlocal];
  relmnts = new int[nlocal];
  wspace = new int[nlocal];

  srandom(myrank);
  for (i = 0; i < nlocal; i++)
    elmnts[i] = random();

  qsort(elmnts, nlocal, sizeof(int), IncOrder);

  if (myrank % 2 == 0) {
    oddrank = myrank - 1;
    evenrank = myrank + 1;
  } else {
    oddrank = myrank + 1;
    evenrank = myrank - 1;
  }

  /* Set the ranks of the processors at the end of the linear */
  if (oddrank == -1 || oddrank == npes)
    oddrank = MPI_PROC_NULL;
  if (evenrank == -1 || evenrank == npes)
    evenrank = MPI_PROC_NULL;

  /* Get into the main loop of the odd-even sorting algorithm */
  for (i = 0; i < npes - 1; i++) {
    if (i % 2 == 1) /* Odd phase */
      MPI_Sendrecv(elmnts, nlocal, MPI_INT, oddrank, 1, relmnts, nlocal,
                   MPI_INT, oddrank, 1, MPI_COMM_WORLD, &status);
    else /* Even phase */
      MPI_Sendrecv(elmnts, nlocal, MPI_INT, evenrank, 1, relmnts, nlocal,
                   MPI_INT, evenrank, 1, MPI_COMM_WORLD, &status);
    CompareSplit(nlocal, elmnts, relmnts, wspace, myrank < status.MPI_SOURCE);
  }

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