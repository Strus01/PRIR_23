#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <vector>

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int currentState, currentPollution;

    for (int row = 0; row < size_1; row++) {
        for (int col = beginning; col < end; col++) {
            currentState = localCells[row][col];
            currentPollution = localPollution[row][col];
            cells[row][col] = rules->cellNextState(currentState,
                                                       liveNeighbours(row, col),
                                                       currentPollution);
            pollution[row][col] = rules->nextPollution(currentState,
                                                           currentPollution,
                                                           localPollution[row + 1][col] + localPollution[row - 1][col] + localPollution[row][col - 1] + localPollution[row][col + 1],
                                                           localPollution[row - 1][col - 1] + localPollution[row - 1][col + 1] + localPollution[row + 1][col - 1] + localPollution[row + 1][col + 1]);
        }
    }
}

void LifeParallelImplementation::oneStep() {
    exchangeBorders();
    realStep();
}

int LifeParallelImplementation::numberOfLivingCells() {}

double LifeParallelImplementation::averagePollution() {}

void LifeParallelImplementation::beforeFirstStep() {
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);

    sizeOfPartition = size / noProcesses;
    beginning = processID * sizeOfPartition;
    end = beginning + sizeOfPartition;

    for (int i = 0; i < sizeOfPartition; i++) {
        std::vector<int> localCellsRow(cells[i], cells[i] + size_1);
        localCells.push_back(localCellsRow);

        std::vector<int> localPollutionRow(pollution[i], pollution[i] + size_1);
        localPollution.push_back(localPollutionRow);
    }
}

void LifeParallelImplementation::afterLastStep() {

}

void LifeParallelImplementation::exchangeBorders() {
    int prevRank = (processID - 1 + noProcesses) % noProcesses;
    int nextRank = (processID + 1) % noProcesses;

    if (processID == 0) {
        std::vector<int> cellsBorder = getRightBorder(localCells);
        std::vector<int> pollutionBorder = getRightBorder(localCells);

        MPI_Send(cellsBorder.data(), int(cellsBorder.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD);
        MPI_Send(pollutionBorder.data(), int(pollutionBorder.size()), MPI_INT, nextRank, 1, MPI_COMM_WORLD);

        MPI_Recv(cellsBorder.data(), int(cellsBorder.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(pollutionBorder.data(), int(pollutionBorder.size()), MPI_INT, nextRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


        if (int(localCells.size()) == sizeOfPartition) {
            localCells.push_back(cellsBorder);
        } else {
            localCells.back() = cellsBorder;
        }

        if (int(localPollution.size()) == sizeOfPartition) {
            localPollution.push_back(pollutionBorder);
        } else {
            localPollution.back() = pollutionBorder;
        }
    }
    else if (processID == noProcesses - 1) {
        std::vector<int> cellsBorder = getLeftBorder(localCells);
        std::vector<int> pollutionBorder = getLeftBorder(localPollution);

        MPI_Send(cellsBorder.data(), int(cellsBorder.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD);
        MPI_Send(pollutionBorder.data(), int(pollutionBorder.size()), MPI_INT, prevRank, 1, MPI_COMM_WORLD);

        MPI_Recv(cellsBorder.data(), int(cellsBorder.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(pollutionBorder.data(), int(pollutionBorder.size()), MPI_INT, prevRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (int(localCells.size()) == sizeOfPartition) {
            localCells.insert(localCells.begin(), cellsBorder);
        } else {
            localCells.front() = cellsBorder;
        }

        if (int(localPollution.size()) == sizeOfPartition) {
            localPollution.insert(localPollution.begin(), pollutionBorder);
        } else {
            localPollution.front() = pollutionBorder;
        }
    } else {
        std::vector<int> leftCellsBorder = getLeftBorder(localCells);
        std::vector<int> rightCellsBorder = getRightBorder(localCells);

        std::vector<int> leftPollutionBorder = getLeftBorder(localPollution);
        std::vector<int> rightPollutionBorder = getRightBorder(localCells);

        MPI_Send(leftCellsBorder.data(), int(leftCellsBorder.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD);
        MPI_Send(leftPollutionBorder.data(), int(leftPollutionBorder.size()), MPI_INT, prevRank, 1, MPI_COMM_WORLD);
        MPI_Send(rightCellsBorder.data(), int(rightCellsBorder.size()), MPI_INT, nextRank, 2, MPI_COMM_WORLD);
        MPI_Send(rightPollutionBorder.data(), int(rightPollutionBorder.size()), MPI_INT, nextRank, 3, MPI_COMM_WORLD);

        MPI_Recv(leftCellsBorder.data(), int(leftCellsBorder.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(leftPollutionBorder.data(), int(leftPollutionBorder.size()), MPI_INT, nextRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(rightCellsBorder.data(), int(rightCellsBorder.size()), MPI_INT, nextRank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(rightPollutionBorder.data(), int(rightPollutionBorder.size()), MPI_INT, nextRank, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (int(localCells.size()) == sizeOfPartition) {
            localCells.insert(localCells.begin(), rightCellsBorder);
            localCells.push_back(leftCellsBorder);
        } else {
            localCells.front() = rightCellsBorder;
            localCells.back() = leftCellsBorder;
        }

        if (int(localPollution.size()) == sizeOfPartition) {
            localPollution.insert(localPollution.begin(), rightPollutionBorder);
            localPollution.push_back(leftPollutionBorder);
        } else {
            localPollution.front() = rightPollutionBorder;
            localPollution.back() = leftPollutionBorder;
        }
    }
}

std::vector<int> LifeParallelImplementation::getLeftBorder(const std::vector<std::vector<int>>& vectorOfVectors) {
    std::vector<int> leftBorder;
    if (!vectorOfVectors.empty()) {
        const std::vector<int>& firstRow = vectorOfVectors.front();
        leftBorder = firstRow;
    }
    return leftBorder;
}

std::vector<int> LifeParallelImplementation::getRightBorder(const std::vector<std::vector<int>>& vectorOfVectors) {
    std::vector<int> rightBorder;
    if (!vectorOfVectors.empty()) {
        const std::vector<int>& lastRow = vectorOfVectors.back();
        rightBorder = lastRow;
    }
    return rightBorder;
}
