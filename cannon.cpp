
#include<iostream>
#include <mpi.h>
#include<math.h>
#include <stdlib.h>

using namespace std;

void MatrixMultiply(int n, double *a, double *b, double *c);
void MatrixMatrixMultiply(int n, double *a, double *b, double *c, MPI_Comm comm);

int main(int argc, char *argv[]) {
  const unsigned int width = 2;
  const unsigned int height = width;
  const unsigned int num_elements = width * height;

  double *a = new double[num_elements];
  double *b = new double[num_elements];
  double *c = new double[num_elements];

  for(unsigned int i = 0; i < num_elements; i++) {
    a[i] = 1;
    b[i] = 1;
    c[0] = 0;
  }

  MPI_Init(&argc, &argv);

  MatrixMatrixMultiply(width, a, b, c, MPI_COMM_WORLD);

  delete[] a;
  delete[] b;
  delete[] c;
  MPI_Finalize();
  return 0;
}

void MatrixMatrixMultiply(int n, double *a, double *b, double *c, MPI_Comm comm) {
  int i;
  int nlocal;
  int npes, dims[2], periods[2];
  int myrank, my2drank, mycoords[2];
  int uprank, downrank, leftrank, rightrank, coords[2];

  int shiftsource, shiftdest;
  MPI_Status status;
  MPI_Comm comm_2d;
  /* Get the communicator related information */
  MPI_Comm_size(comm, &npes);
  MPI_Comm_rank(comm, &myrank);
  /* Set up the Cartesian topology */
  dims[0] = dims[1] = sqrt(npes);
  /* Set the periods for wraparound connections */
  periods[0] = periods[1] = 1;
  /* Create the Cartesian topology, with rank reordering */
  MPI_Cart_create(comm, 2, dims, periods, 1, &comm_2d);
  /* Get the rank and coordinates with respect to the new topology */
  MPI_Comm_rank(comm_2d, &my2drank);
  MPI_Cart_coords(comm_2d, my2drank, 2, mycoords);
  /* Compute ranks of the up and left shifts */
  MPI_Cart_shift(comm_2d, 0, -1, &rightrank, &leftrank);
  MPI_Cart_shift(comm_2d, 1, -1, &downrank, &uprank);
  /* Determine the dimension of the local matrix block */
  nlocal = n / dims[0];
  /* Perform the initial matrix alignment. First for A and then for B */
  MPI_Cart_shift(comm_2d, 0, -mycoords[0], &shiftsource, &shiftdest);
  MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
                       shiftsource, 1, comm_2d, &status);
  MPI_Cart_shift(comm_2d, 1, -mycoords[1], &shiftsource, &shiftdest);
  MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
                       shiftsource, 1, comm_2d, &status);

  cout << "here " << nlocal << endl;

  /* Get into the main computation loop */
  for (i = 0; i < dims[0]; i++) {
    MatrixMultiply(nlocal, a, b, c); /*c=c+a*b*/
    /* Shift matrix a left by one */
    MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_DOUBLE, leftrank, 1, rightrank,
                         1, comm_2d, &status);
    /* Shift matrix b up by one */
    MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_DOUBLE, uprank, 1, downrank, 1,
                         comm_2d, &status);
    cout << i << endl;
  }

  /* Restore the original distribution of a and b */
  MPI_Cart_shift(comm_2d, 0, +mycoords[0], &shiftsource, &shiftdest);
  MPI_Sendrecv_replace(a, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
                       shiftsource, 1, comm_2d, &status);

  MPI_Cart_shift(comm_2d, 1, +mycoords[1], &shiftsource, &shiftdest);
  MPI_Sendrecv_replace(b, nlocal * nlocal, MPI_DOUBLE, shiftdest, 1,
                       shiftsource, 1, comm_2d, &status);
  MPI_Comm_free(&comm_2d); /* Free up communicator */
}

/* This function performs a serial matrix-matrix multiplication c = a*b */
void MatrixMultiply(int n, double *a, double *b, double *c) {
  int i, j, k;
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      for (k = 0; k < n; k++)
        c[i * n + j] += a[i * n + k] * b[k * n + j];
}