#include <iostream>
#include <random>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <climits>
#include <fstream>
#include <mpi.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/set.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include "./genetic_logic.hpp"
#include "./parallel_logic.hpp"
#include <time.h> /* time */

using namespace std;

int main(int argc, char **argv)
{
    //test

    int rank, size, i;
    int buffer[10];
    MPI_Status status;
    int counter;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // if (rank == 0)
    if (true)
    {
        // createSolutionTuplesFile();
        // tuples_org = createSolutionTuples();
        tuples_org = readSolutionTuplesFile();
        // cout << "READ done" << endl;
        population = createPopulation(INITIAL_SOLUTIONS_AMOUNT);
        // cout << "population created done" << endl;
    }
    // cout << "main execution" << endl;
    srand(time(NULL) + rank);

    while (true)
    {
        if (rank == 0)
        {
            // cout << "rank 0 serizlize" << endl;
            SerializedPopulation sp = serilize(population);
            int sum_of_elems = 0;
            // cout << "rank 0 broadcast" << endl;
            broadcastPopulation(sp);
            //printPopulation(population);
            // cout << "rank 0 broadcast end" << endl;
            counter++;
        }

        if (rank != 0)
        {
            // cout << "rank: " << rank << " receive" << endl;
            SerializedPopulation sp = recivePopulation(0, &status);
            // cout << "rank: " << rank << " got" << endl;
            vector<Solution *> pop = deserialize(sp);
            vector<Solution *> newSolutions = createNewSolutions(pop);
            SerializedPopulation newSolSerialized = serilize(newSolutions);
            //printPopulation(newSolutions);
            sendNewSolutionsToMaster(newSolSerialized);
        }
        if (rank == 0)
        {
            population.clear();
            for (int i = 1; i < size; i++)
            {
                // cout << "rank: " << rank << " reveive solution from rank: " << i << endl;
                SerializedPopulation sp = recivePopulation(i, &status);
                // cout << "rank: " << rank << " reveive SOLUTIONS END" << endl;
                vector<Solution *> pop = deserialize(sp);
                //printPopulation(pop);
                population.insert(population.end(), pop.begin(), pop.end());
            }
            // cout << "rank: " << rank << " natural selection" << endl;
            population = naturalSelection(population);
            // cout << "rank: " << rank << " natural selection end with amount: " << population.size() << endl;
            cout << endl
                 << countSolutionCost(population[0]) << endl;

            for (int j = 0; j < population.size(); j++)
            {
                if (countSolutionCost(population[j]) == 0)
                {
                    printCSVSolution(population[j]);
                    MPI_Abort(MPI_COMM_WORLD, 0);
                }
            }
        }
    }

    MPI_Finalize();
    return 0;
}
