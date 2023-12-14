

#ifndef LIFEPARALLELIMPLEMENTATION_H
#define LIFEPARALLELIMPLEMENTATION_H

#include "Life.h"

class LifeParallelImplementation: public Life {
private:
    int processID;
    int noProcesses;
    int sizeOfPartition;
    int startOfPartition;
    int endOfPartition;
    int additionalDataRows;
    void exchangeBorders();
    void exchangeMergedBorders(int* mergedBorders, int mergedBorderSize, int sendRecvProcessID, int borderIdx);
    int* mergeArrays(int* array1, int size1, int* array2, int size2);
    int* flattenMatrix(int** matrix, int cols, int startRow, int endRow);
    void writeVectorToMatrix(int* vector, int** matrix, int cols, int startRow, int endRow);
protected:
    void realStep();
public:
    LifeParallelImplementation();
    int numberOfLivingCells();
    double averagePollution();
    void oneStep();
    void beforeFirstStep();
    void afterLastStep();
};

#endif //LIFEPARALLELIMPLEMENTATION_H
