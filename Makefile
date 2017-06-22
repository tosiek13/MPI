CC      = g++
CFLAGS  = -lm -std=gnu++11 
LDFLAGS = -lboost_serialization -I/opt/nfs/mpe2-2.4.9b/include -L/opt/nfs/mpe2-2.4.9b/lib -lmpe 

all: mpi run

simple:
	g++ $(CFLAGS) main.cpp

mpi:
	mpicxx -o a.out $(CFLAGS)  main.cpp $(LDFLAGS)

run:
	mpirun -np $(CORES) ./a.out

clean:
	rm *.out
