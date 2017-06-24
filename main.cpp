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

    int iterNr = 0;
    while (!exe_end)
    {
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
        if (rank != ROOT_THREAD)
        {
            vector<Solution *> pop = deserialize(popBuffer);
            // printPopulationInfo(pop);
            vector<Solution *> newSolutions = createNewSolutions(pop);
            PopulationBuffer new_sol = serilize(newSolutions);

            // send to master
            int pop_size = new_sol.pop_size;
            int periods_size = pop_size * PERIODS_AMOUNT;
            int tuples_size = pop_size * TUPLES_AMOUNT;

            int values[] = {1, 2, 3, 4};

            MPI_Send(&(new_sol.pop_size), 1, MPI_INT, ROOT_THREAD, 0, MPI_COMM_WORLD);
            MPI_Send(new_sol.tuplesNumInPeriods, periods_size, MPI_INT, ROOT_THREAD, 0, MPI_COMM_WORLD);
            MPI_Send(new_sol.tuplesIds, tuples_size, MPI_INT, ROOT_THREAD, 0, MPI_COMM_WORLD);
            MPI_Send(new_sol.groupIds, tuples_size, MPI_INT, ROOT_THREAD, 0, MPI_COMM_WORLD);
            MPI_Send(new_sol.lecturerIds, tuples_size, MPI_INT, ROOT_THREAD, 0, MPI_COMM_WORLD);
            MPI_Send(new_sol.roomIds, tuples_size, MPI_INT, ROOT_THREAD, 0, MPI_COMM_WORLD);

            freePopulationBuffer(new_sol);
        }
        if (rank == ROOT_THREAD)
        {
            population.clear();
            for (int i = 0; i < size; i++)
            {
                if (i != ROOT_THREAD)
                {
                    int *pop_size_buff = getIntsMessage(i);
                    int *tuplesNumInPeriods_buff = getIntsMessage(i);
                    int *tuplesIds_buff = getIntsMessage(i);
                    int *groupIds_buff = getIntsMessage(i);
                    int *lecturerIds_buff = getIntsMessage(i);
                    int *roomIds_buff = getIntsMessage(i);

                    PopulationBuffer recived_sol_buff = {
                        pop_size_buff[0],
                        tuplesNumInPeriods_buff,
                        tuplesIds_buff,
                        groupIds_buff,
                        lecturerIds_buff,
                        roomIds_buff};

                    vector<Solution *> rec_sol = deserialize(recived_sol_buff);
                    freePopulationBuffer(recived_sol_buff);
                    population.insert(population.end(), rec_sol.begin(), rec_sol.end());
                }
            }

            population = naturalSelection(population);

            //print solution
            iterNr++;
            for (int j = 0; j < population.size(); j++)
            {
                if (countSolutionCost(population[j]) == 0 && (iterNr++ > MAX_ITER_NR))
                {
                    printCSVSolution(population[j]);
                    exe_end = true;
                }
            }
        }
    }
    freePopulationBuffer(popBuffer);

    MPI_Finalize();
    return 0;
}