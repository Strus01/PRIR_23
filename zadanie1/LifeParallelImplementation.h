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
    void exchangeBorders();
    void removeBorders();
    void reshapeBuffs();
    std::vector<int> prepareLeftBuffToSend();
    std::vector<int> prepareRightBuffToSend();
    void postprocessLeftRecivedBuff(const std::vector<int> &recivedBuff);
    void postprocessRightRecivedBuff(const std::vector<int> &recivedBuff);
    std::vector<int> mergeVectors(const std::vector<int> &cellsBorder, const std::vector<int> &pollutionBorder);
    std::vector<int> flattenMatrix(const std::vector<std::vector<int>> &vec, int &size);
    std::vector<std::vector<int>> vectorToMatrix(const std::vector<int> &vec);
protected:
    void realStep();
    int liveNeighbours(int row, int col);
public:
    LifeParallelImplementation();
    int numberOfLivingCells();
    double averagePollution();
    void oneStep();
    void beforeFirstStep();
    void afterLastStep();
};

#endif //LIFEPARALLELIMPLEMENTATION_H
