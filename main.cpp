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

    vector<Solution *> population;
    if (rank == ROOT_THREAD)
    {
        // createSolutionTuplesFile();
        // tuples_org = createSolutionTuples();
        vector<Tuple *> tuples_org = readSolutionTuplesFile();
        population = createPopulation(tuples_org, POPULATION_SIZE);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    bool exe_end = false;

    int iterNr = 0;
    while (!exe_end)
    {
        cout << "Rank: " << rank << endl;
        //update buffer with new population
        writeToBuffer(population, &popBuffer);
        // BROADCAST buffer
        int pop_periods_amount = POPULATION_SIZE * PERIODS_AMOUNT;
        int pop_tuples_size = POPULATION_SIZE * TUPLES_AMOUNT;

        broadcastPopulationBuffer(popBuffer, ROOT_THREAD);

        //DO COMPUTATION
        if (rank != ROOT_THREAD)
        {
            vector<Solution *> pop = deserialize(popBuffer);
            // printPopulationInfo(pop);
            //ACTION
            vector<Solution *> newSolutions = createNewSolutions(pop);
            //END ACTION
            PopulationBuffer new_sol = serilize(newSolutions);
            sendPopulationBuffer(new_sol, ROOT_THREAD);
            freePopulationBuffer(new_sol);
        }
        if (rank == ROOT_THREAD)
        {
            population.clear();
            for (int i = 0; i < size; i++)
            {
                if (i != ROOT_THREAD)
                {
                    PopulationBuffer recived_sol_buff = receivePopulationBuffer(i);

                    vector<Solution *> rec_sol = deserialize(recived_sol_buff);
                    freePopulationBuffer(recived_sol_buff);
                    population.insert(population.end(), rec_sol.begin(), rec_sol.end());
                }
            }

            population = naturalSelection(population);

            //print solution
            cout << "Iter nr" << iterNr << endl;
            iterNr++;
            for (int j = 0; j < population.size(); j++)
            {
                if (countSolutionCost(population[j]) == 0 || (iterNr > MAX_ITER_NR))
                {
                    printCSVSolution(population[j]);
                    exe_end = true;
                }
            }
            cout << "Generation end" << endl;
        }
         MPI_Barrier(MPI_COMM_WORLD);
    }
    freePopulationBuffer(popBuffer);

    MPI_Finalize();
    return 0;
}