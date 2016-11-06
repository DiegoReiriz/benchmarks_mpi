#include <stdio.h>
#include <stdlib.h>
#include <openmpi/mpi.h>

#ifdef HAS_IMB_SETTINGS_H
    #include <imbsettings.h>
#else
    #define MSGSPERSAMPLE 1000
    #define OVERALL_VOL 4*1048576
    #define N_BARR 2
    #define N_WARMUP 2
#endif

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef unsigned char byte;

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    double time;

    int count = 0;

    if(world_rank == 0)
        printf("\t#bytes\t#repetitions\tt[usec]\t\tMBytes/sec\n");


    time = MPI_Wtime();


    for (count = 0;count < 100000;count++) {
        MPI_Barrier(MPI_COMM_WORLD);

    }

    time = (MPI_Wtime()-time)/count;



    if(world_rank == 0){
        printf("%.20f\n",time*1000000);
    }


    MPI_Finalize();
}
