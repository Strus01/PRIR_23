#ifndef LIFEPARALLELIMPLEMENTATION_H_
#define LIFEPARALLELIMPLEMENTATION_H_

#include "Life.h"
#include <vector>

class LifeParallelImplementation : public Life {
private:
    int processID;
    int noProcesses;
    int localBuffSize;
    int sizeOfPartition;
    std::vector<int> localCellsBuff;
    std::vector<int> localPollutionBuff;
    std::vector<std::vector<int>> localPollution;
    std::vector<std::vector<int>> localCells;
    std::vector<int> getLeftBorder(const std::vector<std::vector<int>>& vectorOfVectors);
    std::vector<int> getRightBorder(const std::vector<std::vector<int>>& vectorOfVectors);
    std::vector<int> mergeVectors(const std::vector<int>& cellsBorder, const std::vector<int>& pollutionBorder);
    std::vector<std::vector<int>> vectorToMatrix(const std::vector<int> &inputVector);
    void reshapeBuffs();
    void exchangeBorders();
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
