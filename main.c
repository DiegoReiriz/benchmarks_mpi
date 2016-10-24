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

    if(world_size > 1){
        int nbytes = 0;

        if(world_rank == 0)
            printf("\t#bytes\t#repetitions\tt[usec]\t\tMBytes/sec\n");

        while(nbytes <= OVERALL_VOL){

            int n_sample = nbytes == 0? MSGSPERSAMPLE : max(1,min(MSGSPERSAMPLE,OVERALL_VOL/nbytes*10));

            int count = 0;
            int i = 0;

            //o n barrier debería aplicarse solo sobrea primeira barrera e as iteracions que se fan para quentar son as de WARMUP
            for (i=0; i<N_WARMUP; i++ ){
                count =0;
                byte buffer[nbytes];

                /*This loop has no sense*/
                /*int j;
                for(j=0;j < N_BARR;j++)*/
                    MPI_Barrier(MPI_COMM_WORLD);

                time = MPI_Wtime();


                while (count < n_sample) {
                    if (world_rank == 0) {
    //                    printf("ARRANCANDO %d\n",world_rank);
                        MPI_Send(&buffer, nbytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                        MPI_Recv(&buffer, nbytes, MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    } else if(world_rank == 1) {
    //                    printf("ARRANCANDO %d\n",world_rank);
                        MPI_Recv(&buffer, nbytes, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        MPI_Send(&buffer, nbytes, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
                    }
                    count++;
                }


                /*This loop has no sense*/
                //for(j=0;j < N_BARR;j++)
                    MPI_Barrier(MPI_COMM_WORLD);

                if(world_rank == 0)
                    time = (MPI_Wtime()-time)/n_sample/2;// entre 2 porque ping pong é especial

            }

            if(world_rank == 0){
                //time = (MPI_Wtime()-time)/n_sample;
                double bandwith=nbytes/time/1024/1024;
                printf("\t%d\t%d\t%.20f\t\t%.20f\n",nbytes,n_sample,time*1000000,bandwith);
            }

            nbytes = nbytes == 0 ? 1 : nbytes * 2 ;

        }
    }else{
        printf("Non existen nodos suficienetes\n");
    }
    // Finalize the MPI environment.
    MPI_Finalize();
}
