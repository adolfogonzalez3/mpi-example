// Example program for showing the effects of redirecting stdin.
// By default, mpirun gives process 0 access to stdin and the other
// processes receive dev/null. This means that trying to receive
// input from stdin from any other process aside from process 0
// will result in strange behaviour.
// In order to change which process receives stdin you must use
// the command
// > mpirun -np <total number of processes> -stdin <process number> <program>
// Where total number of processes is greater than process number.
// An example is running homework 1 which you would run the minimum_prime_and_twin_finder
// program by running
// > mpirun -np 5 -stdin 4 minimum_prime_and_twin_finder
// This will run your program correctly.

#include<iostream>
#include<string>
#include<math.h>
#include<limits.h>
#include <stdlib.h>
#include <mpi.h>

////////////////////////////////////////////////////////////////////////
//https://stackoverflow.com/questions/194465/how-to-parse-a-string-to-an-int-in-c
enum STR2INT_ERROR { SUCCESS, OVERFLOW, UNDERFLOW, INCONVERTIBLE };

STR2INT_ERROR str2int (int &i, char const *s, int base = 0)
{
    char *end;
    long  l;
    errno = 0;
    l = strtol(s, &end, base);
    if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
        return OVERFLOW;
    }
    if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
        return UNDERFLOW;
    }
    if (*s == '\0' || *end != '\0') {
        return INCONVERTIBLE;
    }
    i = l;
    return SUCCESS;
}
////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    int myrank;
    int world_size;
    int n;
    int q[2] = {100, 100};
    int p[2];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    if(argc <= 1){
        if(myrank == 0) {
            std::cout << "Not receiving the root argument.";
            std::cout << "Ex." << std::endl;
            std::cout << "mpirun -np 8 -stdin 7 redirect_input 7" << std::endl;
        }
        return 0;
    }
    int root;
    str2int(root, argv[1]);
    std::string user_input;

    if(myrank == root) {
        std::cout << "Process " << myrank << " is root.";
        std::cout << "If you see weird behavior then ensure you are running mpirun with the -stdin arg" << std::endl;
        std::cout << "Ex." << std::endl;
        std::cout << "mpirun -np 8 -stdin 7 redirect_input 7" << std::endl;
        std::cin >> user_input;
        std::cout << "Process " << myrank << " received " << user_input << std::endl;
    }

    MPI_Finalize();
    return 0;
}