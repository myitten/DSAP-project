#ifndef LOOKAHEAD_PERIMETER_H_INCLUDED
#define LOOKAHEAD_PERIMETER_H_INCLUDED

#include "Agent.h"
#include "OutbreakEngine.h"
#include <climits>
#include <set>
#include <string>
#include <vector>

template <int lookAheadRounds, int perimeterParameter, int infectedParameter, bool placeWall>
class LookAheadAgent : public BaseAgent
{
private:
    std::set<std::pair<int, int>> getCandidates(const Map& m, const std::deque<std::pair<int, int>>& infectedQueue) const
    {
        std::set<std::pair<int, int>> candidates;
        int dx[] = {0, 0, -1, 1};
        int dy[] = {1, -1, 0, 0};

        for(const auto& curPoint : infectedQueue)
        {
            for(int i=0; i<4; i++)
            {
                int nextX = curPoint.first + dx[i];
                int nextY = curPoint.second + dy[i];

                if(m.isValid(nextX, nextY) && m.getStatus(nextX, nextY) == 0)
                {
                    candidates.insert({nextX, nextY});
                }
            }
        }
        return candidates;
    }

    int calculatePerimeter(const Map& m) const
    {
        std::set<std::pair<int, int>> perimeterPoints;
        int dx[] = {0, 0, -1, 1};
        int dy[] = {1, -1, 0, 0};

        for(int y = 0; y < m.getHeight(); y++)
        {
            for(int x = 0; x < m.getWidth(); x++)
            {
                if(m.getStatus(x, y) == 1)
                {
                    for(int i=0; i<4; i++)
                    {
                        int nx = x + dx[i];
                        int ny = y + dy[i];
                        if(m.isValid(nx, ny) && m.getStatus(nx, ny) == 0)
                        {
                            perimeterPoints.insert({nx, ny});
                        }
                    }
                }
            }
        }
        return perimeterPoints.size();
    }

    int evaluate(Map currentMap, OutbreakEngine currentEngine, int roundsLeft) const
    {
        currentEngine.spread(currentMap);
        roundsLeft--;

        if(roundsLeft == 0)
        {
            int currentPerimeter = calculatePerimeter(currentMap);
            int totalInfected = currentMap.infectedCount;

            return (currentPerimeter * perimeterParameter) + (totalInfected * infectedParameter); // evaluate weighted score
        }

        std::set<std::pair<int, int>> nextCandidates = getCandidates(currentMap, currentEngine.getQueue());

        if(nextCandidates.empty() || !placeWall)
        {
            for(int i = 0; i < roundsLeft; i++) {
                currentEngine.spread(currentMap);
            }
            int finalPerimeter = calculatePerimeter(currentMap);
            int finalInfected = currentMap.infectedCount;
            return (finalPerimeter * perimeterParameter) + (finalInfected * infectedParameter);
        }

        int minScore = INT_MAX;
        for(const auto& cand : nextCandidates)
        {
            Map nextMap = currentMap;
            nextMap.setStatus(cand.first, cand.second, 2);

            OutbreakEngine nextEngine;
            nextEngine.setQueue(currentEngine.getQueue());

            int score = evaluate(nextMap, nextEngine, roundsLeft);
            if(score < minScore)
            {
                minScore = score;
            }
        }

        return minScore;
    }

public:
    std::string getAgentName() const override
    {
        std::string wallStr = placeWall ? "Active" : "Passive";
        return "Look-" + std::to_string(lookAheadRounds) +
               " (P:" + std::to_string(perimeterParameter) +
               " I:" + std::to_string(infectedParameter) +
               " Place wall:" + wallStr + ")";
    }

    std::pair<int, int> getBlockPoint(const Map& m, const std::deque<std::pair<int, int>>& infectedQueue) override
    {
        std::set<std::pair<int, int>> candidates = getCandidates(m, infectedQueue);

        if(candidates.empty()) return {-1, -1};

        int minFuturePerimeter = INT_MAX;
        std::pair<int, int> bestPoint = {-1, -1};

        for(const auto& cand : candidates)
        {
            Map simMap = m;
            simMap.setStatus(cand.first, cand.second, 2);

            OutbreakEngine simEngine;
            simEngine.setQueue(infectedQueue);

            int score = evaluate(simMap, simEngine, lookAheadRounds);

            if(score < minFuturePerimeter)
            {
                minFuturePerimeter = score;
                bestPoint = cand;
            }

            else if (score == minFuturePerimeter && bestPoint != std::pair<int, int>{-1, -1})
            {
                int dx[] = {0, 0, -1, 1};
                int dy[] = {1, -1, 0, 0};
                int oldChoke = 0, newChoke = 0;
                for (int i = 0; i < 4; i++) {
                    int nx1 = bestPoint.first + dx[i], ny1 = bestPoint.second + dy[i];
                    int nx2 = cand.first + dx[i], ny2 = cand.second + dy[i];
                    if (!m.isValid(nx1, ny1) || m.getStatus(nx1, ny1) == 2) oldChoke++;
                    if (!m.isValid(nx2, ny2) || m.getStatus(nx2, ny2) == 2) newChoke++;
                }
                if (newChoke > oldChoke) bestPoint = cand;
            }
        }

        return bestPoint;
    }
};

#endif // LOOKAHEAD_PERIMETER_H_INCLUDED
