#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <vector>
#include<cmath>

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int currentState, currentPollution;
    for (int row = 1; row < sizeOfPartition - 1; row++) {
        for (int col = 1; col < size_1; col++) {
            currentState = localCells[row][col];
            currentPollution = localPollution[row][col];
            localCells[row][col] = rules->cellNextState(currentState, liveNeighbours(row, col), currentPollution);
            int pollutionSumNN = localPollution[row + 1][col] + localPollution[row - 1][col] + localPollution[row][col - 1] + localPollution[row][col + 1];
            int pollutionSumNNN = localPollution[row - 1][col - 1] + localPollution[row - 1][col + 1] + localPollution[row + 1][col - 1] + localPollution[row + 1][col + 1];
            localPollution[row][col] = rules->nextPollution(currentState, currentPollution, pollutionSumNN, pollutionSumNNN);
        }
    }
}

void LifeParallelImplementation::oneStep() {
    MPI_Barrier(MPI_COMM_WORLD);
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

        std::cout << "Process ID: " << processID << " Hi from afterLastStep()" << std::endl;
        // TODO flatten localCells and localPollution. Or figure out why MPI_Gather is not working
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
    if (processID == 0) {
        std::vector<int> cellsBorder = getRightBorder(localCells);
        std::vector<int> pollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedBorders = mergeVectors(cellsBorder, pollutionBorder);
        std::vector<int> buff(int(mergedBorders.size()));

        MPI_Sendrecv(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, processID + 1, 100,
                     buff.data(), int(mergedBorders.size()), MPI_INT, processID + 1, 100, MPI_COMM_WORLD, NULL);

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
    } else if (processID == noProcesses - 1) {
        std::vector<int> cellsBorder = getRightBorder(localCells);
        std::vector<int> pollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedBorders = mergeVectors(cellsBorder, pollutionBorder);
        std::vector<int> buff(int(mergedBorders.size()));

        MPI_Sendrecv(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, processID - 1, 100,
                     buff.data(), int(mergedBorders.size()), MPI_INT, processID - 1, 100, MPI_COMM_WORLD, NULL);

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
        std::vector<int> leftPollutionBorder = getLeftBorder(localPollution);
        std::vector<int> mergedLeftBorder = mergeVectors(leftCellsBorder, leftPollutionBorder);
        std::vector<int> rightCellsBorder = getRightBorder(localCells);
        std::vector<int> rightPollutionBorder = getRightBorder(localCells);
        std::vector<int> mergedRightBorder = mergeVectors(rightCellsBorder, rightPollutionBorder);
        std::vector<int> leftBuff(int(mergedLeftBorder.size()));
        std::vector<int> rightBuff(int(mergedRightBorder.size()));

        MPI_Sendrecv(mergedLeftBorder.data(), int(mergedLeftBorder.size()), MPI_INT, processID - 1, 100,
                     leftBuff.data(), int(mergedLeftBorder.size()), MPI_INT, processID - 1, 100, MPI_COMM_WORLD, NULL);
        MPI_Sendrecv(mergedRightBorder.data(), int(mergedRightBorder.size()), MPI_INT, processID + 1, 100,
                     rightBuff.data(), int(mergedRightBorder.size()), MPI_INT, processID + 1, 100, MPI_COMM_WORLD, NULL);

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
