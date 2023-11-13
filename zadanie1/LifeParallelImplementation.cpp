#include "LifeParallelImplementation.h"
#include "mpi.h"

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int currentState, currentPollution;

    for (int row = 0; row < size_1; row++) {
        for (int col = beginning; col < end; col++) {
            currentState = cells[row][col];
            currentPollution = pollution[row][col];
            cellsNext[row][col] = rules->cellNextState(currentState,
                                                       liveNeighbours(row, col),
                                                       currentPollution);
            pollutionNext[row][col] = rules->nextPollution(currentState,
                                                           currentPollution,
                                                           pollution[row + 1][col] + pollution[row - 1][col] + pollution[row][col - 1] + pollution[row][col + 1],
                                                           pollution[row - 1][col - 1] + pollution[row - 1][col + 1] + pollution[row + 1][col - 1] + pollution[row + 1][col + 1]);
        }
    }
}

void LifeParallelImplementation::oneStep() {
    exchangeBorders();
    realStep();
    swapTables();
}

int LifeParallelImplementation::numberOfLivingCells() {}

double LifeParallelImplementation::averagePollution() {}

void LifeParallelImplementation::beforeFirstStep() {
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);

    sizeOfPartition = size_1 / noProcesses;
    beginning = processID * sizeOfPartition;
    end = beginning + sizeOfPartition;

    bufferLeft = new int*[2];
    bufferRight = new int*[2];
    for (int i = 0; i < 2; i++) {
        bufferLeft[i] = new int[size_1];
        bufferRight[i] = new int[size_1];
    }
}

void LifeParallelImplementation::afterLastStep() {
    for (int i = 0; i < 2; i++){
        delete[] bufferLeft[i];
        delete[] bufferRight[i];
    }
    delete[] bufferLeft;
    delete[] bufferRight;
}

void LifeParallelImplementation::exchangeBorders() {

}