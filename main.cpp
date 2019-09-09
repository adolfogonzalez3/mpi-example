#include<iostream>
#include<map>
#include "mpi.h"

using namespace std;

void check_error(int ierr) {
    if(ierr) {
        exit(1);
    }
}

int scatter_integer(int& recv_int, const int* array = NULL) {
    return MPI_Scatter(
        &array[0], 1, MPI_INT, &recv_int, 1,
        MPI_INT, 0, MPI_COMM_WORLD
    );
}

int scatter_strings(
    char* recv_string, int received_length,
    char** strings = NULL, int* lengths = NULL,
    int* displacements = NULL
    ) {
    void* buffer = NULL;
    if(strings != NULL)
        buffer = &strings[0][0];
    return MPI_Scatterv(
        buffer, lengths, displacements, MPI_BYTE,
        &recv_string[0], received_length, MPI_BYTE,
        0, MPI_COMM_WORLD
    );
}

// Return the length of a string.
//
// :char_array: A character string.
// :return: The length of the string.
int len(const char* char_array) {
    int length = 0;
    while(char_array[length] != '\0') {
        length++;
    }
    return length + 1;
}

// Send strings over MPI
//
// :strings: A sequence of strings each ending with a null character.
// :num_of_str: Number of strings.
// :recvbuffer: A buffer to receive the strings.
// :return: A MPI error
int send_strings(char** strings, int num_of_str, char* recvbuffer) {
    int* lengths = new int[num_of_str];
    int* displacements = new int[num_of_str];
    int received_length = 0;
    int current_displacement = 0;
    for(int i = 0; i < num_of_str; i++) {
        lengths[i] = len(strings[i]);
        displacements[i] = current_displacement;
        current_displacement += lengths[i];
    }
    scatter_integer(received_length, lengths);
    scatter_strings(
        recvbuffer, received_length, strings, lengths, displacements
    );
    delete[] lengths;
    delete[] displacements;
    return 0;
}

int recv_strings(char* recvbuffer) {
    int received_length = 0;
    scatter_integer(received_length);
    scatter_strings(recvbuffer, received_length);
    return 0;
}

int main(int argc, char *argv[]){
    int id;
    int p;
    char received_string[15];
    char* some_string[] = {
        "Hello", "World!", "what?",
        "Hello", "World!", "what?",
        "Hello", "World!", "what?",
        "Hello", "World!", "what?",
        "Hello", "World!", "what?"
    };
    check_error(MPI_Init(NULL, NULL));
    check_error(MPI_Comm_size( MPI_COMM_WORLD, &p ));
    check_error(MPI_Comm_rank(MPI_COMM_WORLD, &id));
    if (id == 0) {
        cout << "I'm number one!" << endl;
        cout << p << endl;
        send_strings(some_string, 15, received_string);
    }else {
        recv_strings(received_string);
    }
    cout << len(received_string) << endl;
    //cout << received_string << endl;
    check_error(MPI_Finalize());

    return 0;
}