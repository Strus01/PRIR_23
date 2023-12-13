#include "LifeParallelImplementation.h"
#include "mpi.h"
#include <cmath>
#include <stdlib.h>
#include <iostream>

LifeParallelImplementation::LifeParallelImplementation() {}

void LifeParallelImplementation::realStep()
{
    int currentState, currentPollution;

    for (int row = startOfPartition + 1; row < endOfPartition - 1; row++) {
        for (int col = 1; col < size_1; col++) {
            currentState = cells[row][col];
            currentPollution = pollution[row][col];

            cellsNext[row][col] = rules->cellNextState(currentState,
                                                       liveNeighbours(row, col),
                                                       currentPollution);
            pollutionNext[row][col] =
                    rules->nextPollution(currentState,
                                         currentPollution,
                                         pollution[row + 1][col] + pollution[row - 1][col] + pollution[row][col - 1] + pollution[row][col + 1],
                                         pollution[row - 1][col - 1] + pollution[row - 1][col + 1] + pollution[row + 1][col - 1] + pollution[row + 1][col + 1]);
        }
    }
}

void LifeParallelImplementation::oneStep()
{
    exchangeBorders();
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

    int *flattenedCells = new int[size * size];
    int *flattenedPollution = new int[size * size];

    if (processID == 0) {
        for (int row = 0; row < size; row++) {
            for (int col = 0; col < size; col++) {
                flattenedCells[row * size + col] = cells[row][col];
                flattenedPollution[row * size + col] = pollution[row][col];
            }
        }
        MPI_Bcast(flattenedCells, size * size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(flattenedPollution, size * size, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Bcast(flattenedCells, size * size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(flattenedPollution, size * size, MPI_INT, 0, MPI_COMM_WORLD);
    }

    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            cells[row][col] = flattenedCells[row * size + col];
            pollution[row][col] = flattenedPollution[row * size + col];
        }
    }
}

void LifeParallelImplementation::afterLastStep() {
    int* cellsToSend = new int[size * sizeOfPartition];
    int* pollutionToSend = new int[size * sizeOfPartition];

    if (additionalDataRows == 0) {
        int* cellsRecvBuff = new int[size * size];
        int* pollutionRecvBuff = new int[size * size];
        int idx = 0;
        for (int row = startOfPartition; row < endOfPartition; row++) {
            for (int col = 0; col < size; col++) {
                cellsToSend[idx * size + col] = cells[row][col];
                pollutionToSend[idx * size + col] = pollution[row][col];
            }
            idx++;
        }
        
        MPI_Gather(cellsToSend, size * sizeOfPartition, MPI_INT, cellsRecvBuff, size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(pollutionToSend, size * sizeOfPartition, MPI_INT, pollutionRecvBuff, size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (processID == 0) {
            for (int row = 0; row < size; row++) {
                for (int col = 0; col < size; col++) {
                    cells[row][col] = cellsRecvBuff[row * size + col];
                    pollution[row][col] = pollutionRecvBuff[row * size + col];
                }
            }
        }
    } else {
        if (processID == noProcesses - 1) {
            int* additionalCells = new int[additionalDataRows * size];
            int* additionalPollution = new int[additionalDataRows * size];

            int idx = 0;
            for (int row = startOfPartition; row < endOfPartition - additionalDataRows; row++) {
                for (int col = 0; col < size; col++) {
                        cellsToSend[idx * size + col] = cells[row][col];
                        pollutionToSend[idx * size + col] = pollution[row][col];
                }
                idx++;
            }
            idx = 0;
            for (int row = endOfPartition - additionalDataRows; row < endOfPartition; row++) {
                for (int col = 0; col < size; col++) {
                        additionalCells[idx * size + col] = cells[row][col];
                        additionalPollution[idx * size + col] = pollution[row][col];
                }
                idx++;
            }
            MPI_Send(additionalCells, additionalDataRows * size, MPI_INT, 0, 50, MPI_COMM_WORLD);
            MPI_Send(additionalPollution, additionalDataRows * size, MPI_INT, 0, 60, MPI_COMM_WORLD);
        } else {
            int idx = 0;
            for (int row = startOfPartition; row < endOfPartition; row++) {
                for (int col = 0; col < size; col++) {
                    cellsToSend[idx * size + col] = cells[row][col];
                    pollutionToSend[idx * size + col] = pollution[row][col];
                }
                idx++;
            }
        }
        int* cellsRecvBuff = new int[size * size - (additionalDataRows * size)];
        int* pollutionRecvBuff = new int[size * size - (additionalDataRows * size)];
        int* additionalCellsRecvBuff;
        int* additionalPollutionRecvBuff;
        
        if (processID == 0) {
            additionalCellsRecvBuff = new int[size * additionalDataRows];
            additionalPollutionRecvBuff = new int[size * additionalDataRows];
            MPI_Recv(additionalCellsRecvBuff, size * additionalDataRows, MPI_INT, noProcesses - 1, 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(additionalPollutionRecvBuff, size * additionalDataRows, MPI_INT, noProcesses - 1, 60, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Gather(cellsToSend, size * sizeOfPartition, MPI_INT, cellsRecvBuff, size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(pollutionToSend, size * sizeOfPartition, MPI_INT, pollutionRecvBuff, size * sizeOfPartition, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (processID == 0) {
            for (int row = 0; row < size - additionalDataRows; row++) {
                for (int col = 0; col < size; col++) {
                        cells[row][col] = cellsRecvBuff[row * size + col];
                        pollution[row][col] = pollutionRecvBuff[row * size + col];
                }
            }
            int idx = 0;
            for (int row = size - additionalDataRows; row < size; row++) {
                for (int col = 0; col < size; col++) {
                        cells[row][col] = additionalCellsRecvBuff[idx * size + col];
                        pollution[row][col] = additionalPollutionRecvBuff[idx * size + col];
                }
                idx++;
            }
        }
    }
}

void LifeParallelImplementation::exchangeBorders() {
    if (processID == 0) {
        int *cellsBorder = cells[endOfPartition];
        int *pollutionBorder = pollution[endOfPartition];
        int sizeCells = sizeof(cellsBorder) / sizeof(cellsBorder[0]);
        int sizePollution = sizeof(pollutionBorder) / sizeof(pollutionBorder[0]);
        int* mergedArray = mergeArrays(cellsBorder, sizeCells, pollutionBorder, sizePollution);

        int* buff = new int[sizeCells + sizePollution];

        MPI_Sendrecv(mergedArray, sizeCells + sizePollution, MPI_INT, processID + 1, 100,
                     buff, sizeCells + sizePollution, MPI_INT, processID + 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < sizeCells + sizePollution; i++) {
            if (i < sizeCells)
                cells[endOfPartition][i] = buff[i];
            else
                pollution[endOfPartition][i] = buff[i];
        }
        delete[] buff;

    } else if (processID == noProcesses - 1) {
        int *cellsBorder = cells[startOfPartition];
        int *pollutionBorder = pollution[startOfPartition];
        int sizeCells = sizeof(cellsBorder) / sizeof(cellsBorder[0]);
        int sizePollution = sizeof(pollutionBorder) / sizeof(pollutionBorder[0]);
        int* mergedArray = mergeArrays(cellsBorder, sizeCells, pollutionBorder, sizePollution);

        int* buff = new int[sizeCells + sizePollution];

        MPI_Sendrecv(mergedArray, sizeCells + sizePollution, MPI_INT, processID - 1, 100,
                     buff, sizeCells + sizePollution, MPI_INT, processID - 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < sizeCells + sizePollution; i++) {
            if (i < sizeCells)
                cells[startOfPartition][i] = buff[i];
            else
                pollution[startOfPartition][i] = buff[i];
        }
        delete[] buff;

    } else {
        int *cellsRightBorder = cells[endOfPartition];
        int *cellsLeftBorder = cells[startOfPartition];
        int *pollutionRightBorder = pollution[endOfPartition];
        int *pollutionLeftBorder = pollution[startOfPartition];
        int sizeCells = sizeof(cellsRightBorder) / sizeof(cellsRightBorder[0]);
        int sizePollution = sizeof(pollutionRightBorder) / sizeof(pollutionRightBorder[0]);
        int* mergedLeftArray = mergeArrays(cellsRightBorder, sizeCells, pollutionLeftBorder, sizePollution);
        int* mergedRightArray = mergeArrays(cellsLeftBorder, sizeCells, pollutionRightBorder, sizePollution);

        int* leftBuff = new int[sizeCells + sizePollution];
        int* rightBuff = new int[sizeCells + sizePollution];

        MPI_Sendrecv(mergedLeftArray, sizeCells + sizePollution, MPI_INT, processID - 1, 100,
                     rightBuff, sizeCells + sizePollution, MPI_INT, processID - 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(mergedRightArray, sizeCells + sizePollution, MPI_INT, processID + 1, 100,
                     leftBuff, sizeCells + sizePollution, MPI_INT, processID + 1, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < sizeCells + sizePollution; i++) {
            if (i < sizeCells)
                cells[endOfPartition][i] = rightBuff[i];
            else
                pollution[endOfPartition][i] = rightBuff[i];
        }

        for (int i = 0; i < sizeCells + sizePollution; i++) {
            if (i < sizeCells)
                cells[startOfPartition][i] = leftBuff[i];
            else
                pollution[startOfPartition][i] = leftBuff[i];
        }
        delete[] rightBuff;
        delete[] leftBuff;
    }
}

int* LifeParallelImplementation::mergeArrays(int* array1, int size1, int* array2, int size2) {
    int sizeMerged = size1 + size2;
    int* mergedArray = new int[sizeMerged];

    for (int i = 0; i < size1; ++i) {
        mergedArray[i] = array1[i];
    }
    for (int i = 0; i < size2; ++i) {
        mergedArray[size1 + i] = array2[i];
    }
    return mergedArray;
}
