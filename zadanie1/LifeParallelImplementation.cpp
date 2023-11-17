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
    MPI_Bcast(&(cells[0][0]), size_1_squared, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&(pollution[0][0]), size_1_squared, MPI_INT, 0, MPI_COMM_WORLD);

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
    sendBorders();
    recvBorders();
}

void LifeParallelImplementation::sendBorders() {
    int prevRank = (processID - 1 + noProcesses) % noProcesses;
    int nextRank = (processID + 1) % noProcesses;

    if (processID == 0) {
        std::vector<int> cellsBorder = getRightBorder(localCells);
        std::vector<int> pollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedBorders = mergeVectors(cellsBorder, pollutionBorder);
        MPI_Send(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD);
    }
    else if (processID == noProcesses - 1) {
        std::vector<int> cellsBorder = getLeftBorder(localCells);
        std::vector<int> pollutionBorder = getLeftBorder(localPollution);
        std::vector<int> mergedBorders = mergeVectors(cellsBorder, pollutionBorder);
        MPI_Send(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD);
    } else {
        std::vector<int> leftCellsBorder = getLeftBorder(localCells);
        std::vector<int> leftPollutionBorder = getLeftBorder(localPollution);
        std::vector<int> mergedLeftBorder = mergeVectors(leftCellsBorder, leftPollutionBorder);
        MPI_Send(mergedLeftBorder.data(), int(mergedLeftBorder.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD);

        std::vector<int> rightCellsBorder = getRightBorder(localCells);
        std::vector<int> rightPollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedRightBorder = mergeVectors(rightCellsBorder, rightPollutionBorder);
        MPI_Send(mergedRightBorder.data(), int(mergedRightBorder.size()), MPI_INT, prevRank, 1, MPI_COMM_WORLD);
    }
}

void LifeParallelImplementation::recvBorders() {
    int prevRank = (processID - 1 + noProcesses) % noProcesses;
    int nextRank = (processID + 1) % noProcesses;

    if (processID == 0) {
        std::vector<int> buff;
        MPI_Recv(buff.data(), int(buff.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> cellsBorder(buff.begin(), buff.begin() + (buff.size() / 2));
        std::vector<int> pollutionBorder(buff.begin() + (buff.size() / 2), buff.end());

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
        std::vector<int> buff;
        MPI_Recv(buff.data(), int(buff.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> cellsBorder(buff.begin(), buff.begin() + (buff.size() / 2));
        std::vector<int> pollutionBorder(buff.begin() + (buff.size() / 2), buff.end());

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
    } else {
        std::vector<int> leftBuff;
        MPI_Recv(leftBuff.data(), int(leftBuff.size()), MPI_INT, prevRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> rightBuff;
        MPI_Recv(rightBuff.data(), int(rightBuff.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> leftCellsBorder(leftBuff.begin(), leftBuff.begin() + (leftBuff.size() / 2));
        std::vector<int> leftPollutionBorder(leftBuff.begin() + (leftBuff.size() / 2), leftBuff.end());
        std::vector<int> rightCellsBorder(rightBuff.begin(), rightBuff.begin() + (rightBuff.size() / 2));
        std::vector<int> rightPollutionBorder(rightBuff.begin() + (rightBuff.size() / 2), rightBuff.end());

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

// możliwe że trzeba przerobić te metody bo co gdy size localArray jest localArray +2 lub +1 wiersze z granic
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

std::vector<int> LifeParallelImplementation::mergeVectors(const std::vector<int> &cellsBorder, const std::vector<int> &pollutionBorder) {
    std::vector<int> mergedBorders;
    mergedBorders.reserve(cellsBorder.size() + pollutionBorder.size());
    mergedBorders.insert(mergedBorders.end(), cellsBorder.begin(), cellsBorder.end());
    mergedBorders.insert(mergedBorders.end(), pollutionBorder.begin(), pollutionBorder.end());
    return mergedBorders;
}
