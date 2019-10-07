// A simple Hello World program.
//
// Each MPI process prints their rank out of the total world size.
// Ex.
/// > mpirun -np 8 role_call.out
/// > I'm process 1 out of 8
/// > I'm process 2 out of 8
/// > I'm process 3 out of 8
/// > I'm process 4 out of 8
/// > I'm process 5 out of 8
/// > I'm process 6 out of 8
/// > I'm process 7 out of 8
/// > I'm process 8 out of 8
#include <iostream>     // std::cout, std::endl
#include <thread>       // std::this_thread::sleep_for
#include <chrono>       // std::chrono::seconds
#include "mpi.h"        // MPI_Init, MPI_Comm_size, MPI_Comm_rank, MPI_Finalize

using namespace std;

int main(int argc, char *argv[]){
    int myrank, world_size;

    // Initialize the MPI context.
    MPI_Init(&argc, &argv);
    // Get the size of the world.
    MPI_Comm_size( MPI_COMM_WORLD, &world_size);
    // Get the rank of the MPI process.
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Wait a short time depending on the process's rank.
    std::this_thread::sleep_for(std::chrono::milliseconds(100*myrank));

    cout << "I'm process " << myrank + 1 << " out of " << world_size << endl;

    // Clean up necessary for MPI.
    MPI_Finalize();

    return 0;
}