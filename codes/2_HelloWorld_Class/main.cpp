#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <mpi.h>
#include <sched.h>
#include <omp.h>

#include "utilities/include/GpuCommon.h"
#include "utilities/include/Hello.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlength; // dummy
    MPI_Get_processor_name(name, &resultlength);

    Hello hello(size, rank, name);
    hello.hello();   

    MPI_Finalize();

    return 0;

}

