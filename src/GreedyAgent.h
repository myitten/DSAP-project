#ifndef GREEDYAGENT_H_INCLUDED
#define GREEDYAGENT_H_INCLUDED

class GreedyAgent : public BaseAgent
{
public:
    string getAgentName() const override {return "Greedy AI";}
    pair<int, int> getBlockPoint(const Map& m, const deque<pair<int, int>>& infectedQueue) override
    {
        set<pair<int, int>> candidates;
        for(const auto& curPoint : infectedQueue)
        {
            for(int i=0;i<4;i++)
            {
                int nextX = curPoint.first + dx[i];
                int nextY = curPoint.second + dy[i];

                if(m.isValid(nextX, nextY) && m.getStatus(nextX, nextY) == 0)
                    candidates.insert({nextX, nextY});
            }
        }

        int maxScore = -1;
        pair<int, int> obj = {-1, -1};

        for(const auto& cand : candidates)
        {
            int currentScore = 0;
            for(int i=0;i<4;i++)
            {
                int neighborX = cand.first + dx[i];
                int neighborY = cand.second + dy[i];

                if(m.isValid(neighborX, neighborY) && m.getStatus(neighborX, neighborY) == 0)
                    currentScore++;
            }

            if(currentScore > maxScore)
            {
                maxScore = currentScore;
                obj = cand;
            }
        }
        return obj;
    }
};

#endif // GREEDYAGENT_H_INCLUDED
