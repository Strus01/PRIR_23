#include "LifeParallelImplementation_vector.h"
#include "mpi.h"
#include <cmath>
#include <vector>


LifeParallelImplementation_vector::LifeParallelImplementation_vector() {}

void LifeParallelImplementation_vector::realStep() {
    int end;
    std::vector<std::vector<int>> nextCells;
    std::vector<std::vector<int>> nextPollution;
    if (processID == 0) {
        end = sizeOfPartition;
        nextCells.resize(sizeOfPartition + 1, std::vector<int>(size));
        nextPollution.resize(sizeOfPartition + 1, std::vector<int>(size));
    } else if (processID != noProcesses - 1){
        end = sizeOfPartition + 1;
        nextCells.resize(sizeOfPartition + 2, std::vector<int>(size));
        nextPollution.resize(sizeOfPartition + 2, std::vector<int>(size));
    } else {
        if (additionalDataSize == 0) {
            end = sizeOfPartition;
            nextCells.resize(sizeOfPartition + 1, std::vector<int>(size));
            nextPollution.resize(sizeOfPartition + 1, std::vector<int>(size));
        } else {
            end = sizeOfPartition + additionalDataRows;
            nextCells.resize(sizeOfPartition + additionalDataRows + 1, std::vector<int>(size));
            nextPollution.resize(sizeOfPartition + additionalDataRows + 1, std::vector<int>(size));
        }
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
    nextCells.clear();
    nextPollution.clear();
}

int LifeParallelImplementation_vector::liveNeighbours(int row, int col) {
    return localCells[row - 1][col] + localCells[row + 1][col] + localCells[row][col - 1] + localCells[row][col + 1] +
           localCells[row - 1][col - 1] + localCells[row - 1][col + 1] + localCells[row + 1][col - 1] + localCells[row + 1][col + 1];
}

void LifeParallelImplementation_vector::oneStep() {
    exchangeBorders();
    realStep();
    removeBorders();
}

int LifeParallelImplementation_vector::numberOfLivingCells() {
    return sumTable( cells );
}

double LifeParallelImplementation_vector::averagePollution() {
    return (double)sumTable( pollution ) / size_1_squared / rules->getMaxPollution();
}

void LifeParallelImplementation_vector::beforeFirstStep() {
    MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    MPI_Comm_size(MPI_COMM_WORLD, &noProcesses);

    sizeOfPartition = floor(size / noProcesses);
    localBuffSize = floor((size * size) / noProcesses);

    bool additionalRow = false;
    if (size % noProcesses != 0) {
        additionalRow = true;
    }

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
    std::cout << "Process ID: " << processID << " dupa\n";

    if (additionalRow && processID == 0 || processID == noProcesses - 1) {
        additionalDataSize = (size * size) - (localBuffSize * noProcesses);
        additionalDataRows = additionalDataSize / size;
        std::cout << "Process ID: " << processID << " additionalDataSize: " << additionalDataSize << "\n";
        std::cout << "Process ID: " << processID << " additionalDataRows: " << additionalDataRows << "\n";
        if (processID == 0) {
            std::vector<int> sendCellsBuff;
            std::vector<int> sendCPollutionBuff;
            for (int i = additionalDataSize; i < size * size; i++) {
                sendCellsBuff.push_back(flattened_cells[i]);
                sendCPollutionBuff.push_back(flattened_pollution[i]);
            }
            std::vector<int> sendBuffMerged = mergeVectors(sendCellsBuff, sendCPollutionBuff);

            MPI_Send(sendBuffMerged.data(), int(sendBuffMerged.size()), MPI_INT, noProcesses - 1, 50, MPI_COMM_WORLD);
        }
        if (processID == noProcesses - 1) {
            additionalData.resize(additionalDataSize * 2);
            MPI_Recv(additionalData.data(), int(additionalData.size()), MPI_INT, 0, 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    reshapeBuffs();
}

void LifeParallelImplementation_vector::afterLastStep() {
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

    if (processID == noProcesses - 1) {
        std::vector<int> sendCellsBuff;
        std::vector<int> sendPollutionBuff;
        for (int i = localBuffSize; i < localBuffSize + additionalDataSize; i++) {
            sendCellsBuff.push_back(localCellsBuff[i]);
            sendPollutionBuff.push_back(localPollutionBuff[i]);
        }
        std::vector<int> mergedBuff = mergeVectors(sendCellsBuff, sendPollutionBuff);
        MPI_Send(mergedBuff.data(), int(mergedBuff.size()), MPI_INT, noProcesses - 1, 50, MPI_COMM_WORLD);
    }

    if (processID == 0) {
        std::vector<int> recvMergedBuff;
        recvMergedBuff.resize(2 * additionalDataSize);
        MPI_Recv(recvMergedBuff.data(), int(recvMergedBuff.size()), MPI_INT, 0, 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

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

void LifeParallelImplementation_vector::exchangeBorders() {
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

std::vector<int> LifeParallelImplementation_vector::prepareLeftBuffToSend() {
    std::vector<int> leftCellsBorder = localCells.front();
    std::vector<int> leftPollutionBorder = localPollution.front();
    std::vector<int> mergedLeftBorder = mergeVectors(leftCellsBorder, leftPollutionBorder);
    return mergedLeftBorder;
}

std::vector<int> LifeParallelImplementation_vector::prepareRightBuffToSend() {
    std::vector<int> rightCellsBorder = localCells.back();
    std::vector<int> rightPollutionBorder = localPollution.back();
    std::vector<int> mergedRightBorder = mergeVectors(rightCellsBorder, rightPollutionBorder);
    return mergedRightBorder;
}

void LifeParallelImplementation_vector::postprocessLeftRecivedBuff(const std::vector<int> &recivedBuff) {
    std::vector<int> cellsBorder(recivedBuff.begin(), recivedBuff.begin() + (recivedBuff.size() / 2));
    std::vector<int> pollutionBorder(recivedBuff.begin() + (recivedBuff.size() / 2), recivedBuff.end());
    localCells.insert(localCells.begin(), cellsBorder);
    localPollution.insert(localPollution.begin(), pollutionBorder);
}

void LifeParallelImplementation_vector::postprocessRightRecivedBuff(const std::vector<int> &recivedBuff) {
    std::vector<int> cellsBorder(recivedBuff.begin(), recivedBuff.begin() + (recivedBuff.size() / 2));
    std::vector<int> pollutionBorder(recivedBuff.begin() + (recivedBuff.size() / 2), recivedBuff.end());
    localCells.push_back(cellsBorder);
    localPollution.push_back(pollutionBorder);
}

void LifeParallelImplementation_vector::reshapeBuffs() {
    if (additionalDataSize == 0) {
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
    } else {
        if (processID != 0 ) {
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
        } else {
            localCells.resize(sizeOfPartition + additionalDataRows, std::vector<int>(size));
            localPollution.resize(sizeOfPartition + additionalDataRows, std::vector<int>(size));
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
    }
}

void LifeParallelImplementation_vector::removeBorders() {
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

std::vector<int> LifeParallelImplementation_vector::mergeVectors(const std::vector<int> &vec1, const std::vector<int> &vec2) {
    std::vector<int> mergedBorders;
    mergedBorders.reserve(vec1.size() + vec2.size());
    mergedBorders.insert(mergedBorders.end(), vec1.begin(), vec1.end());
    mergedBorders.insert(mergedBorders.end(), vec2.begin(), vec2.end());
    return mergedBorders;
}

std::vector<int> LifeParallelImplementation_vector::flattenMatrix(const std::vector<std::vector<int>> &vec, int &size) {
    std::vector<int> flattened;
    for (const auto &innerVector : vec) {
        flattened.insert(flattened.end(), innerVector.begin(), innerVector.end());
    }
    return flattened;
}

std::vector<std::vector<int>> LifeParallelImplementation_vector::vectorToMatrix(const std::vector<int> &vec) {
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
