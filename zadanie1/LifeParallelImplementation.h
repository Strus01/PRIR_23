#ifndef LIFEPARALLELIMPLEMENTATION_H_
#define LIFEPARALLELIMPLEMENTATION_H_

#include "Life.h"
#include <vector>

class LifeParallelImplementation : public Life{
private:
    int processID;
    int noProcesses;
    std::vector<std::vector<int>> localCells;
    std::vector<std::vector<int>> localPollution;
    int sizeOfPartition;
    int beginning;
    int end;
    std::vector<int> mergeVectors(const std::vector<int>& cellsBorder, const std::vector<int>& pollutionBorder);
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
    void sendBorders();
    void recvBorders();
    std::vector<int> getLeftBorder(const std::vector<std::vector<int>>& vectorOfVectors);
    std::vector<int> getRightBorder(const std::vector<std::vector<int>>& vectorOfVectors);
};

#endif //LIFEPARALLELIMPLEMENTATION_H
