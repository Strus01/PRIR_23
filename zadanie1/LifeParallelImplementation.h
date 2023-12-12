

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
    int* mergeArrays(int* array1, int size1, int* array2, int size2);
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
