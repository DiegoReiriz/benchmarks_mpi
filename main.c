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

    double time = MPI_Wtime();
    double time_global;
    double min_time_global;
    double max_time_global;


    if(world_size > 1){
        int nbytes = 0;

        if(world_rank == 0)
            printf("\t#bytes\t#repetitions\tt_min[usec]\t\tt_max[usec]\t\tt[usec]\t\tMBytes/sec\n");

        while(nbytes <= OVERALL_VOL){

            int n_sample = nbytes == 0? MSGSPERSAMPLE : max(1,min(MSGSPERSAMPLE,OVERALL_VOL/nbytes*10));

            int count = 0;
            int i = 0;

            //o n barrier debería aplicarse solo sobrea primeira barrera e as iteracions que se fan para quentar son as de WARMUP
            for (i=0; i<N_WARMUP; i++ ){
                count =0;
                void* sendBuffer = malloc(sizeof(MPI_BYTE)*nbytes*world_size);

                void* recvBuffer = malloc(sizeof(MPI_BYTE)*nbytes);

                MPI_Barrier(MPI_COMM_WORLD);

                time = MPI_Wtime();

                int k = 0;
                for (count = 0;count < n_sample;count++)
                    for(k = 0; k<world_size;k++)
                        MPI_Scatter(sendBuffer,nbytes,MPI_BYTE,
                                    recvBuffer,nbytes,MPI_BYTE,i,MPI_COMM_WORLD);

                MPI_Barrier(MPI_COMM_WORLD);

                time = (MPI_Wtime()-time)/world_size/n_sample;

//                MPI_Reduce(&time, &time_global, 1, MPI_DOUBLE, MPI_SUM, 0,
//                           MPI_COMM_WORLD);

                MPI_Reduce(&time, &min_time_global, 1, MPI_DOUBLE, MPI_MIN, 0,
                           MPI_COMM_WORLD);

                MPI_Reduce(&time, &max_time_global, 1, MPI_DOUBLE, MPI_MAX, 0,
                           MPI_COMM_WORLD);

                free(sendBuffer);
                free(recvBuffer);
            }

            if(world_rank == 0){
                //time = (MPI_Wtime()-time)/n_sample;
                double bandwith=nbytes/time/1024/1024/world_size;
                printf("\t%d\t%d\t%.20f\t\t%.20f\t\t%.20f\t\t%.20f\n",nbytes,n_sample,min_time_global*1000000,max_time_global*1000000,time_global*1000000,bandwith);
            }

            nbytes = nbytes == 0 ? 1 : nbytes * 2 ;

        }
    }else{
        printf("Non existen nodos suficienetes\n");
    }
    // Finalize the MPI environment.
    MPI_Finalize();
}
