#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <vector>
#include<cmath>
#include <unistd.h>

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int currentState, currentPollution;

    for (int row = beginning; row < end; row++) {
        for (int col = 0; col < size_1; col++) {
            currentState = localCells[row][col];
            currentPollution = localPollution[row][col];
            localCells[row][col] = rules->cellNextState(currentState,
                                                   liveNeighbours(row, col),
                                                   currentPollution);
            localPollution[row][col] = rules->nextPollution(currentState,
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

int LifeParallelImplementation::numberOfLivingCells() {
    return sumTable( cells );
}

double LifeParallelImplementation::averagePollution() {
    return (double)sumTable( pollution ) / size_1_squared / rules->getMaxPollution();
}

void LifeParallelImplementation::beforeFirstStep() {
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);
    sizeOfPartition = size / noProcesses;
    beginning = processID * sizeOfPartition;
    end = beginning + sizeOfPartition;
    localBuffSize = (size * size) / noProcesses;

    localCellsBuff.resize(localBuffSize);
    localPollutionBuff.resize(localBuffSize);

    MPI_Scatter(&(cells[0][0]), localBuffSize, MPI_INT, localCellsBuff.data(), localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&(pollution[0][0]), localBuffSize, MPI_INT, localPollutionBuff.data(), localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);

    reshapeBuffs();
}

void LifeParallelImplementation::afterLastStep() {
    if (processID == 0) {
        std::vector<int> cellsRecvBuff(size * size);
        std::vector<int> pollutionRecvBuff(size * size);

        MPI_Gather(localCells.data(),  localBuffSize, MPI_INT, cellsRecvBuff.data(),  localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(localPollution.data(),  localBuffSize, MPI_INT, pollutionRecvBuff.data(),  localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<std::vector<int>> cellsV = vectorToMatrix(cellsRecvBuff);
        std::vector<std::vector<int>> pollutionV= vectorToMatrix(pollutionRecvBuff);

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                cells[i][j] = cellsV[i][j];
                pollution[i][j] = pollutionV[i][j];
            }
        }
    }
}

void LifeParallelImplementation::exchangeBorders() {
//    MPI_Barrier(MPI_COMM_WORLD);
    sendBorders();
    recvBorders();
}

void LifeParallelImplementation::sendBorders() {
    if (processID == 0) {
        int nextRank = (processID + 1) % noProcesses;
        std::vector<int> cellsBorder = getRightBorder(localCells);
        std::vector<int> pollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedBorders = mergeVectors(cellsBorder, pollutionBorder);
        std::cout << "To ja proces: " << processID << " Jestem tu" << std::endl;
        MPI_Send(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD);
    }
    else if (processID == noProcesses - 1) {
//        int i = 0;
//        while (!i)
//            sleep(5);
        int prevRank = (processID - 1 + noProcesses) % noProcesses;
        std::vector<int> cellsBorder = getLeftBorder(localCells);
        std::vector<int> pollutionBorder = getLeftBorder(localPollution);
        std::vector<int> mergedBorders = mergeVectors(cellsBorder, pollutionBorder);
        std::cout << "To ja proces: " << processID << " Jestem tu" << std::endl;
        MPI_Send(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD);
    } else {
        int prevRank = (processID - 1 + noProcesses) % noProcesses;
        int nextRank = (processID + 1) % noProcesses;
        std::vector<int> leftCellsBorder = getLeftBorder(localCells);
        std::vector<int> leftPollutionBorder = getLeftBorder(localPollution);
        std::vector<int> mergedLeftBorder = mergeVectors(leftCellsBorder, leftPollutionBorder);
        MPI_Send(mergedLeftBorder.data(), int(mergedLeftBorder.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD);

        std::vector<int> rightCellsBorder = getRightBorder(localCells);
        std::vector<int> rightPollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedRightBorder = mergeVectors(rightCellsBorder, rightPollutionBorder);
        MPI_Send(mergedRightBorder.data(), int(mergedRightBorder.size()), MPI_INT, nextRank, 1, MPI_COMM_WORLD);
    }
}

void LifeParallelImplementation::recvBorders() {
    int prevRank = (processID - 1 + noProcesses) % noProcesses;
    int nextRank = (processID + 1) % noProcesses;

    if (processID == 0) {
        std::vector<int> buff(size);
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
        std::vector<int> buff(size);
        MPI_Recv(buff.data(), int(buff.size()), MPI_INT, prevRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> cellsBorder(buff.begin(), buff.begin() + (buff.size() / 2));
        std::vector<int> pollutionBorder(buff.begin() + (buff.size() / 2), buff.end());

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
        std::vector<int> leftBuff(size);
        MPI_Recv(leftBuff.data(), int(leftBuff.size()), MPI_INT, prevRank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> rightBuff(size);
        MPI_Recv(rightBuff.data(), int(rightBuff.size()), MPI_INT, nextRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<int> leftCellsBorder(leftBuff.begin(), leftBuff.begin() + (leftBuff.size() / 2));
        std::vector<int> leftPollutionBorder(leftBuff.begin() + (leftBuff.size() / 2), leftBuff.end());
        std::vector<int> rightCellsBorder(rightBuff.begin(), rightBuff.begin() + (rightBuff.size() / 2));
        std::vector<int> rightPollutionBorder(rightBuff.begin() + (rightBuff.size() / 2), rightBuff.end());

        if (int(localCells.size()) == sizeOfPartition) {
            localCells.insert(localCells.begin(), leftCellsBorder);
            localCells.push_back(rightCellsBorder);
        } else {
            localCells.front() = leftCellsBorder;
            localCells.back() = rightCellsBorder;
        }

        if (int(localPollution.size()) == sizeOfPartition) {
            localPollution.insert(localPollution.begin(), leftPollutionBorder);
            localPollution.push_back(rightPollutionBorder);
        } else {
            localPollution.front() = leftPollutionBorder;
            localPollution.back() = rightPollutionBorder;
        }
    }
}

std::vector<int> LifeParallelImplementation::getLeftBorder(const std::vector<std::vector<int>>& vectorOfVectors) {
    std::vector<int> leftBorder;
    if (!vectorOfVectors.empty()) {
        if (int(vectorOfVectors.size()) == sizeOfPartition) {
            const std::vector<int>& firstRow = vectorOfVectors.front();
            leftBorder = firstRow;
        } else {
            const std::vector<int>& firstRow = vectorOfVectors[1];
            leftBorder = firstRow;
        }
    }
    return leftBorder;
}

std::vector<int> LifeParallelImplementation::getRightBorder(const std::vector<std::vector<int>>& vectorOfVectors) {
    std::vector<int> rightBorder;
    if (!vectorOfVectors.empty()) {
        if (int(vectorOfVectors.size()) == sizeOfPartition) {
            const std::vector<int>& lastRow = vectorOfVectors.back();
            rightBorder = lastRow;
        } else {
            const std::vector<int>& lastRow = vectorOfVectors[vectorOfVectors.size() - 2];
            rightBorder = lastRow;
        }
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

void LifeParallelImplementation::reshapeBuffs() {
    localCells.resize(sizeOfPartition, std::vector<int>(size));
    localPollution.resize(sizeOfPartition, std::vector<int>(size));
    int index = 0;
    for (int i = 0; i < sizeOfPartition; i++) {
        for (int j = 0; j < size; j++) {
            localCells[i][j] = localCellsBuff[index];
            localPollution[i][j] = localPollutionBuff[index];
            index++;
        }
    }
    localCellsBuff.clear();
    localPollutionBuff.clear();
}

std::vector<std::vector<int>> LifeParallelImplementation::vectorToMatrix(const std::vector<int>& inputVector) {
    int matrixSize = static_cast<int>(std::sqrt(inputVector.size()));
    std::vector<std::vector<int>> outputMatrix(matrixSize, std::vector<int>(matrixSize, 0));

    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            int index = i * matrixSize + j;
            if (index < inputVector.size()) {
                outputMatrix[i][j] = inputVector[index];
            }
        }
    }
    return outputMatrix;
}
