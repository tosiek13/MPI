

struct SerializedPopulation{
    std::vector<int> tuplesNumInPeriods;
    std::vector<int> tuplesIds;
    std::vector<int> groupIds;
    std::vector<int> lecturerIds;
    std::vector<int> roomIds;
};

SerializedPopulation serilize(vector<Solution *> population){
    SerializedPopulation result;


    int *wektor = new int[PERIODS_AMOUNT*INITIAL_SOLUTIONS_AMOUNT];
    for(int j = 0; j < population.size() ; j++ ){
        for( int i = 0; i < PERIODS_AMOUNT; i++ ){
            result.tuplesNumInPeriods.push_back( population[j]->periods[i]->tuples.size() );
            for(int k = 0; k < population[j]->periods[i]->tuples.size() ; k++){
                result.tuplesIds.push_back(population[j]->periods[i]->tuples[k]->id);
                result.groupIds.push_back(population[j]->periods[i]->tuples[k]->groupId);
                result.lecturerIds.push_back(population[j]->periods[i]->tuples[k]->lecturerId);
                result.roomIds.push_back(population[j]->periods[i]->tuples[k]->roomId);            
            }
        }
    }
    return result;
}




 vector<Solution *>  deserialize(SerializedPopulation sp){
    vector<Solution *> result;
    int populationSize =  sp.tuplesNumInPeriods.size()/PERIODS_AMOUNT;
    int tupleCounter = 0;

    for(int i = 0; i < populationSize; i++){
        Solution * newSol = new Solution[1];
        for(int j = 0; j < PERIODS_AMOUNT; j++){
            Period * newPeriod = new Period[1];
            int tuplesNum = sp.tuplesNumInPeriods[i*PERIODS_AMOUNT +j];
            for(int k =0; k < tuplesNum; k++){
                int tupleId = sp.tuplesIds[tupleCounter];
                int groupId = sp.groupIds[tupleCounter];
                int lecturerId = sp.lecturerIds[tupleCounter];
                int roomId = sp.roomIds[tupleCounter];

                Tuple * newTuple = new Tuple(tupleId,  lecturerId,groupId, roomId);
                newPeriod->tuples.push_back(newTuple);
                tupleCounter++;
            }
            newSol->periods.push_back(newPeriod);
        }
        result.push_back(newSol);
    }
    return result;
}


void sendSerializedPopulation(SerializedPopulation sp, int reciverId ){
        int* periodsNum = new int[1];
    int* tuplesNum = new int [1];

    * periodsNum = sp.tuplesNumInPeriods.size();
    * tuplesNum = sp.roomIds.size();


    MPI_Send(periodsNum, 1, MPI_INT, reciverId, 121, MPI_COMM_WORLD);
    MPI_Send(tuplesNum, 1, MPI_INT, reciverId, 123, MPI_COMM_WORLD);

    MPI_Send(&sp.tuplesNumInPeriods[0], *periodsNum, MPI_INT, reciverId, 14, MPI_COMM_WORLD);

    MPI_Send(&sp.tuplesIds[0], *tuplesNum, MPI_INT, reciverId, 10, MPI_COMM_WORLD);
    MPI_Send(&sp.groupIds[0], *tuplesNum, MPI_INT, reciverId, 11, MPI_COMM_WORLD);
    MPI_Send(&sp.lecturerIds[0], *tuplesNum, MPI_INT, reciverId, 12, MPI_COMM_WORLD);
    MPI_Send(&sp.roomIds[0], *tuplesNum, MPI_INT, reciverId, 13, MPI_COMM_WORLD);



}

void broadcastPopulation(SerializedPopulation sp){

    for (int i = 1; i < 8; i++ ){
        sendSerializedPopulation(sp, i);
    }

}

SerializedPopulation recivePopulation(int senderId, MPI_Status * status){
    SerializedPopulation result;

    int* periodsNum = new int[1];
    int* tuplesNum = new int [1];

     


    MPI_Recv(periodsNum, 1, MPI_INT, senderId, 121, MPI_COMM_WORLD, status);
    MPI_Recv(tuplesNum, 1, MPI_INT, senderId, 123, MPI_COMM_WORLD, status);

    int * tuplesNumInPeriods = new int[*periodsNum];

    int * tuplesIds = new int[*tuplesNum];
    int * groupIds = new int[*tuplesNum];
    int * lecturerIds = new int[*tuplesNum];
    int * roomIds = new int[*tuplesNum];



    MPI_Recv(tuplesNumInPeriods, *periodsNum, MPI_INT, senderId, 14, MPI_COMM_WORLD, status);

    MPI_Recv(tuplesIds, *tuplesNum, MPI_INT, senderId, 10, MPI_COMM_WORLD, status);
    MPI_Recv(groupIds, *tuplesNum, MPI_INT, senderId, 11, MPI_COMM_WORLD, status);
    MPI_Recv(lecturerIds, *tuplesNum, MPI_INT, senderId, 12, MPI_COMM_WORLD, status);
    MPI_Recv(roomIds, *tuplesNum, MPI_INT, senderId, 13, MPI_COMM_WORLD, status);


    result.tuplesNumInPeriods = vector<int>(tuplesNumInPeriods, tuplesNumInPeriods + *periodsNum);
    result.tuplesIds = vector<int>(tuplesIds, tuplesIds + *tuplesNum);
    result.groupIds = vector<int>(groupIds, groupIds + *tuplesNum);
    result.lecturerIds = vector<int>(lecturerIds, lecturerIds + *tuplesNum);
    result.roomIds = vector<int>(roomIds, roomIds + *tuplesNum);


    
    return result;
}


void sendNewSolutionsToMaster(SerializedPopulation sp){
    sendSerializedPopulation(sp, 0);
}




void masterInitialization(){}