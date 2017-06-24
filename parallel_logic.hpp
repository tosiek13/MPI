

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
    int pop_size;
    int *tuplesNumInPeriods;
    int *tuplesIds;
    int *groupIds;
    int *lecturerIds;
    int *roomIds;
};

/**/
int *getIntsMessage(int srcRank)
{
    int int_amount;
    int *ints_buffer;

    MPI_Status status;
    MPI_Probe(srcRank, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_INT, &int_amount);

    ints_buffer = new int[int_amount];

    MPI_Recv(ints_buffer, int_amount, MPI_INT, srcRank, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return ints_buffer;
}

PopulationBuffer allocatePopulationBuffer(int pop_size)
{
    int pop_periods_amount = pop_size * PERIODS_AMOUNT;
    int pop_tuples_size = pop_size * TUPLES_AMOUNT;

    // allocate memory
    int *periodTuplesAmounts = new int[pop_periods_amount];
    int *tuplesIDBuffer = new int[pop_tuples_size];
    int *tuplesGroupBuffer = new int[pop_tuples_size];
    int *tuplesLecturerBuffer = new int[pop_tuples_size];
    int *tuplesRoomBuffer = new int[pop_tuples_size];

    PopulationBuffer popBuffer = {pop_size,
                                  periodTuplesAmounts,
                                  tuplesIDBuffer,
                                  tuplesGroupBuffer,
                                  tuplesLecturerBuffer,
                                  tuplesRoomBuffer};
    return popBuffer;
}

void writeToBuffer(vector<Solution *> population, PopulationBuffer *popBuffer)
{
    int index_1 = 0;
    int index_2 = 0;
    for (int j = 0; j < population.size(); j++)
    {
        for (int i = 0; i < population[j]->periods.size(); i++)
        {
            popBuffer->tuplesNumInPeriods[index_1++] = population[j]->periods[i]->tuples.size();
            for (int k = 0; k < population[j]->periods[i]->tuples.size(); k++)
            {
                popBuffer->tuplesIds[index_2] = population[j]->periods[i]->tuples[k]->id;
                popBuffer->groupIds[index_2] = population[j]->periods[i]->tuples[k]->groupId;
                popBuffer->lecturerIds[index_2] = population[j]->periods[i]->tuples[k]->lecturerId;
                popBuffer->roomIds[index_2] = population[j]->periods[i]->tuples[k]->roomId;
                index_2++;
            }
        }
    }
}

PopulationBuffer serilize(vector<Solution *> population)
{
    int pop_size = population.size();
    int pop_periods_amount = pop_size * PERIODS_AMOUNT;
    int pop_tuples_size = pop_size * TUPLES_AMOUNT;

    // allocate memory
    // cout << "Allocation" << endl;
    int *periodTuplesAmounts = new int[pop_periods_amount];
    int *tuplesIDBuffer = new int[pop_tuples_size];
    int *tuplesGroupBuffer = new int[pop_tuples_size];
    int *tuplesLecturerBuffer = new int[pop_tuples_size];
    int *tuplesRoomBuffer = new int[pop_tuples_size];

    PopulationBuffer popBuffer = {pop_size,
                                  periodTuplesAmounts,
                                  tuplesIDBuffer,
                                  tuplesGroupBuffer,
                                  tuplesLecturerBuffer,
                                  tuplesRoomBuffer};
    // reasign data
    // cout << "Reasignments" << endl;
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
    return popBuffer;
}

vector<Solution *> deserialize(PopulationBuffer popBuffer)
{
    vector<Solution *> result;
    int tupleCounter = 0;
    for (int i = 0; i < popBuffer.pop_size; i++)
    {
        Solution *newSol = new Solution[1];
        for (int j = 0; j < PERIODS_AMOUNT; j++)
        {
            Period *newPeriod = new Period[1];
            int tuplesNum = popBuffer.tuplesNumInPeriods[i * PERIODS_AMOUNT + j];
            for (int k = 0; k < tuplesNum; k++)
            {
                int tupleId = popBuffer.tuplesIds[tupleCounter];
                int groupId = popBuffer.groupIds[tupleCounter];
                int lecturerId = popBuffer.lecturerIds[tupleCounter];
                int roomId = popBuffer.roomIds[tupleCounter];

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

void broadcastPopulationBuffer(PopulationBuffer &popBuffer, int rootRank)
{
    int pop_periods_amount = popBuffer.pop_size * PERIODS_AMOUNT;
    int pop_tuples_size = popBuffer.pop_size * TUPLES_AMOUNT;

    MPI_Bcast(&(popBuffer.pop_size), 1, MPI_INT, rootRank, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.tuplesNumInPeriods, pop_periods_amount, MPI_INT, rootRank, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.tuplesIds, pop_tuples_size, MPI_INT, rootRank, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.groupIds, pop_tuples_size, MPI_INT, rootRank, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.lecturerIds, pop_tuples_size, MPI_INT, rootRank, MPI_COMM_WORLD);
    MPI_Bcast(popBuffer.roomIds, pop_tuples_size, MPI_INT, rootRank, MPI_COMM_WORLD);
}

void sendPopulationBuffer(PopulationBuffer &popBuffer, int destRank)
{
    int pop_size = popBuffer.pop_size;
    int periods_size = pop_size * PERIODS_AMOUNT;
    int tuples_size = pop_size * TUPLES_AMOUNT;

    MPI_Send(&(popBuffer.pop_size), 1, MPI_INT, destRank, 0, MPI_COMM_WORLD);
    MPI_Send(popBuffer.tuplesNumInPeriods, periods_size, MPI_INT, destRank, 0, MPI_COMM_WORLD);
    MPI_Send(popBuffer.tuplesIds, tuples_size, MPI_INT, destRank, 0, MPI_COMM_WORLD);
    MPI_Send(popBuffer.groupIds, tuples_size, MPI_INT, destRank, 0, MPI_COMM_WORLD);
    MPI_Send(popBuffer.lecturerIds, tuples_size, MPI_INT, destRank, 0, MPI_COMM_WORLD);
    MPI_Send(popBuffer.roomIds, tuples_size, MPI_INT, destRank, 0, MPI_COMM_WORLD);
}

PopulationBuffer receivePopulationBuffer(int srcRank)
{
    int *pop_size_buff = getIntsMessage(srcRank);
    int *tuplesNumInPeriods_buff = getIntsMessage(srcRank);
    int *tuplesIds_buff = getIntsMessage(srcRank);
    int *groupIds_buff = getIntsMessage(srcRank);
    int *lecturerIds_buff = getIntsMessage(srcRank);
    int *roomIds_buff = getIntsMessage(srcRank);

    PopulationBuffer recived_sol_buff = {
        pop_size_buff[0],
        tuplesNumInPeriods_buff,
        tuplesIds_buff,
        groupIds_buff,
        lecturerIds_buff,
        roomIds_buff};

    return recived_sol_buff;
}

void freePopulationBuffer(PopulationBuffer &pop_buff)
{
    delete[] pop_buff.tuplesNumInPeriods;
    delete[] pop_buff.tuplesIds;
    delete[] pop_buff.groupIds;
    delete[] pop_buff.lecturerIds;
    delete[] pop_buff.roomIds;
}

void sendNewSolutionsToMaster(SerializedPopulation sp)
{
    sendSerializedPopulation(sp, 0);
}

void masterInitialization() {}