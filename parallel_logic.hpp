

struct SerializedPopulation
{
    std::vector<int> tuplesNumInPeriods;
    std::vector<int> tuplesIds;
    std::vector<int> groupIds;
    std::vector<int> lecturerIds;
    std::vector<int> roomIds;
};

struct PopulationBuffer
{
    int *tuplesNumInPeriods;
    int *tuplesIds;
    int *groupIds;
    int *lecturerIds;
    int *roomIds;
};

void serilize(vector<Solution *> population, PopulationBuffer popBuffer)
{
    int index_1 = 0;
    int index_2 = 0;
    for (int j = 0; j < population.size(); j++)
    {
        for (int i = 0; i < population[j]->periods.size(); i++)
        {
            popBuffer.tuplesNumInPeriods[index_1++] = population[j]->periods[i]->tuples.size();
            for (int k = 0; k < population[j]->periods[i]->tuples.size(); k++)
            {
                popBuffer.tuplesIds[index_2] = population[j]->periods[i]->tuples[k]->id;
                popBuffer.groupIds[index_2] = population[j]->periods[i]->tuples[k]->groupId;
                popBuffer.lecturerIds[index_2] = population[j]->periods[i]->tuples[k]->lecturerId;
                popBuffer.roomIds[index_2] = population[j]->periods[i]->tuples[k]->roomId;
                index_2++;
            }
        }
    }
}

vector<Solution *> deserialize(SerializedPopulation sp)
{
    vector<Solution *> result;
    int populationSize = sp.tuplesNumInPeriods.size() / PERIODS_AMOUNT;
    int tupleCounter = 0;

    for (int i = 0; i < populationSize; i++)
    {
        Solution *newSol = new Solution[1];
        for (int j = 0; j < PERIODS_AMOUNT; j++)
        {
            Period *newPeriod = new Period[1];
            int tuplesNum = sp.tuplesNumInPeriods[i * PERIODS_AMOUNT + j];
            for (int k = 0; k < tuplesNum; k++)
            {
                int tupleId = sp.tuplesIds[tupleCounter];
                int groupId = sp.groupIds[tupleCounter];
                int lecturerId = sp.lecturerIds[tupleCounter];
                int roomId = sp.roomIds[tupleCounter];

                Tuple *newTuple = new Tuple(tupleId, lecturerId, groupId, roomId);
                newPeriod->tuples.push_back(newTuple);
                tupleCounter++;
            }
            newSol->periods.push_back(newPeriod);
        }
        result.push_back(newSol);
    }
    return result;
}

void sendSerializedPopulation(SerializedPopulation sp, int reciverId)
{
    int periodAmount = sp.tuplesNumInPeriods.size();
    int tuplesAmount = sp.roomIds.size();

    MPI_Send(&sp.tuplesNumInPeriods[0], periodAmount, MPI_INT, reciverId, 0, MPI_COMM_WORLD);

    MPI_Send(&sp.tuplesIds[0], tuplesAmount, MPI_INT, reciverId, 0, MPI_COMM_WORLD);
    MPI_Send(&sp.groupIds[0], tuplesAmount, MPI_INT, reciverId, 0, MPI_COMM_WORLD);
    MPI_Send(&sp.lecturerIds[0], tuplesAmount, MPI_INT, reciverId, 0, MPI_COMM_WORLD);
    MPI_Send(&sp.roomIds[0], tuplesAmount, MPI_INT, reciverId, 0, MPI_COMM_WORLD);
    cout << "Sent: " << reciverId << endl;
}

void broadcastPopulation(SerializedPopulation sp)
{
    int exeCoreAmount;
    MPI_Comm_size(MPI_COMM_WORLD, &exeCoreAmount);
    for (int i = 1; i < exeCoreAmount; i++)
    {
        // std::cout<<"Message to rank: " <<i <<std::endl;
        sendSerializedPopulation(sp, i);
    }
}

SerializedPopulation recivePopulation(int senderId, MPI_Status *status)
{
    SerializedPopulation result;

    MPI_Status localStatus;
    //debug
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //debug

    cout << "Receive population rank: " << rank << endl;

    // Tuples length info
    MPI_Probe(0, 0, MPI_COMM_WORLD, &localStatus);
    int tuplesNumInPeriodsAmount;
    MPI_Get_count(&localStatus, MPI_INT, &tuplesNumInPeriodsAmount);
    cout << "Tuples num in periods amount: " << tuplesNumInPeriodsAmount << endl;
    int *tuplesNumInPeriods = (int *)malloc(sizeof(int) * tuplesNumInPeriodsAmount);

    MPI_Recv(tuplesNumInPeriods, tuplesNumInPeriodsAmount, MPI_INT, 0, 0,
             MPI_COMM_WORLD, &localStatus);

    cout << "Got array of data length: " << (sizeof(tuplesNumInPeriods) / sizeof(*tuplesNumInPeriods)) << endl;
    // result.tuplesNumInPeriods = vector<int>(tuplesNumInPeriods, tuplesNumInPeriods + tuplesNumInPeriodsAmount);
    free(tuplesNumInPeriods);

    // cout << "Receive population got tuples in period amount:" << endl;

    // recive tuples
    MPI_Probe(0, 0, MPI_COMM_WORLD, status);
    int tuplesAmount;
    MPI_Get_count(status, MPI_INT, &tuplesAmount);

    //ID
    int *tuplesIds = (int *)malloc(sizeof(int) * tuplesAmount);
    MPI_Recv(tuplesIds, tuplesAmount, MPI_INT, 0, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    result.tuplesIds = vector<int>(tuplesIds, tuplesIds + tuplesAmount);
    free(tuplesIds);

    // griups
    int *groupIds = (int *)malloc(sizeof(int) * tuplesAmount);
    MPI_Recv(groupIds, tuplesAmount, MPI_INT, 0, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    result.groupIds = vector<int>(groupIds, groupIds + tuplesAmount);
    free(groupIds);

    // lecurers
    int *lecturerIds = (int *)malloc(sizeof(int) * tuplesAmount);
    MPI_Recv(lecturerIds, tuplesAmount, MPI_INT, 0, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    result.lecturerIds = vector<int>(lecturerIds, lecturerIds + tuplesAmount);
    free(lecturerIds);

    // roomId
    int *roomIds = (int *)malloc(sizeof(int) * tuplesAmount);
    MPI_Recv(roomIds, tuplesAmount, MPI_INT, 0, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    result.roomIds = vector<int>(roomIds, roomIds + tuplesAmount);
    free(roomIds);

    cout << "GOT ALL MESSAGES rank: " << rank << endl;

    return result;
}

void sendNewSolutionsToMaster(SerializedPopulation sp)
{
    sendSerializedPopulation(sp, 0);
}

void masterInitialization() {}