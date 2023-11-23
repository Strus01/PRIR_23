#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <vector>


LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep() {
    int end;
    std::vector<std::vector<int>> nextCells;
    std::vector<std::vector<int>> nextPollution;
    if (processID == 0 || processID == noProcesses - 1) {
        end = sizeOfPartition;
        nextCells.resize(sizeOfPartition + 1, std::vector<int>(size));
        nextPollution.resize(sizeOfPartition + 1, std::vector<int>(size));
    } else {
        end = sizeOfPartition + 1;
        nextCells.resize(sizeOfPartition + 2, std::vector<int>(size));
        nextPollution.resize(sizeOfPartition + 2, std::vector<int>(size));
    }

    int currentState, currentPollution;
    for (int row = 1; row < end; row++) {
        for (int col = 1; col < size_1; col++) {
            currentState = localCells[row][col];
            currentPollution = localPollution[row][col];
            nextCells[row][col] = rules->cellNextState(currentState, liveNeighbours(row, col), currentPollution);
            int pollutionSumNN = localPollution[row + 1][col] + localPollution[row - 1][col] + localPollution[row][col - 1] + localPollution[row][col + 1];
            int pollutionSumNNN = localPollution[row - 1][col - 1] + localPollution[row - 1][col + 1] + localPollution[row + 1][col - 1] + localPollution[row + 1][col + 1];
            nextPollution[row][col] = rules->nextPollution(currentState, currentPollution, pollutionSumNN, pollutionSumNNN);
        }
    }

    localCells = nextCells;
    localPollution = nextPollution;
}

int LifeParallelImplementation::liveNeighbours(int row, int col) {
    return localCells[row - 1][col] + localCells[row + 1][col] + localCells[row][col - 1] + localCells[row][col + 1] +
           localCells[row - 1][col - 1] + localCells[row - 1][col + 1] + localCells[row + 1][col - 1] + localCells[row + 1][col + 1];
}

void LifeParallelImplementation::oneStep() {
    MPI_Barrier(MPI_COMM_WORLD);
    exchangeBorders();
    realStep();
    removeBorders();
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

    std::vector<int> flattened_cells;
    std::vector<int> flattened_pollution;
    if (processID == 0) {
        flattened_cells.reserve(size * size);
        flattened_pollution.reserve(size * size);
        for (int i = 0; i < size; i++) {
            flattened_cells.insert(flattened_cells.end(), cells[i], cells[i] + size);
            flattened_pollution.insert(flattened_pollution.end(), pollution[i], pollution[i] + size);
        }
    }

    localCellsBuff.resize(localBuffSize);
    localPollutionBuff.resize(localBuffSize);
    MPI_Scatter(flattened_cells.data(), localBuffSize, MPI_INT, localCellsBuff.data(), localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(flattened_pollution.data(), localBuffSize, MPI_INT, localPollutionBuff.data(), localBuffSize, MPI_INT, 0, MPI_COMM_WORLD);
    reshapeBuffs();
}

void LifeParallelImplementation::afterLastStep() {
    localCellsBuff = flattenMatrix(localCells, localBuffSize);
    localPollutionBuff = flattenMatrix(localPollution, localBuffSize);

    std::vector<int> cellsRecvBuff;
    std::vector<int> pollutionRecvBuff;

    if (processID == 0) {
        cellsRecvBuff.resize(size * size);
        pollutionRecvBuff.resize(size * size);
    }
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
                     buff.data(), int(mergedBorders.size()), MPI_INT, processID + 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        postprocessRightRecivedBuff(buff);
    } else if (processID == noProcesses - 1) {
        std::vector<int> mergedBorders = prepareLeftBuffToSend();
        std::vector<int> buff(int(mergedBorders.size()));

        MPI_Sendrecv(mergedBorders.data(), int(mergedBorders.size()), MPI_INT, processID - 1, 100,
                     buff.data(), int(mergedBorders.size()), MPI_INT, processID - 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        postprocessLeftRecivedBuff(buff);
    } else {
        std::vector<int> mergedLeftBorder = prepareLeftBuffToSend();
        std::vector<int> mergedRightBorder = prepareRightBuffToSend();

        std::vector<int> leftBuff(int(mergedLeftBorder.size()));
        std::vector<int> rightBuff(int(mergedRightBorder.size()));

        MPI_Sendrecv(mergedLeftBorder.data(), int(mergedLeftBorder.size()), MPI_INT, processID - 1, 100,
                     leftBuff.data(), int(mergedLeftBorder.size()), MPI_INT, processID - 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(mergedRightBorder.data(), int(mergedRightBorder.size()), MPI_INT, processID + 1, 100,
                     rightBuff.data(), int(mergedRightBorder.size()), MPI_INT, processID + 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        postprocessLeftRecivedBuff(leftBuff);
        postprocessRightRecivedBuff(rightBuff);
    }
}

std::vector<int> LifeParallelImplementation::prepareLeftBuffToSend() {
    std::vector<int> leftCellsBorder = localCells.front();
    std::vector<int> leftPollutionBorder = localPollution.front();
    std::vector<int> mergedLeftBorder = mergeVectors(leftCellsBorder, leftPollutionBorder);
    return mergedLeftBorder;
}

std::vector<int> LifeParallelImplementation::prepareRightBuffToSend() {
    std::vector<int> rightCellsBorder = localCells.back();
    std::vector<int> rightPollutionBorder = localPollution.back();
    std::vector<int> mergedRightBorder = mergeVectors(rightCellsBorder, rightPollutionBorder);
    return mergedRightBorder;
}

void LifeParallelImplementation::postprocessLeftRecivedBuff(const std::vector<int> &recivedBuff) {
    std::vector<int> cellsBorder(recivedBuff.begin(), recivedBuff.begin() + (recivedBuff.size() / 2));
    std::vector<int> pollutionBorder(recivedBuff.begin() + (recivedBuff.size() / 2), recivedBuff.end());
    localCells.insert(localCells.begin(), cellsBorder);
    localPollution.insert(localPollution.begin(), pollutionBorder);
}

void LifeParallelImplementation::postprocessRightRecivedBuff(const std::vector<int> &recivedBuff) {
    std::vector<int> cellsBorder(recivedBuff.begin(), recivedBuff.begin() + (recivedBuff.size() / 2));
    std::vector<int> pollutionBorder(recivedBuff.begin() + (recivedBuff.size() / 2), recivedBuff.end());
    localCells.push_back(cellsBorder);
    localPollution.push_back(pollutionBorder);
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

std::vector<int> LifeParallelImplementation::flattenMatrix(const std::vector<std::vector<int>> &vec, int &size) {
    std::vector<int> flattened;
    for (const auto &innerVector : vec) {
        flattened.insert(flattened.end(), innerVector.begin(), innerVector.end());
    }
    return flattened;
}

std::vector<std::vector<int>> LifeParallelImplementation::vectorToMatrix(const std::vector<int> &vec) {
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
