#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <vector>


LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int end;
    if (processID == 0 || processID == noProcesses - 1) {
        end = sizeOfPartition;
    } else {
        end = sizeOfPartition + 1;
    }

    int currentState, currentPollution;
    for (int row = 1; row < end; row++) {
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
    removeBorders();
    localCellsBuff = flattenMatrix(localCells);
    localPollutionBuff = flattenMatrix(localPollution);

    std::vector<int> cellsRecvBuff(size * size);
    std::vector<int> pollutionRecvBuff(size * size);

    MPI_Gather(localCellsBuff.data(),  localBuffSize, MPI_INT, cellsRecvBuff.data(),  localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(localPollutionBuff.data(),  localBuffSize, MPI_INT, pollutionRecvBuff.data(),  localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);

    if (processID == 0) {
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
        std::vector<int> mergedBorders = prepareRightBuffToSend();
        std::vector<int> buff(int(mergedBorders.size()));

        MPI_Sendrecv(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, processID + 1, 100,
                     buff.data(), int(mergedBorders.size()), MPI_INT, processID + 1, 100, MPI_COMM_WORLD, NULL);

        postprocessRightRecivedBuff(buff);

    } else if (processID == noProcesses - 1) {
        std::vector<int> mergedBorders = prepareLeftBuffToSend();
        std::vector<int> buff(int(mergedBorders.size()));

        MPI_Sendrecv(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, processID - 1, 100,
                     buff.data(), int(mergedBorders.size()), MPI_INT, processID - 1, 100, MPI_COMM_WORLD, NULL);

        postprocessLeftRecivedBuff(buff);

    } else {
        std::vector<int> mergedLeftBorder = prepareLeftBuffToSend();
        std::vector<int> mergedRightBorder = prepareRightBuffToSend();

        std::vector<int> leftBuff(int(mergedLeftBorder.size()));
        std::vector<int> rightBuff(int(mergedRightBorder.size()));

        MPI_Sendrecv(mergedLeftBorder.data(), int(mergedLeftBorder.size()), MPI_INT, processID - 1, 100,
                     leftBuff.data(), int(mergedLeftBorder.size()), MPI_INT, processID - 1, 100, MPI_COMM_WORLD, NULL);
        MPI_Sendrecv(mergedRightBorder.data(), int(mergedRightBorder.size()), MPI_INT, processID + 1, 100,
                     rightBuff.data(), int(mergedRightBorder.size()), MPI_INT, processID + 1, 100, MPI_COMM_WORLD, NULL);

        postprocessRecivedBuffs(leftBuff, rightBuff);
    }
}

std::vector<int> LifeParallelImplementation::prepareLeftBuffToSend() {
    std::vector<int> leftCellsBorder = getLeftBorder(localCells);
    std::vector<int> leftPollutionBorder = getLeftBorder(localPollution);
    std::vector<int> mergedLeftBorder = mergeVectors(leftCellsBorder, leftPollutionBorder);
    return mergedLeftBorder;
}

std::vector<int> LifeParallelImplementation::prepareRightBuffToSend() {
    std::vector<int> rightCellsBorder = getRightBorder(localCells);
    std::vector<int> rightPollutionBorder = getRightBorder(localCells);
    std::vector<int> mergedRightBorder = mergeVectors(rightCellsBorder, rightPollutionBorder);
    return mergedRightBorder;
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

void LifeParallelImplementation::postprocessLeftRecivedBuff(const std::vector<int> &recivedBuff) {
    std::vector<int> cellsBorder(recivedBuff.begin(), recivedBuff.begin() + (recivedBuff.size() / 2));
    std::vector<int> pollutionBorder(recivedBuff.begin() + (recivedBuff.size() / 2), recivedBuff.end());

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
}

void LifeParallelImplementation::postprocessRightRecivedBuff(const std::vector<int> &recivedBuff) {
    std::vector<int> cellsBorder(recivedBuff.begin(), recivedBuff.begin() + (recivedBuff.size() / 2));
    std::vector<int> pollutionBorder(recivedBuff.begin() + (recivedBuff.size() / 2), recivedBuff.end());

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

void LifeParallelImplementation::postprocessRecivedBuffs(const std::vector<int> &leftRecivedBuff, const std::vector<int> &rightRecivedBuff) {
    std::vector<int> leftCellsBorder(leftRecivedBuff.begin(), leftRecivedBuff.begin() + (leftRecivedBuff.size() / 2));
    std::vector<int> leftPollutionBorder(leftRecivedBuff.begin() + (leftRecivedBuff.size() / 2), leftRecivedBuff.end());
    std::vector<int> rightCellsBorder(rightRecivedBuff.begin(), rightRecivedBuff.begin() + (rightRecivedBuff.size() / 2));
    std::vector<int> rightPollutionBorder(rightRecivedBuff.begin() + (rightRecivedBuff.size() / 2), rightRecivedBuff.end());

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

void LifeParallelImplementation::removeBorders() {
    if (processID == 0) {
        localCells.pop_back();
        localPollution.pop_back();
    } else if (processID == noProcesses - 1) {
        localCells.erase(localCells.begin());
        localPollution.erase(localPollution.begin());
    } else {
        localCells.pop_back();
        localPollution.pop_back();
        localCells.erase(localCells.begin());
        localPollution.erase(localPollution.begin());
    }
}

std::vector<int> LifeParallelImplementation::mergeVectors(const std::vector<int> &cellsBorder, const std::vector<int> &pollutionBorder) {
    std::vector<int> mergedBorders;
    mergedBorders.reserve(cellsBorder.size() + pollutionBorder.size());
    mergedBorders.insert(mergedBorders.end(), cellsBorder.begin(), cellsBorder.end());
    mergedBorders.insert(mergedBorders.end(), pollutionBorder.begin(), pollutionBorder.end());
    return mergedBorders;
}


std::vector<int> LifeParallelImplementation::flattenMatrix(const std::vector<std::vector<int>> &vec) {
    std::vector<int> flattened;
    for (const auto& innerVector : vec) {
        flattened.insert(flattened.end(), innerVector.begin(), innerVector.end());
    }
    return flattened;
}

std::vector<std::vector<int>> LifeParallelImplementation::vectorToMatrix(const std::vector<int>& vec) {
    std::vector<std::vector<int>> outputMatrix(size, std::vector<int>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int index = i * size + j;
            if (index < vec.size()) {
                outputMatrix[i][j] = vec[index];
            }
        }
    }
    return outputMatrix;
}
