#include <iostream>
#include <random>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <stdlib.h>
#include <mpi.h>

using namespace std;

const int POPULATION_SIZE = 160;
const int INITIAL_SOLUTIONS_AMOUNT = 100;
const int TUPLES_AMOUNT = 50;
const int PERIODS_AMOUNT = 50;
const int LECTURERS_AMOUNT = 50;
const int ROOMS_AMOUNT = 3;
const int GROUPS_AMOUNT = 50;
const int NEW_SOLUTIONS_NUM = 20;
float POPULATION_INCREASE_FAKTOR = 1;

struct Tuple
{
    int id, groupId, lecturerId, roomId;
    Tuple(int id, int lecturerId, int groupId, int roomId) : id(id), lecturerId(lecturerId), groupId(groupId), roomId(roomId) {}
};

vector<Tuple *> tuples_org;

struct Period
{
    vector<Tuple *> tuples;
};

struct Solution
{
    vector<Period *> periods;
    Solution(vector<Period *> periods) : periods(periods) {}
    Solution() {}
};

/*UTILS*/
int getRandRangeInt(int beg, int end)
{
    return rand() % end + beg;
}

Tuple *getTupleById(Solution *s, int id)
{
    for (int i = 0; i < s->periods.size(); i++)
    {
        Period *period = s->periods[i];
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples[j];
            if (tuple->id == id)
            {
                return tuple;
            }
        }
    }
}

/*Solution genetic processing functions*/
int countSolutionCost(Solution *s)
{
    int cost = 0;
    for (int i = 0; i < s->periods.size(); i++)
    {
        Period *period = s->periods[i];
        set<int> usedRoomIds;
        set<int> usedLecturersIds;
        set<int> usedGroupIds;
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples[j];
            if (usedRoomIds.find(tuple->roomId) != usedRoomIds.end())
            {
                cost++;
            }
            if (usedLecturersIds.find(tuple->roomId) != usedLecturersIds.end())
            {
                cost++;
            }
            if (usedGroupIds.find(tuple->roomId) != usedGroupIds.end())
            {
                cost++;
            }
            usedRoomIds.insert(tuple->roomId);
            usedLecturersIds.insert(tuple->lecturerId);
            usedGroupIds.insert(tuple->groupId);
        }
    }
    return cost;
}

void recombineSolution(Solution *s)
{
    //prepare
    vector<int> duplicatedIds;
    vector<int> missingIds;
    std::set<int> ids;
    for (int i = 0; i < s->periods.size(); i++)
    {
        Period *period = s->periods[i];
        for (int j = 0; j < period->tuples.size(); j++)
        {
            int tupleId = period->tuples[j]->id;
            if (ids.find(tupleId) == ids.end())
            {
                ids.insert(tupleId);
            }
            else
            {
                duplicatedIds.push_back(tupleId);
            }
        }
    }
    for (int i = 0; i < tuples_org.size(); i++)
    {
        int tupleId = tuples_org[i]->id;
        if (ids.find(tupleId) == ids.end())
        {
            missingIds.push_back(tupleId);
        }
    }

    for (int i = 0; i < duplicatedIds.size(); i++)
    {
        int duplicatedTupleId = duplicatedIds[i];
        Tuple *dup_Tuple = getTupleById(s, duplicatedTupleId);
        // cout << "tuple by id end" << endl;
        Tuple *orginalTuple = tuples_org[missingIds[0]];
        // cout << "orginal tuple id end" << endl;
        missingIds.erase(missingIds.begin());

        //reasign values
        dup_Tuple->id = orginalTuple->id;
        dup_Tuple->groupId = orginalTuple->groupId;
        dup_Tuple->lecturerId = orginalTuple->lecturerId;
        dup_Tuple->roomId = orginalTuple->roomId;
    }
}

void mutateSolution(Solution *s)
{
    // cout<<"Mutation not implements yet"<<endl;
}

/*Solution genetic processing functions*/ //END

vector<Tuple *> createSolutionTuples()
{
    std::vector<Tuple *> tuples;
    int id = 0;
    for (int classesId = 0; classesId < TUPLES_AMOUNT; classesId++)
    {
        int lecturerId = getRandRangeInt(0, LECTURERS_AMOUNT);
        int groupId = getRandRangeInt(0, GROUPS_AMOUNT);
        int roomId = getRandRangeInt(0, ROOMS_AMOUNT);

        Tuple *tuple = new Tuple(id++, lecturerId, groupId, roomId);
        tuples.push_back(tuple);
    }
    return tuples;
}

void printToConsole(Solution *s)
{
    for (int i = 0; i < s->periods.size(); i++)
    {
        std::cout << "Period " << i << endl;
        Period *period = s->periods.at(i);
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples.at(j);
            std::cout << "id: " << tuple->id << ", groupId: " << tuple->groupId << ", lecturerId: " << tuple->lecturerId << ", roomId: " << tuple->roomId << endl;
            // printf ("%s \n", "A string");
        }
    }
}

/*OTHER FUNCTIONS*/

Solution *generateRandomSolution(vector<Tuple *> tuples, int periodsAmount)
{
    //initialize
    std::vector<Period *> periods(periodsAmount);
    for (int i = 0; i < periodsAmount; i++)
    {
        periods[i] = new Period();
    }
    //
    for (int i = 0; i < tuples.size(); i++)
    {
        int periodId = getRandRangeInt(0, periodsAmount);
        Tuple *tuple = tuples[i];
        periods[periodId]->tuples.push_back(new Tuple(tuple->id, tuple->groupId, tuple->lecturerId, tuple->roomId));
    }
    return new Solution(periods);
}

vector<Solution *> createPopulation(int solutionsAmount)
{
    vector<Solution *> solutions;
    for (int i = 0; i < solutionsAmount; i++)
    {
        solutions.push_back(generateRandomSolution(tuples_org, PERIODS_AMOUNT));
    }
    return solutions;
}

Period *periodsCrossover(Period *p1, Period *p2)
{
    int p1_tuplesAmount = p1->tuples.size();
    int p2_tuplesAmount = p2->tuples.size();
    int smallerAmount = p1_tuplesAmount > p2_tuplesAmount ? p2_tuplesAmount : p1_tuplesAmount;

    Period *p_result = new Period();

    int amountOfNotChangingTuples = getRandRangeInt(0, smallerAmount + 1);
    //reasign not changed values
    for (int i = 0; i < amountOfNotChangingTuples; i++)
    {
        p_result->tuples.push_back(p1->tuples[i]);
    }
    // assign new tuples
    for (int i = amountOfNotChangingTuples; i < p2->tuples.size(); i++)
    {
        p_result->tuples.push_back(p2->tuples[i]);
    }
    return p_result;
}

Solution *crossSolutions(Solution *s1, Solution *s2)
{
    vector<Period *> periods_result;
    for (int i = 0; i < s1->periods.size(); i++)
    {
        Period *crossOveredPeriod = periodsCrossover(s1->periods[i], s2->periods[i]);
        periods_result.push_back(crossOveredPeriod);
    }
    return new Solution(periods_result);
}

vector<Solution *> naturalSelection(vector<Solution *> population)
{
    vector<int> costs(population.size());
    int max_cost = INT_MIN;
    for (int i = 0; i < population.size(); i++)
    {
        int cost = countSolutionCost(population[i]);
        costs[i] = cost;
        if (cost > max_cost)
        {
            max_cost = cost;
        }
    }
    std::sort(costs.rbegin(), costs.rend());
    //degug
    cout << "costs:" << endl;
    for (int i = 0; i < costs.size(); i++)
    {
        cout << costs[i] << ", ";
    }
    cout << endl;
    //end debug
    // casual mode
    int selectionMode = 0;
    if (population.size() <= 10)
    {
        // slow mode
        selectionMode = 1;
    }

    int survialsAmount = population.size() >= 10 ? ceil(population.size() * 0.5) : population.size() - 1;
    int threshold = costs.at(population.size() - survialsAmount - 1);

    vector<Solution *> populationSelected;
    int addedToSolution = 0;
    for (int i = 0; i < population.size(); i++)
    {
        bool reasign = false;
        if (addedToSolution < survialsAmount)
            switch (selectionMode)
            {
            case 0:
                if (countSolutionCost(population[i]) <= threshold)
                {
                    reasign = true;
                }
                break;
            case 1:
                reasign = true;
                break;
            }
        if (reasign)
        {
            addedToSolution++;
            populationSelected.push_back(population[i]);
        }
    }
    //when all reasigned - remove one solution
    return populationSelected;
}

bool isSolutionSatisfying(vector<Solution *> population)
{
    return false;
}

void printPopulation(vector<Solution *> population)
{
    for (int i = 0; i < 1; i++)
    {
        printToConsole(population[i]);
    }
}

void printRoomSolution(Solution *population)
{
    //periods iterate
    for (int i = 0; i < population->periods.size(); i++)
    {
        // tuple iterate
        Period *period = population->periods.at(i);
        vector<int> roomOverload(ROOMS_AMOUNT, 0);
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples.at(j);
            roomOverload.at(tuple->roomId) = roomOverload.at(tuple->roomId) + 1;
        }
        for (int j = 0; j < roomOverload.size(); j++)
        {
            cout << roomOverload.at(j) << ";";
        }
        cout << endl;
    }
}

void printRoomsOverload(ofstream &ofstream, Solution *population)
{
    ofstream << "Room Overload" << endl;
    //periods iterate
    for (int i = 0; i < population->periods.size(); i++)
    {
        // tuple iterate
        Period *period = population->periods.at(i);
        vector<int> roomOverload(ROOMS_AMOUNT, 0);
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples.at(j);
            roomOverload.at(tuple->roomId) = roomOverload.at(tuple->roomId) + 1;
        }
        for (int j = 0; j < roomOverload.size(); j++)
        {
            ofstream << roomOverload.at(j) << ";";
        }
        ofstream << endl;
    }
}

void printGroupsOverload(ofstream &ofstream, Solution *population)
{
    ofstream << "Group Overload" << endl;
    //periods iterate
    for (int i = 0; i < population->periods.size(); i++)
    {
        // tuple iterate
        Period *period = population->periods.at(i);
        vector<int> roomOverload(GROUPS_AMOUNT, 0);
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples.at(j);
            roomOverload.at(tuple->groupId) = roomOverload.at(tuple->roomId) + 1;
        }
        for (int j = 0; j < roomOverload.size(); j++)
        {
            ofstream << roomOverload.at(j) << ";";
        }
        ofstream << endl;
    }
}

void printLecturersOverload(ofstream &ofstream, Solution *population)
{
    ofstream << "Lecturer Overload" << endl;
    //periods iterate
    for (int i = 0; i < population->periods.size(); i++)
    {
        // tuple iterate
        Period *period = population->periods.at(i);
        vector<int> roomOverload(LECTURERS_AMOUNT, 0);
        for (int j = 0; j < period->tuples.size(); j++)
        {
            Tuple *tuple = period->tuples.at(j);
            roomOverload.at(tuple->lecturerId) = roomOverload.at(tuple->roomId) + 1;
        }
        for (int j = 0; j < roomOverload.size(); j++)
        {
            ofstream << roomOverload.at(j) << ";";
        }
        ofstream << endl;
    }
}

void printCSVSolution(Solution *solution)
{
    ofstream myfile;
    myfile.open("solution.csv");

    printRoomsOverload(myfile, solution);
    printGroupsOverload(myfile, solution);
    printLecturersOverload(myfile, solution);

    myfile.close();
}

vector<Solution *> createNewSolutions(vector<Solution *> population)
{
    int coresAmount;
    MPI_Comm_size(MPI_COMM_WORLD, &coresAmount);
    int newSolutionToCreate = POPULATION_SIZE / coresAmount;

    vector<Solution *> new_population;
    for (int i = 0; i < newSolutionToCreate; i++)
    {

        int index1 = getRandRangeInt(0, population.size());
        int index2 = getRandRangeInt(0, population.size());
        Solution *new_solution = crossSolutions(population[index1], population[index2]);
        recombineSolution(new_solution);
        mutateSolution(new_solution);
        new_population.push_back(new_solution);
    }

    return new_population;
}