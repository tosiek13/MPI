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

int ROOT_THREAD = 0;

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //INITIALIZATION
    srand(time(NULL) + rank);
    PopulationBuffer popBuffer = allocatePopulationBuffer(POPULATION_SIZE);

    if (rank == ROOT_THREAD)
    {
        // createSolutionTuplesFile();
        // tuples_org = createSolutionTuples();
        vector<Tuple *> tuples_org = readSolutionTuplesFile();
        population = createPopulation(tuples_org, POPULATION_SIZE);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    bool exe_end = false;

    // while (!exe_end)
    // {

    //update buffer
    writeToBuffer(population, &popBuffer);
    // BROADCAST buffer
    int pop_periods_amount = POPULATION_SIZE * PERIODS_AMOUNT;
    int pop_tuples_size = POPULATION_SIZE * TUPLES_AMOUNT;
    MPI_Bcast(&(popBuffer.pop_size), 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.tuplesNumInPeriods, pop_periods_amount, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.tuplesIds, pop_tuples_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.groupIds, pop_tuples_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.lecturerIds, pop_tuples_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.roomIds, pop_tuples_size, MPI_INT, 0, MPI_COMM_WORLD);

    //DO COMPUTATION
    // if (rank != ROOT_THREAD)
    if (rank == 2)
    {
        vector<Solution *> pop = deserialize(popBuffer);
        // printPopulationInfo(pop);
        vector<Solution *> newSolutions = createNewSolutions(pop);
        PopulationBuffer newSolution_ser = serilize(newSolutions);
        // cout << "Send to root: " << rank << endl;
        // sendNewSolutionsToMaster(newSolSerialized);
        // cout << "Send done: " << rank << endl;
    }
    // if (rank == 0)
    // {
    //     population.clear();
    //     for (int i = 1; i < size; i++)
    //     {
    //         SerializedPopulation sp = recivePopulation(i, &status);
    //         cout << "Recive solution done: " << i << endl;
    //         vector<Solution *> pop = deserialize(sp);
    //         population.insert(population.end(), pop.begin(), pop.end());
    //     }
    //     population = naturalSelection(population);
    //     cout << endl
    //          << countSolutionCost(population[0]) << endl;

    //     for (int j = 0; j < population.size(); j++)
    //     {
    //         if (countSolutionCost(population[j]) == 0)
    //         {
    //             printCSVSolution(population[j]);
    //             MPI_Abort(MPI_COMM_WORLD, 0);
    //         }
    //     }
    // }


    // }

    delete[] popBuffer.tuplesNumInPeriods;
    delete[] popBuffer.tuplesIds;
    delete[] popBuffer.groupIds;
    delete[] popBuffer.lecturerIds;
    delete[] popBuffer.roomIds;

    MPI_Finalize();
    return 0;
}
