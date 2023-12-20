

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
    int* mergeArrays(const int* array1, int size1, const int* array2, int size2);
    int* flattenMatrix(int** matrix, int cols, int startRow, int endRow);
    void writeVectorToMatrix(const int* vector, int** matrix, int cols, int startRow, int endRow);
protected:
    void realStep() override;
public:
    LifeParallelImplementation();
    int numberOfLivingCells() override;
    double averagePollution() override;
    void oneStep() override;
    void beforeFirstStep()override;
    void afterLastStep() override;
};

#endif //LIFEPARALLELIMPLEMENTATION_H
