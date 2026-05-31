#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include "Map.h"
#include "OutbreakEngine.h"
#include <deque>
#include <utility>

class BaseAgent {
public:
    virtual ~BaseAgent() = default;
    virtual std::pair<int, int> getBlockPoint(const Map& m, const std::deque<std::pair<int, int>>& infectedQueue) = 0;
    virtual std::string getAgentName() const = 0;
};

#endif // AGENT_H_INCLUDED
