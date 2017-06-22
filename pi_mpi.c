/* ======================================================================== */
/*   pi.c                                                                   */
/*   MPI program for calculating Pi by numerical integration                */
/*   - full code                                                            */
/* ======================================================================== */
#include "mpi.h"
#include "mpe.h"
#include <stdio.h> 
#include <math.h>
#define START_BCAST 0
#define END_BCAST 1
#define START_REDUCE 2
#define END_REDUCE 3

int main( int argc, char *argv[] ) 
{ 
    int n, myid, numprocs, i; 
    double PI25DT = 3.141592653589793238462643; 
    double mypi, pi, h, sum, x; 
    MPI_Init(&argc,&argv);
	MPE_Init_log();
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs); 
    MPI_Comm_rank(MPI_COMM_WORLD,&myid); 

/***
   * Calculate pi by numerical integration of a arctangent derivative function.
   * Read a number of stripes (rectangles or trapezes) under 
   * the function curve in every loop iteration,
   */
	if (myid == 0) { 
		MPE_Describe_state(START_BCAST, END_BCAST, "broadcast", "red");
		MPE_Describe_state(START_REDUCE, END_REDUCE, "broadcast", "green");
	} 


	int k = 0;
    while (k<1) { 
	k++;
       if (myid == 0) { 
          /*printf("Enter the number of intervals: (0 quits) "); 
          scanf("%d",&n);*/
		n = 4;
       } 

	MPE_Log_event(START_BCAST, 0, "pierwszy broadcast");
       MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); 
	MPE_Log_event(END_BCAST, 0, "pierwszy broadcast");

       if (n == 0) 
          break; 
       else { 
          h   = 1.0 / (double) n; 
          sum = 0.0; 
          for (i = myid + 1; i <= n; i += numprocs) { 
               x = h * ((double)i - 0.5); 
               sum += (4.0 / (1.0 + x*x)); 
          } 
          mypi = h * sum; 

          MPE_Log_event(START_REDUCE, 0, "reduce");
		MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); 
	MPE_Log_event(END_REDUCE, 0, "reduce");


          if (myid == 0)  
          printf("pi is approximately %.16f, Error is %.16f\n", 
                  pi, fabs(pi - PI25DT)); 
       } // end of else

    } // end of while

	MPE_Finish_log("log.txt");
    MPI_Finalize(); 
    return 0; 
} 
