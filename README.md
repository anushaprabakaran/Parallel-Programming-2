REPORT - PROGRAMMING ASSIGNMENT 2

DOCUMENTATION
Wave2D.cpp
The sequential version was done first. Implemented the time > 1 and time > 2 conditions using the Schroedinger’s wave diffusion formula along with the edge cell case.

Rotation: In order to prevent array copying and speed up the execution, we follow shifting down the values. I use curr (current), prev (previous), prevPrev (previous of previous). For instance, when t = 3, 
prevPrev = 1    prev = 2    curr = 3. After this, I set the curr to prevPrev and the prev to curr and prevPrev to prev using a temp as only last two are needed for calculation. So when t = 4, curr which is now pointing to prevprev will automatically be 4. 

Printing the simulation space: It has to be printed before calculation of wave for t >= 2. I used Cygwin for running the graphical output with java. For the third parameter, interval, if it is 0, no simulation output is printed.

Calculating (dt/dd)^2 for the wave formula: I am calculating this portion of the formula, using pow for squaring outside the main. 

Wave2D_mpi.cpp
The sequential version was first parallelized using MPI and then with OpenMP especially for the formula.

Stripes/Slicing: The matrix is sliced by the rows i.e the i-axis. The columns are all sent completely. The entire size is broadcasted to all ranks using MPI_Bcast() but they will work on only the sliced stripes. 

Boundary Exchange using MPI_Send() and MPI_Recv():  As debugging using MPI is very difficult, I initially hard coded the program for matrix size 100. I checked whether it is working correctly with that boundary exchange. When it was all working without any deadlocks, I generalized the code. I separated them as even and odd ranks (using modulo of 2).  The generalization includes the exact [i] number and the corresponding rank number it has to be sent and received. So the even will include rank 0, which does not have a left boundary to exchange data and the odd will have the last rank = mpi_size -1, which does not have a right boundary for exchanging data. The rest all ranks will have both the boundaries. All the calculation is consolidated and only the results are shared through MPI, which helps in reducing the computation network latency.

Thread 0 for exchange and other threads for calculation: I tried implementing this using the omp_get_thread_num(). If it is equal to 0, it has to do all the MPI exchanges. When I checked the out4.txt, the output was not matching with out1.txt. The ordering of the operations made it very complex to implement the correct way. If I was able to do the calculations before the exchanges, it would have worked correctly. The rotations using curr and prev and prevPrev was not aligned properly when I tried implementing. I have included the Wave2D_mpi_threadcheck.cpp file just to show that I tried implementing. But it doesn’t work.  

Multithreading using OpenMP:  #pragma omp parallel for with the schedule (static), firstprivate, shared and default is used above the inner and outer for loops of the formula for calculating the wave simulation above time 
t > 2. The inner j loop (columns) are passed completely to all the nodes. It is a main part to be parallelized. I also tried applying omp on the for loop for time t > 1 calculation. But there was no performance improvement. 

Printing and Collecting at Rank = 0: The rank 0 is responsible for printing the simulation at the specified intervals. So all the ranks 1 to 3, send the strips to rank 0. Not only the printing, but the collection from the other ranks itself is done only when the interval is not 0 and if time t is equal to the max_time or time t modulo interval is 0. This process of not collecting, reduces the computation time greatly.

Out.txt: I have checked the output correctness with few other names out1_my.txt and out_my_1.txt. So the screen shots have them as I dint want to rewrite the existing file every time. 

Boundary Exchange 
Stripes shown for Size = 100

DISCUSSION
Results: 
The first requirement is to have a performance improvement equal or greater than 2.2x on running the program with four machines. This code achieves a performance improvement of about 2.498 times.                                      The second requirement is to have a have a performance improvement equal or greater than 4.0x on running the program with four machines and with multithreading. This code achieves a performance improvement of about 4.725 times. 

The elapsed time for sequential code is 2220958 seconds.
The elapsed time for MPI with four machines is 888918 seconds.
The elapsed time for MPI with four machines and 4 threads (OpenMP) is 470061 seconds.

Possible Performance Improvement:
I think there will be a better performance, if all the MPI calls is done by one thread which computes the two rows to be exchanged and the computation is done by other threads. One of the professor’s slides, show this overlapping communication and computation. The exchange of messages should not affect the current computation. When I tried to implement this, as the order of operations was strictly mentioned that it has to print, exchange and then calculate, I was unable to get the results. The rotation was not aligning correctly for this. If I do calculation before exchange, then it would have worked correctly with the output. The ordering of the code has made the implementation very complex. When I tried to analyze this, say for a matrix size of 100, for 4 computers its 25 rows each, and with 4 threads its 6 rows each thread. I am not sure as how much performance difference or delta it will make between 6 and 2 rows computation time. 
Using MPI_Isend() with MPI_Wait() and MPI_Irecv() may increase the performance as there is communication and computation overlap. While waiting for the communication to be finished, some computation can be done.
Performance is limited by the overheads of the communication between the nodes. The exchange of data across the boundary for each time t, causes a lot of kernel operations. About one-fourth of the computation time is roughly about 50 – 60 %. So the rest is the communication overhead. It takes lot of time to communicate between the ranks.
With the use of 1 node and 4 threads, the time is 657773 and with 4 nodes and 4 threads, the time is 470001. Increasing the amount of resources has not helped very much in performance improvement. Whereas, with the use of 4 nodes and 1 thread, the time is 888918. The increase in number of threads has helped a lot in performance improvement. It is approximately halved.

Limitation: 
Time t = 1 is calculated every time separately. It may be calculated at once and transfer it to all. I have to code and see whether this transfer will save some time. 
The order of the operations doesn’t easily allow to implement thread 0 for exchange and other threads for calculation and increase the performnce.
One of the main limitation of using MPI is, it is very hard to debug. (I initially hard coded for size 100 and checked. When it was working correctly, I generalized it)
Performance varies depending on network latency, even though servers are dedicated for this. There is lot of communication and it is difficult to say the exact performance gain with no consistency. 
The race for printing the ranks with their chunk ranges, has sometimes resulted in garbled output as you can see in one of my screen shots below. It is not easy to read. 


EXECUTION OUTPUT

PrintOut of the simulation space at every interval step : out1.txt and out2.txt


 
 
