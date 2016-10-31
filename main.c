#include <stdio.h>
#include <stdlib.h>
#include <openmpi/mpi.h>

#ifdef HAS_IMB_SETTINGS_H
    #include <imbsettings.h>
#else
    #define MSGSPERSAMPLE 1000
    #define OVERALL_VOL 4*1048576
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

    double min_time_global;
    double max_time_global;
    double time_global;

    double time = MPI_Wtime();
    double min_time = -1;
    double max_time = -1;

    if(world_size > 1){
        int nbytes = 0;

        if(world_rank == 0){
            printf("#----------------------------------------------\n");
            printf("# BENCHMARK: SendRecv\n");
            printf("# #processes %d \n",world_size);
            printf("#----------------------------------------------\n");
            printf("\t#bytes\t#repetitions\ttmin[usec]\t\ttmax[usec]\t\ttavg[usec]\t\tMBytes/sec\n");

        }

        while(nbytes <= OVERALL_VOL){

            int n_sample = nbytes == 0? MSGSPERSAMPLE : max(1,min(MSGSPERSAMPLE,OVERALL_VOL/nbytes*10));
            int count = 0;
            int i = 0;

            for (i=0; i<N_WARMUP; i++ ){

                void* sendBuffer = malloc(sizeof(MPI_BYTE) * nbytes);
                void* recvBuffer = malloc(sizeof(MPI_BYTE) * nbytes);

                /*This loop has no sense*/
                /*int j;
                for(j=0;j < N_BARR;j++)*/
                MPI_Barrier(MPI_COMM_WORLD);

                int nodoSuperior = (world_rank+1)%world_size;
                int nodoInferior = (world_rank+world_size-1)%world_size;

                time = MPI_Wtime();

                for (count=0;count < n_sample;count++) {
                    MPI_Sendrecv(sendBuffer, nbytes, MPI_BYTE, nodoSuperior, 0,
                                 recvBuffer, nbytes, MPI_BYTE, nodoInferior, 0,
                                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

//                    Input Parameters
//                    ================
//                    sendbuf - initial address of send sendBuffer (choice)
//                    sendcount - number of elements in send sendBuffer (integer)
//                    sendtype - type of elements in send sendBuffer (handle)
//                    dest - rank of destination (integer)
//                    sendtag - send tag (integer)
//                    recvBuffer -
//                    recvcount - number of elements in receive sendBuffer (integer)
//                    recvtype -  type of elements in receive sendBuffer (handle)
//                    source - rank of source (integer)
//                    recvtag - receive tag (integer)
//                    comm - communicator (handle)
//                    status -
//
                }

                time = (MPI_Wtime()-time)/n_sample;
                /*This loop has no sense*/
                //for(j=0;j < N_BARR;j++)
//                MPI_Barrier(MPI_COMM_WORLD);

                //obtención de los tiempos locales


                free(sendBuffer);
                free(recvBuffer);

                //sumamos el tiempo de todas las mediciones locales en una medición global y lo dividimos entre el número de muestras
                MPI_Reduce(&time, &time_global, 1, MPI_DOUBLE, MPI_SUM, 0,
                           MPI_COMM_WORLD);

                MPI_Reduce(&time, &min_time_global, 1, MPI_DOUBLE, MPI_MIN, 0,
                           MPI_COMM_WORLD);

                MPI_Reduce(&time, &max_time_global, 1, MPI_DOUBLE, MPI_MAX, 0,
                           MPI_COMM_WORLD);

            }

            if(world_rank == 0){
                time_global/=world_size;

                //time = (MPI_Wtime()-time)/n_sample;
                double bandwith=nbytes/time_global/1024/1024;
                printf("\t%d\t%d\t\t%.10f\t\t%.10f\t\t%.10f\t\t%.10f\n",nbytes,n_sample,min_time_global*1000000,max_time_global*1000000,time_global*1000000,bandwith);
            }

            nbytes = nbytes == 0 ? 1 : nbytes * 2 ;

        }
    }else{
        printf("Non existen nodos suficienetes\n");
    }
    // Finalize the MPI environment.
    MPI_Finalize();
}
