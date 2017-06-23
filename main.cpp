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

    if (rank == 0)
    {
        // cout << "Creating data" << endl;
        // createSolutionTuplesFile();
        // cout << "Data created" << endl;
    }
    if (true)
    {

        // tuples_org = createSolutionTuples();
        tuples_org = readSolutionTuplesFile();
        population = createPopulation(INITIAL_SOLUTIONS_AMOUNT);
    }
    srand(time(NULL) + rank);

    while (true)
    {
        if (rank == 0)
        {
            SerializedPopulation sp = serilize(population);
            int sum_of_elems = 0;
            broadcastPopulation(sp);
            counter++;
        }

        if (rank != 0)
        {
            SerializedPopulation sp = recivePopulation(0, &status);
            vector<Solution *> pop = deserialize(sp);
            vector<Solution *> newSolutions = createNewSolutions(pop);
            SerializedPopulation newSolSerialized = serilize(newSolutions);
            sendNewSolutionsToMaster(newSolSerialized);
        }
        if (rank == 0)
        {
            population.clear();
            for (int i = 1; i < size; i++)
            {
                SerializedPopulation sp = recivePopulation(i, &status);
                vector<Solution *> pop = deserialize(sp);
                population.insert(population.end(), pop.begin(), pop.end());
            }
            population = naturalSelection(population);
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
