#ifndef LIFEPARALLELIMPLEMENTATION_H_
#define LIFEPARALLELIMPLEMENTATION_H_

#include "Life.h"
#include <vector>

class LifeParallelImplementation_vector : public Life {
private:
    int processID;
    int noProcesses;
    int localBuffSize;
    int sizeOfPartition;
    int additionalDataSize;
    int additionalDataRows;
    std::vector<int> additionalData;
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
    std::vector<int> mergeVectors(const std::vector<int> &vec1, const std::vector<int> &vec2);
    std::vector<int> flattenMatrix(const std::vector<std::vector<int>> &vec, int &size);
    std::vector<std::vector<int>> vectorToMatrix(const std::vector<int> &vec);
protected:
    void realStep();
    int liveNeighbours(int row, int col);
public:
    LifeParallelImplementation_vector();
    int numberOfLivingCells();
    double averagePollution();
    void oneStep();
    void beforeFirstStep();
    void afterLastStep();
};

#endif //LIFEPARALLELIMPLEMENTATION_H
