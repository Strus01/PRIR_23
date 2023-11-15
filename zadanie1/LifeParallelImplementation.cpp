#include "LifeParallelImplementation.h"
#include "mpi.h"

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int currentState, currentPollution;

    for (int row = 0; row < size_1; row++) {
        for (int col = beginning; col < end; col++) {
            currentState = localCells[row][col];
            currentPollution = localPollution[row][col];
            cellsNext[row][col] = rules->cellNextState(currentState,
                                                       liveNeighbours(row, col),
                                                       currentPollution);
            pollutionNext[row][col] = rules->nextPollution(currentState,
                                                           currentPollution,
                                                           localPollution[row + 1][col] + localPollution[row - 1][col] + localPollution[row][col - 1] + localPollution[row][col + 1],
                                                           localPollution[row - 1][col - 1] + localPollution[row - 1][col + 1] + localPollution[row + 1][col - 1] + localPollution[row + 1][col + 1]);
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

    sizeOfPartition = size / noProcesses;
    beginning = processID * sizeOfPartition;
    end = beginning + sizeOfPartition;

    localCells = new int*[size];
    localPollution = new int*[size];

    for (int i = 0; i < size_1; i++) {
        localCells[i] = new int[sizeOfPartition];
        localPollution[i] = new int[sizeOfPartition];
    }

    for (int i = 0; i < size_1; i++) {
        for (int j = beginning; j < end; j++) {
            localCells[i][j] = cells[i][j];
            localPollution[i][j] = pollution[i][j];
        }
    }

    if (processID == 0) {
        rightBorder = new int*[2];
        for (int i = 0; i < 2; i++) {
            rightBorder[i] = new int[size_1];
        }
    }
    else if (processID == noProcesses - 1) {
        leftBorder = new int*[2];
        for (int i = 0; i < 2; i++) {
            leftBorder[i] = new int[size_1];
        }
    } else {
        leftBorder = new int*[2];
        rightBorder = new int*[2];
        for (int i = 0; i < 2; i++) {
            leftBorder[i] = new int[size_1];
            rightBorder[i] = new int[size_1];
        }
    }
}

void LifeParallelImplementation::afterLastStep() {
    for (int i = 0; i < size_1; i++) {
        delete[] localCells[i];
        delete[] localPollution[i];
    }
    delete[] localCells;
    delete[] localPollution;

    if (processID == 0) {
        for (int i = 0; i < 2; i++) {
            delete[] rightBorder[i];
        }
        delete[] rightBorder;
    }
    else if (processID == noProcesses - 1) {
        for (int i = 0; i < 2; i++) {
            delete[] leftBorder[i];
        }
        delete[] leftBorder;
    } else {
        for (int i = 0; i < 2; i++) {
            delete[] leftBorder[i];
            delete[] rightBorder[i];
        }
        delete[] leftBorder;
        delete[] rightBorder;
    }
}

void LifeParallelImplementation::exchangeBorders() {
    int prevRank = (processID - 1 + noProcesses) % noProcesses;
    int nextRank = (processID + 1) % noProcesses;

    if (processID == 0) {
        MPI_Send(rightBorder[CELLS_BORDER_IDX], size_1, MPI_INT, nextRank, 0, MPI_COMM_WORLD);
        MPI_Send(rightBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, nextRank, 1, MPI_COMM_WORLD);

        MPI_Recv(rightBorder[CELLS_BORDER_IDX], size_1, MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(rightBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, nextRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else if (processID == noProcesses - 1) {
        MPI_Send(leftBorder[CELLS_BORDER_IDX], size_1, MPI_INT, prevRank, 0, MPI_COMM_WORLD);
        MPI_Send(leftBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, prevRank, 1, MPI_COMM_WORLD);

        MPI_Recv(leftBorder[CELLS_BORDER_IDX], size_1, MPI_INT, prevRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(leftBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, prevRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
        MPI_Send(leftBorder[CELLS_BORDER_IDX], size_1, MPI_INT, prevRank, 0, MPI_COMM_WORLD);
        MPI_Send(leftBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, prevRank, 1, MPI_COMM_WORLD);
        MPI_Send(rightBorder[CELLS_BORDER_IDX], size_1, MPI_INT, nextRank, 2, MPI_COMM_WORLD);
        MPI_Send(rightBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, nextRank, 3, MPI_COMM_WORLD);

        MPI_Recv(leftBorder[CELLS_BORDER_IDX], size_1, MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(leftBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, nextRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(rightBorder[CELLS_BORDER_IDX], size_1, MPI_INT, nextRank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(rightBorder[POLLUTION_BORDER_IDX], size_1, MPI_INT, nextRank, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void LifeParallelImplementation::swapTables() {
    int **tmp;

    tmp = localCells;
    localCells = cellsNext;
    cellsNext = tmp;

    tmp = localPollution;
    localPollution = pollutionNext;
    pollutionNext = tmp;
}
