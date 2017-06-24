#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <climits>
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

const int MUTATION_PROPABILITY = 0.3;
const float SURVIVAL_RATE = 0.8;
const int MAX_ITER_NR = 5;

struct Tuple
{
    int id, groupId, lecturerId, roomId;
    Tuple(int id, int lecturerId, int groupId, int roomId) : id(id), lecturerId(lecturerId), groupId(groupId), roomId(roomId) {}
};

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

Tuple *copyTuple(Tuple *tuple)
{
    return new Tuple(tuple->id, tuple->groupId, tuple->lecturerId, tuple->roomId);
}

/*Solution genetic processing functions*/
int countSolutionCost(Solution *s)
{
    int cost = 0;
    for (int i = 0; i < s->periods.size(); i++)
    {
        // cout << "Period: " << i << std::endl;
        Period *period = s->periods[i];
        set<int> usedRoomIds;
        set<int> usedLecturersIds;
        set<int> usedGroupIds;
        for (int j = 0; j < period->tuples.size(); j++)
        {
            // cout<<"get tuple: " << j << std::endl;
            Tuple *tuple = period->tuples[j];
            // cout<<"got it"<<std::endl;
            if (usedRoomIds.find(tuple->roomId) != usedRoomIds.end())
            {
                cost++;
            }
            if (usedLecturersIds.find(tuple->lecturerId) != usedLecturersIds.end())
            {
                cost++;
            }
            if (usedGroupIds.find(tuple->groupId) != usedGroupIds.end())
            {
                cost++;
            }
            // usedRoomIds.insert(tuple->roomId);
            // usedLecturersIds.insert(tuple->lecturerId);
            // usedGroupIds.insert(tuple->groupId);
            usedRoomIds.insert(tuple->roomId);
            usedLecturersIds.insert(tuple->lecturerId);
            usedGroupIds.insert(tuple->groupId);
        }
        // cout << "Period END: " << i << std::endl;
    }
    return cost;
}

void recombineSolution(Solution *s, Solution *s_org)
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
    //get original tuples
    vector<Tuple *> org_sol_tuples;
    for (int i = 0; i < s_org->periods.size(); i++)
    {
        Period *period = s_org->periods[i];
        for (int j = 0; j < period->tuples.size(); j++)
        {
            org_sol_tuples.push_back(period->tuples[j]);
        }
    }

    for (int i = 0; i < org_sol_tuples.size(); i++)
    {
        int tupleId = org_sol_tuples[i]->id;
        if (ids.find(tupleId) == ids.end())
        {
            missingIds.push_back(tupleId);
        }
    }

    for (int i = 0; i < duplicatedIds.size(); i++)
    {
        int duplicatedTupleId = duplicatedIds[i];
        Tuple *dup_Tuple = getTupleById(s, duplicatedTupleId);
        Tuple *orginalTuple = getTupleById(s_org, missingIds[0]);

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
    // CHECK WHETHER MUTATION SHOULD HAPPEND
    int randValue = getRandRangeInt(0, INT_MAX);
    if (randValue < MUTATION_PROPABILITY * INT_MAX)
    {
        //PERFORM MUTATION
        int periodFrom = getRandRangeInt(0, s->periods.size());
        int periodTo = getRandRangeInt(0, s->periods.size());

        if (periodFrom == periodTo)
        {
            return;
        }

        int tupleFrom = getRandRangeInt(0, s->periods[periodFrom]->tuples.size());
        int toTuple = getRandRangeInt(0, s->periods[periodTo]->tuples.size());

        s->periods[periodTo]->tuples.push_back(s->periods[periodFrom]->tuples[tupleFrom]);
        s->periods[periodFrom]->tuples.erase(s->periods[periodFrom]->tuples.begin() + tupleFrom);
    }
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

void createSolutionTuplesFile()
{
    ofstream myfile;
    myfile.open("tuples.csv");
    int id = 0;
    for (int id = 0; id < 1000; id++)
    {
        int lecturerId = getRandRangeInt(0, LECTURERS_AMOUNT);
        int groupId = getRandRangeInt(0, GROUPS_AMOUNT);
        int roomId = getRandRangeInt(0, ROOMS_AMOUNT);

        myfile << id << ";" << lecturerId << ";" << groupId << ";" << roomId << endl;
    }
    if (myfile.is_open())
    {
        // myfile.close();
    }
}

vector<Tuple *> readSolutionTuplesFile()
{
    std::vector<Tuple *> tuples;

    string line;
    ifstream myfile("tuples.csv");
    if (myfile.is_open())
    {
        int readTuples = 0;
        while (getline(myfile, line) && (readTuples < TUPLES_AMOUNT))
        {
            std::vector<int> indexes;
            for (int i = 0; i < line.length(); i++)
            {
                if (line[i] == ';')
                {
                    indexes.push_back(i);
                }
            }

            int id = atoi(line.substr(0, indexes[0]).c_str());
            int lecturerId = atoi(line.substr(indexes[0] + 1, indexes[1]).c_str());
            int groupId = atoi(line.substr(indexes[1] + 1, indexes[2]).c_str());
            int roomId = atoi(line.substr(indexes[2] + 1, line.length()).c_str());

            // cout << "Tuple read: " << id << " " << lecturerId << " " << groupId << " " << roomId << endl;

            Tuple *tuple = new Tuple(id, lecturerId, groupId, roomId);
            tuples.push_back(tuple);
            readTuples++;
            // cout << line << '\n';
        }
        myfile.close();
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

vector<Solution *> createPopulation(vector<Tuple *> tuples_org, int solutionsAmount)
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
        Tuple *newTuple = copyTuple(p1->tuples[i]);
        p_result->tuples.push_back(newTuple);
    }
    // assign new tuples
    for (int i = amountOfNotChangingTuples; i < p2->tuples.size(); i++)
    {
        Tuple *newTuple = copyTuple(p2->tuples[i]);
        p_result->tuples.push_back(newTuple);
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
    // cout << "costs:" << endl;
    cout << "MIN cost: " << costs[population.size() - 1] << endl;
    for (int i = 0; i < costs.size(); i++)
    {
        // cout << costs[i] << ", ";
    }
    // cout << endl;
    //end debug
    // casual mode
    int selectionMode = 0;
    if (population.size() <= 10)
    {
        // slow mode
        selectionMode = 1;
    }

    // std::cout << "POP SIZE: " << population.size() << std::endl;
    int survialsAmount = population.size() >= 10 ? ceil(population.size() * SURVIVAL_RATE) : population.size() - 1;
    // std::cout << "Survivals amount: " << survialsAmount << std::endl;
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
        recombineSolution(new_solution, population[index1]);
        // mutateSolution(new_solution);
        new_population.push_back(new_solution);
    }

    return new_population;
}

void printPopulationInfo(vector<Solution *> population)
{
    cout << "POPULATION INFO:" << endl;
    cout << "Solution amount: " << population.size() << endl;
    cout << "Solution 1 :" << endl;
    for (int i = 0; i < population[0]->periods.size(); i++)
    {
        Period *period = population[0]->periods[i];
        for (int j = 0; j < period->tuples.size(); j++)
        {
            cout << period->tuples[j]->id << " ";
        }
        cout << endl;
    }
}