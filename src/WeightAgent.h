#ifndef WEIGHTAGENT_H_INCLUDED
#define WEIGHTAGENT_H_INCLUDED
#include "OutbreakEngine.h"
#include <set>
#include <vector>

template <int W_INFECTED, int W_MOBILITY, int W_CHOKE>
class WeightAgent : public BaseAgent {
private:
    // compute obstacle near the point
    int getChokeScore(const Map& m, int x, int y) {
        int obstacleCount = 0;

        for(int i=0;i<4;i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(!m.isValid(nx, ny) || m.getStatus(nx, ny) == 2) obstacleCount++;
        }
        return obstacleCount;
    }
public:
    std::string getAgentName() const override
    {
        return "Heuristic Weight AI (I:"
                     + std::to_string(W_INFECTED) +
               " M:" + std::to_string(W_MOBILITY) +
               " C:" + std::to_string(W_CHOKE) + ")";
    }

    std::pair<int, int> getBlockPoint(const Map& m, const std::deque<std::pair<int, int>>& infectedQueue) override
    {
        // get candidates
        std::set<std::pair<int, int>> candidates;

        for (const auto& curPoint : infectedQueue) {
            for(int i=0;i<4;i++)
            {
                int nextX = curPoint.first + dx[i];
                int nextY = curPoint.second + dy[i];
                if (m.isValid(nextX, nextY) && m.getStatus(nextX, nextY) == 0)
                    candidates.insert({nextX, nextY});
            }
        }

        if (candidates.empty()) return {-1, -1};

        // start simulation
        double minTotalScore = INT_MAX;
        std::pair<int, int> bestPoint = {-1, -1};

        for (const auto& cand : candidates)
        {
            Map simMap = m;
            simMap.setStatus(cand.first, cand.second, 2);

            OutbreakEngine simEngine;
            simEngine.setQueue(infectedQueue);

            int lookAheadRounds = 2;
            for (int r = 0; r < lookAheadRounds; r++) {
                simEngine.spread(simMap);
            }

            // compute weighted score
            int totalInfectedInFuture = 0;
            int totalMobility = 0;

            for(int y=0;y<simMap.getHeight();y++)
            {
                for(int x=0;x<simMap.getWidth();x++)
                {
                    if (simMap.getStatus(x, y) == 1)
                    {
                        totalInfectedInFuture++;
                        for (int i = 0; i < 4; i++) {
                            int nx = x + dx[i];
                            int ny = y + dy[i];
                            if (simMap.isValid(nx, ny) && simMap.getStatus(nx, ny) == 0) {
                                totalMobility++;
                            }
                        }
                    }
                }
            }

            int chokeIndex = getChokeScore(m, cand.first, cand.second);

            double currentUniverseScore = (W_INFECTED * totalInfectedInFuture)
                                        + (W_MOBILITY * totalMobility)
                                        - (W_CHOKE * chokeIndex);

            if(currentUniverseScore < minTotalScore)
            {
                minTotalScore = currentUniverseScore;
                bestPoint = cand;
            }
        }

        return bestPoint;
    }
};

#endif // WEIGHTAGENT_H_INCLUDED
