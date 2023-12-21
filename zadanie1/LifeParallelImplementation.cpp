#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <cmath>
#include <iostream>

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep()
{
    int currentState, currentPollution;

    int start, end;

    if (processID == 0) {
        start = startOfPartition + 1;
        end = endOfPartition;
    }
    else if (processID == noProcesses - 1) {
        start = startOfPartition;
        end = endOfPartition - 1;
    } else {
        start = startOfPartition;
        end = endOfPartition;
    }

    for (int row = start; row < end; row++) {
        for (int col = 1; col < size_1; col++) {
            currentState = cells[row][col];
            currentPollution = pollution[row][col];
            cellsNext[row][col] = rules->cellNextState(currentState, liveNeighbours(row, col), currentPollution);

            int pollutionSumNN = pollution[row + 1][col] + pollution[row - 1][col] + pollution[row][col - 1] + pollution[row][col + 1];
            int pollutionSumNNN = pollution[row - 1][col - 1] + pollution[row - 1][col + 1] + pollution[row + 1][col - 1] + pollution[row + 1][col + 1];
            pollutionNext[row][col] = rules->nextPollution(currentState, currentPollution, pollutionSumNN, pollutionSumNNN);
        }
    }
}

void LifeParallelImplementation::oneStep()
{
    MPI_Barrier(MPI_COMM_WORLD);
    exchangeBorders();
    MPI_Barrier(MPI_COMM_WORLD);
    realStep();
    swapTables();
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

    sizeOfPartition = floor(size / noProcesses);
    startOfPartition = sizeOfPartition * processID;
    endOfPartition = startOfPartition + sizeOfPartition;

    if (size % noProcesses != 0) {
        additionalDataRows = size % noProcesses;
        if (processID == noProcesses - 1) {
            endOfPartition = startOfPartition + sizeOfPartition + additionalDataRows;
        }
    }

    int* flattenedCells = flattenMatrix(cells, size, 0, size);
    MPI_Bcast(&(flattenedCells[0]), size * size, MPI_INT, 0, MPI_COMM_WORLD);
    writeVectorToMatrix(flattenedCells, cells, size, 0, size);
    delete[] flattenedCells;

    int* flattenedPollution = flattenMatrix(pollution, size, 0, size);
    MPI_Bcast(&(flattenedPollution[0]), size * size, MPI_INT, 0, MPI_COMM_WORLD);
    writeVectorToMatrix(flattenedPollution, pollution, size, 0, size);
    delete[] flattenedPollution;
}

void LifeParallelImplementation::afterLastStep() {
    int* cellsToSend;
    int* pollutionToSend;

    if (additionalDataRows == 0) {
        cellsToSend = flattenMatrix(cells, size, startOfPartition, endOfPartition);
        pollutionToSend = flattenMatrix(pollution, size, startOfPartition, endOfPartition);

        int* cellsRecvBuff = new int[size * size];
        MPI_Gather(&(cellsToSend[0]), size * sizeOfPartition, MPI_INT, &(cellsRecvBuff[0]), size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        delete[] cellsToSend;

        int* pollutionRecvBuff = new int[size * size];
        MPI_Gather(&(pollutionToSend[0]), size * sizeOfPartition, MPI_INT, &(pollutionRecvBuff[0]), size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        delete[] pollutionToSend;

        if (processID == 0) {
            writeVectorToMatrix(cellsRecvBuff, cells, size, 0, size);
            writeVectorToMatrix(pollutionRecvBuff, pollution, size, 0, size);
        }

        delete[] cellsRecvBuff;
        delete[] pollutionRecvBuff;

    } else {
        if (processID == noProcesses - 1) {
            cellsToSend = flattenMatrix(cells, size, startOfPartition, endOfPartition - additionalDataRows);
            pollutionToSend = flattenMatrix(pollution, size, startOfPartition, endOfPartition - additionalDataRows);
            
            int* additionalCells = flattenMatrix(cells, size, endOfPartition - additionalDataRows, endOfPartition);
            MPI_Send(&(additionalCells[0]), size * additionalDataRows, MPI_INT, 0, 1, MPI_COMM_WORLD);
            delete[] additionalCells;

            int* additionalPollution = flattenMatrix(pollution, size, endOfPartition - additionalDataRows, endOfPartition);
            MPI_Send(&(additionalPollution[0]), size * additionalDataRows, MPI_INT, 0, 2, MPI_COMM_WORLD);
            delete[] additionalPollution;

        } else {
            cellsToSend = flattenMatrix(cells, size, startOfPartition, endOfPartition);
            pollutionToSend = flattenMatrix(pollution, size, startOfPartition, endOfPartition);
        }

        if (processID == 0) {
            int* additionalCellsRecvBuff = new int[size * additionalDataRows];
            MPI_Recv(&(additionalCellsRecvBuff[0]), size * additionalDataRows, MPI_INT, noProcesses - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            writeVectorToMatrix(additionalCellsRecvBuff, cells, size, size - additionalDataRows, size);
            delete[] additionalCellsRecvBuff;

            int* additionalPollutionRecvBuff = new int[size * additionalDataRows];
            MPI_Recv(&(additionalPollutionRecvBuff[0]), size * additionalDataRows, MPI_INT, noProcesses - 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            writeVectorToMatrix(additionalPollutionRecvBuff, pollution, size, size - additionalDataRows, size);
            delete[] additionalPollutionRecvBuff;
        }
    
        int* cellsRecvBuff = new int[size * size - (additionalDataRows * size)];
        MPI_Gather(&(cellsToSend[0]), size * sizeOfPartition, MPI_INT, &(cellsRecvBuff[0]), size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        delete[] cellsToSend;

        int* pollutionRecvBuff = new int[size * size - (additionalDataRows * size)];
        MPI_Gather(&(pollutionToSend[0]), size * sizeOfPartition, MPI_INT, &(pollutionRecvBuff[0]), size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        delete[] pollutionToSend;

        if (processID == 0) {
            writeVectorToMatrix(cellsRecvBuff, cells, size, 0, size - additionalDataRows);
            writeVectorToMatrix(pollutionRecvBuff, pollution, size, 0, size - additionalDataRows);
        }

        delete[] cellsRecvBuff;
        delete[] pollutionRecvBuff;
    }
}

void LifeParallelImplementation::exchangeBorders() {
    int mergedBordersSize = size * 2;
    
    if (processID == 0) {
        int* cellsBorder = cells[endOfPartition - 1];
        int* pollutionBorder = pollution[endOfPartition - 1];
        int* mergedBorders = mergeArrays(cellsBorder, size, pollutionBorder, size);

        exchangeMergedBorders(mergedBorders, mergedBordersSize, processID + 1, endOfPartition);

    } else if (processID == noProcesses - 1) {
        int* cellsBorder = cells[startOfPartition];
        int* pollutionBorder = pollution[startOfPartition];
        int* mergedBorders = mergeArrays(cellsBorder, size, pollutionBorder, size);

        exchangeMergedBorders(mergedBorders, mergedBordersSize, processID - 1, startOfPartition - 1);

    } else {
        int* cellsLeftBorder = cells[startOfPartition];
        int* pollutionLeftBorder = pollution[startOfPartition];
        int* mergedLeftBorders = mergeArrays(cellsLeftBorder, size, pollutionLeftBorder, size);

        exchangeMergedBorders(mergedLeftBorders, mergedBordersSize, processID - 1, startOfPartition - 1);

        int* cellsRightBorder = cells[endOfPartition - 1];
        int* pollutionRightBorder = pollution[endOfPartition - 1];
        int* mergedRightBorders = mergeArrays(cellsRightBorder, size, pollutionRightBorder, size);

        exchangeMergedBorders(mergedRightBorders, mergedBordersSize, processID + 1, endOfPartition);
    }
}

void LifeParallelImplementation::exchangeMergedBorders(int* mergedBorders, int mergedBorderSize, int sendRecvProcessID, int borderIdx) {
    int* buff = new int[mergedBorderSize];

    MPI_Sendrecv(&(mergedBorders[0]), mergedBorderSize, MPI_INT, sendRecvProcessID, 100,
                 &(buff[0]), mergedBorderSize, MPI_INT, sendRecvProcessID, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    for (int i = 0; i < size; i++) {
        cells[borderIdx][i] = buff[i];
    }
    
    for (int i = 0; i < size; i++) {
            pollution[borderIdx][i] = buff[size + i];
    }
    delete[] mergedBorders;
    delete[] buff;
}

int* LifeParallelImplementation::mergeArrays(const int* array1, int size1, const int* array2, int size2) {
    int sizeMerged = size1 + size2;
    int* mergedArray = new int[sizeMerged];

    for (int i = 0; i < size1; i++) {
        mergedArray[i] = array1[i];
    }
    for (int i = 0; i < size2; i++) {
        mergedArray[size1 + i] = array2[i];
    }

    return mergedArray;
}

int* LifeParallelImplementation::flattenMatrix(int** matrix, int cols, int startRow, int endRow) {
    int rows = endRow - startRow;
    int* flattened = new int[rows * cols];

    int index = 0;
    for (int row = startRow; row < endRow; row++) {
        for (int col = 0; col < cols; col++) {
            flattened[index++] = matrix[row][col];
        }
    }

    return flattened;
}

void LifeParallelImplementation::writeVectorToMatrix(const int* vector, int** matrix, int cols, int startRow, int endRow) {
    int index = 0;
    for (int row = startRow; row < endRow; row++) {
        for (int col = 0; col < cols; col++) {
            matrix[row][col] = vector[index++];
        }
    }
}
