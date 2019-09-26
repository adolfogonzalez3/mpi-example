

#include<iostream>
#include <mpi.h>
#include<math.h>
#include <stdlib.h>

using namespace std;

bool is_prime(int);
int prime(int, int);
int twin(int, int);

int main(int argc, char *argv[]) {
  const unsigned int root = 0;
  int myrank;
  int world_size;
  int n;
  int q[2] = {100, 100};
  int p[2];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  if(myrank == root) {
    cout << "Give number: ";
    cin >> n;
  }
  MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);
  q[0] = prime(n, myrank);
  q[1] = twin(n, myrank);
  MPI_Reduce(&q, &p, 2, MPI_INT, MPI_MIN, root, MPI_COMM_WORLD);

  if(myrank == root) {
    cout << "ROOT: " << p[0] << " " << p[1] << endl;
  }

  MPI_Finalize();
  return 0;
}


bool is_prime(int number) {
  for (int i = 2; i <= sqrt(number); i++) {
    if (number % i == 0)
      return false;
  }
  return true;
}

int prime(int number, int rank) {
  int m = 0;
  while (true) {
    int q = 8*m + (2*rank + 1);
    if (q > number && is_prime(q))
      return q;
    m++;
  }
}

int twin(int number, int rank) {
  int m = 0;
  while (true) {
    int q = 8*m + (2*rank + 1);
    if (q > number && is_prime(q) && is_prime(q+2))
      return q;
    m++;
  }
}
