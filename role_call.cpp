#include <iostream>     // std::cout, std::endl
#include <thread>       // std::this_thread::sleep_for
#include <chrono>       // std::chrono::seconds
#include "mpi.h"        // MPI_Init, MPI_Comm_size, MPI_Comm_rank, MPI_Finalize

using namespace std;

int main(int argc, char *argv[]){
    int myrank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    std::this_thread::sleep_for(std::chrono::milliseconds(100*myrank));

    cout << "I'm process " << myrank + 1 << " out of " << world_size << endl;

    MPI_Finalize();

    return 0;
}