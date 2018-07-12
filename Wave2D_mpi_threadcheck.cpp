// Wave 2D : Parallelized version of the two dimensional 
// wave diffusion program calculated from Schroedinger's Wave
// Dissemination formula, using hybrid form of MPI and
// OpenMPI
// 


#include "mpi.h"      // for using MPI
#include <omp.h>      // for using OpenMP
#include <iostream>
#include "Timer.h"
#include <stdlib.h>   // atoi
#include <iostream>
#include <math.h>     // pow

using namespace std;

int default_size = 100;    // the default system size
int defaultCellWidth = 8;  // default cell width
double c = 1.0;            // wave speed
double dt = 0.1;           // time quantum
double dd = 2.0;           // change in system

// calculating (dt/dd)^2 for the wave formula
double dtByddSquare = pow(dt / dd, 2.0);


int main(int argc, char *argv[]) {
    // verify arguments
    if (argc != 5) {
        cerr << "usage: Wave2D size max_time interval threads" << endl;
        return -1;
    }

    int size = atoi(argv[1]);                 // 100
    int max_time = atoi(argv[2]);             // 500
    int interval = atoi(argv[3]);             // 10
    int nthreads = atoi(argv[4]);             // number of threads - CPU cores 

    int my_rank = 0;        // rank of process used by MPI
    int mpi_size = 1;       // # processes used by MPI
    int tag = 0;            // tag for messages used by MPI

    if (size < 100 || max_time < 3 || interval < 0) {
        cerr << "usage: Wave2D size max_time interval" << endl;
        cerr << "       where size >= 100 && time >= 3 && interval >= 0" <<endl;
        return -1;
    }

    MPI_Init(&argc, &argv);    // start MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);  // find out process rank
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size); // find out # processes

    // create a simulation space
    double z[3][size][size];
    for (int p = 0; p < 3; p++) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                z[p][i][j] = 0.0; // no wave
            }
        }
    }

    // start a timer
    Timer time;
    time.start();

    // broadcast the matrix size to all.  
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // time = 0;
    // initialize the simulation space: calculate z[0][][]
    int weight = size / default_size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i > 40 * weight && i < 60 * weight  &&
                j > 40 * weight && j < 60 * weight) {
                z[0][i][j] = 20.0;
            }
            else {
                z[0][i][j] = 0.0;
            }
        }
    }

    // time = 1
    // calculate z[1][][] 
    if (max_time > 1) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                // cell is on an edge
                if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
                    z[1][i][j] = 0.0;
                    continue;
                }
                // cells not on edge
                z[1][i][j] = z[0][i][j] + ((c * c) / 2.0 * dtByddSquare
                    * (z[0][i + 1][j] + z[0][i - 1][j]
                        + z[0][i][j + 1] + z[0][i][j - 1]
                        - 4.0 * z[0][i][j]));
            }
        }
    }

    omp_set_num_threads(nthreads);    // setting number of CPU cores   

    int stripe = size / mpi_size;     // partitioned stripe

    // simulate wave diffusion from time = 2
    int curr = 2;                // current
    int prev = 1;                // previous 
    int prevPrev = 0;            // previous of previous

    if (max_time > 2) {
        for (int t = 2; t < max_time; t++) {
            // prints the time of each interval
            if ((interval != 0) && (t % interval == 0 || t == max_time - 1)) {
                // recieves z[] from the slaves and prints them
                if (my_rank == 0) {
                    for (int rank = 1; rank < mpi_size; rank++) {
                        MPI_Status status;
                        MPI_Recv(&z[curr][0][0] + rank * stripe * size, 
                            stripe * size, MPI_DOUBLE, rank, tag,
                            MPI_COMM_WORLD, &status);
                    }
                    cout << t << endl;
                    for (int i = 0; i < size; i++) {
                        for (int j = 0; j < size; j++) {
                            cout << z[curr][j][i] << " ";
                        }
                        cout << endl;
                    }
                    cout << endl;
                }
                else {
                    // sending z[] to master
                    MPI_Send(&z[curr][0][0] + my_rank * stripe * size, 
                        stripe * size, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);
                }
            }

            #pragma omp parallel
            {
                // printing the stripe allocation details for each rank
                if (omp_get_thread_num() == 0)
                {
                    if (t == 2) {
                        cerr << "rank[" << my_rank << "]'s range = " << 
                            stripe * my_rank << " ~ " << (((my_rank + 1) * stripe) - 1) 
                                << endl;
                    }
                    else
                    {
                        // Exchange boundry data between the ranks
                        // Even ranks
                        if (my_rank % 2 == 0 && mpi_size > 1) {
                            // right neighbor
                            MPI_Send(z[prev][((my_rank + 1) * stripe) - 1], size, 
                                MPI_DOUBLE, my_rank + 1, tag, MPI_COMM_WORLD);

                            MPI_Status status;
                            MPI_Recv(z[prev][((my_rank + 1) * stripe)], size, MPI_DOUBLE, 
                                my_rank + 1, tag, MPI_COMM_WORLD, &status);

                            //not for the starting rank = 0. 
                            if (my_rank != mpi_size - 1) {
                                // left neighbor
                                MPI_Status status;
                                MPI_Recv(z[prev][(my_rank * stripe) - 1], size, MPI_DOUBLE,
                                    my_rank - 1, tag, MPI_COMM_WORLD, &status);

                                MPI_Send(z[prev][(my_rank * stripe)], size, MPI_DOUBLE,
                                    my_rank - 1, tag, MPI_COMM_WORLD);
                            }
                        }
                        // Odd ranks
                        else if (my_rank % 2 == 1) {
                            // left neighbor
                            MPI_Status status;
                            MPI_Recv(z[prev][(my_rank * stripe) - 1], size, MPI_DOUBLE,
                                my_rank - 1, tag, MPI_COMM_WORLD, &status);

                            MPI_Send(z[prev][(my_rank * stripe)], size, MPI_DOUBLE, 
                                my_rank - 1, tag, MPI_COMM_WORLD);

                            // not for the last rank = mpi_size - 1
                            if (my_rank + 1 != mpi_size) {
                                // right neighbor
                                MPI_Send(z[prev][((my_rank + 1) * stripe) - 1], size,
                                    MPI_DOUBLE, my_rank + 1, tag, MPI_COMM_WORLD);

                                MPI_Status status;
                                MPI_Recv(z[prev][((my_rank + 1) * stripe)], size,
                                    MPI_DOUBLE, my_rank + 1, tag, MPI_COMM_WORLD, &status);
                            }
                        }
                    }
                }

                // all ranks should compute wave simulation from t = 2
                #pragma omp for //firstprivate(c, dtByddSquare) //shared (z)     // parallelized 
                for (int i = my_rank * stripe; i < (my_rank + 1) * stripe; i++) {
                    #pragma omp  for  //firstprivate(c, dtByddSquare, size, curr, prev, prevPrev ) //shared (z)     // parallelized
                    for (int j = 0; j < size; j++) {
                        // cell is on an edge
                        if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
                            z[curr][i][j] = 0.0;
                            continue;
                        }
                        // cells not on edge
                        z[curr][i][j] = (2.0 * z[prev][i][j] - z[prevPrev][i][j])
                            + (c * c) * dtByddSquare
                            * (z[prev][i + 1][j] + z[prev][i - 1][j]
                                + z[prev][i][j + 1] + z[prev][i][j - 1]
                                - 4.0 * z[prev][i][j]);
                    }
                }
            }

            // rotation - shifting down the values
            int temp = prevPrev;
            prevPrev = prev;
            prev = curr;
            curr = temp;
        }
    }

    // finish the timer
    if (my_rank == 0) {
        cerr << "Elapsed time = " << time.lap() << endl;
    }

    MPI_Finalize(); // shut down MPI

    return 0;
}



