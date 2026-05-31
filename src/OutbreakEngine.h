#ifndef OUTBREAKENGINE_H_INCLUDED
#define OUTBREAKENGINE_H_INCLUDED

#include "Map.h"
#include <deque>
#include <utility>

class Map;

class OutbreakEngine
{
private:
    std::deque<std::pair<int, int>> infectedQueue;
public:
    void addInfected(Map& m, std::pair<int, int> point);
    void spread(Map& m);
    bool isFinished() const;
    std::deque<std::pair<int, int>> getQueue() {return infectedQueue;};
    void setQueue(const std::deque<std::pair<int, int>>& newQueue)
    {
        this->infectedQueue = newQueue;
    }
};
void OutbreakEngine::addInfected(Map& m, std::pair<int, int> point)
{
    if(m.isValid(point.first, point.second) && m.getStatus(point.first, point.second) == 0)
    {
        m.setStatus(point.first, point.second, 1);
        infectedQueue.push_back(point);
    }
}
void OutbreakEngine::spread(Map& m)
{
    int curNumOfPoint = infectedQueue.size();

    for(int i=0;i<curNumOfPoint;i++)
    {
        std::pair<int ,int> curPoint = infectedQueue.front();
        infectedQueue.pop_front();

        for(int j=0;j<4;j++)
        {
            int x = curPoint.first + dx[j];
            int y = curPoint.second + dy[j];
            this->addInfected(m, {x, y});
        }
    }
}
bool OutbreakEngine::isFinished() const
{
    return infectedQueue.empty();
}


#endif // OUTBREAKENGINE_H_INCLUDED
