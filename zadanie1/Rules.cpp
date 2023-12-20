/*
 * Rules.cpp
 */

#include "Rules.h"

Rules::Rules() {
}

int Rules::cellNextState(int cellCurrentState, int liveN, int currentPollution)
{
    return (cellCurrentState%2) ^ (liveN%2) ^ (currentPollution%2);
}

int Rules::nextPollution(int cellCurrentState, int currentPollution, int pollutionSumNN, int pollutionSumNNN)
{
    return (cellCurrentState%2) ^ (currentPollution%2) ^ (pollutionSumNN%2) ^ (pollutionSumNNN%2);
}

int Rules::getMaxPollution()
{
    return 0;
}