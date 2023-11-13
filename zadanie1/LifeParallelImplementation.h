#ifndef LIFEPARALLELIMPLEMENTATION_H_
#define LIFEPARALLELIMPLEMENTATION_H_

#include "Life.h"

class LifeParallelImplementation : public Life{
private:
    int processID;
    int noProcesses;
    int **bufferLeft;
    int **bufferRight;
    int sizeOfPartition;
    int beginning;
    int end;
    const int CELLS_BORDER_IDX = 0;
    const int POLLUTION_BORDER_IDX = 1;
protected:
    void realStep();
public:
    LifeParallelImplementation();
    int numberOfLivingCells();
    double averagePollution();
    void oneStep();
    void beforeFirstStep();
    void afterLastStep();
    void exchangeBorders();
};

#endif //LIFEPARALLELIMPLEMENTATION_H
