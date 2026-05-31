#include "Agent.h"
#include "GreedyAgent.h"
#include "TimeExpandedMinCutAgent.h"
#include "LookAheadAgent.h"
#include "WeightAgent.h"

// map loader
bool loadCustomMap(const std::string& filename, Map& outMap, std::deque<std::pair<int, int>>& outInfectedQueue) {
    std::ifstream inFile(filename);
    if(!inFile.is_open())
    {
        std::cerr << "Map open failed" << filename << std::endl;
        return false;
    }

    int width = 0, height = 0;
    inFile >> width >> height;
    Map m(width, height);
    outMap = m;
    outInfectedQueue.clear();

    int virusCount = 0;
    inFile >> virusCount;
    for(int i = 0; i < virusCount; i++)
    {
        int vx = 0, vy = 0;
        inFile >> vx >> vy;
        outInfectedQueue.push_back({vx, vy});
    }

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            char symbol;
            inFile >> symbol;
            if (symbol == '#') {
                outMap.setStatus(x, y, 2);
            } else if (symbol == 'V') {
                outMap.setStatus(x, y, 1);
            } else {
                outMap.setStatus(x, y, 0);
            }
        }
    }
    inFile.close();
    return true;
}

int main()
{
    if(LOAD_MAP_MODE)
    {
        try
        {
            Map testMap;
            std::deque<std::pair<int, int>> testQueue;

            if(loadCustomMap("maps/map_0.txt", testMap, testQueue))
            {
                std::cout << "  Map loaded.\n" << std::endl;
                std::cout << "  Initial map:" << std::endl;
                testMap.display();
                OutbreakEngine testEngine;


                BaseAgent* AI = new LookAheadAgent<2, 1, 2, true>();


                testEngine.setQueue(testQueue);
                int round = 0;
                while(!testEngine.isFinished())
                {
                    round++;
                    pair<int,int> wall = AI->getBlockPoint(testMap, testEngine.getQueue());
                    if(wall.first != -1) testMap.setStatus(wall.first, wall.second, 2);
                    testEngine.spread(testMap);
                    std::cout << "\n  Round " << round << std::endl;
                    testMap.display();
                }
                cout << AI->getAgentName() << " in loaded test map ";
                cout << " : Infected: " << testMap.infectedCount << " blocks" << endl;
                delete AI;
            }
            return 0;
        }
        catch(const exception& e)
        {
            cerr << "An error occurred: " << e.what() << endl;
            return 0;
        }
    }
    try
    {
        static mt19937 gen(42);

        // add AI agents
        vector<BaseAgent*> myAIList;

/*
        myAIList.push_back(new GreedyAgent());

        myAIList.push_back(new TimeExpandedMinCutAgent());

        myAIList.push_back(new WeightAgent<1,1,1>());
        myAIList.push_back(new WeightAgent<2,1,1>());
        myAIList.push_back(new WeightAgent<1,2,1>());
        myAIList.push_back(new WeightAgent<1,1,2>());
*/
        //myAIList.push_back(new LookAheadAgent<2,1,1,0>());
        //myAIList.push_back(new LookAheadAgent<2,1,1,1>());

        myAIList.push_back(new LookAheadAgent<2,2,1,1>());
        myAIList.push_back(new LookAheadAgent<2,1,2,1>());

        myAIList.push_back(new LookAheadAgent<2,3,1,1>());
        myAIList.push_back(new LookAheadAgent<2,1,3,1>());

        //myAIList.push_back(new LookAheadAgent<3,1,1,0>());
        //myAIList.push_back(new LookAheadAgent<3,1,1,1>());
/*
        myAIList.push_back(new LookAheadAgent<3,2,1,0>());
        myAIList.push_back(new LookAheadAgent<3,1,2,0>());
        myAIList.push_back(new LookAheadAgent<3,3,1,0>());
        myAIList.push_back(new LookAheadAgent<3,1,3,0>());
*/
        // result file open
        std::ofstream csvFile("results/model_ai_results.csv");
        if(!csvFile.is_open())
        {
            std::cerr << "File open failed" << std::endl;
            return 1;
        }
        csvFile << "Map_ID";
        for(auto* agent : myAIList)
        {
            csvFile << "," << agent->getAgentName();
        }
        csvFile << "\n";

        // init map file open
        std::ofstream mapLogFile("results/all_initial_maps.txt");
        if(!mapLogFile.is_open())
        {
            std::cerr << "File open failed" << std::endl;
            return 1;
        }
        mapLogFile << "==================================================\n";
        mapLogFile << "          ALL MAP INITIAL CONDITION\n";
        mapLogFile << "  ([ # ] = wall, [ V ] = resource, [ . ]=healthy)\n";
        mapLogFile << "==================================================\n\n";

        // generate map pool
        vector<Map> levelPool;
        for(int i = 0; i < MAP_GENERATE_NUM; i++)
        {
            levelPool.push_back(MapGenerator::generateRandom(WIDTH, HEIGHT, WALL_DENSITY));
        }

        // start simulation
        for(int mapIdx = 0; mapIdx < MAP_GENERATE_NUM; mapIdx++)
        {
            csvFile << "Map_" << mapIdx;

            // set source
            double meanX = (WIDTH - 1) / 2.0;
            double meanY = (HEIGHT - 1) / 2.0;
            double standardError = WIDTH / 5.0; // standard error value can be adjusted
            normal_distribution<double> distW(meanX, standardError);
            normal_distribution<double> distH(meanY, standardError);

            int startX, startY;
            while(true)
            {
                startX = round(distW(gen));
                startY = round(distH(gen));
                if(levelPool[mapIdx].isValid(startX, startY)) break;
            }

            levelPool[mapIdx].setStatus(startX, startY, 1);

            // remove walls on neighbors of source(optional)
            if(SOURCE_NEIGHBOR_WALL_REMOVE)
            {
                for(int i=0;i<4;i++)
                {
                    if(levelPool[mapIdx].isValid(startX+dx[i], startY+dy[i])) levelPool[mapIdx].setStatus(startX+dx[i], startY+dy[i], 0);
                }
            }

            // write map log
            mapLogFile << "--------------------------------------------------\n";
            mapLogFile << "  MAP ID: " << mapIdx << "\n";
            mapLogFile << "--------------------------------------------------\n";

            int W = levelPool[mapIdx].getWidth();
            int H = levelPool[mapIdx].getHeight();

            for(int y = 0; y < H; y++)
            {
                for(int x = 0; x < W; x++)
                {
                    int status = levelPool[mapIdx].getStatus(x, y);

                    if(status == 2){
                        mapLogFile << "# ";
                    }
                    else if(status == 1){
                        mapLogFile << "V ";
                    }
                    else{
                        mapLogFile << ". ";
                    }
                }
                mapLogFile << "\n";
            }
            mapLogFile << "\n\n";

            // save single map
            std::string individualMapFilename = "maps/map_" + std::to_string(mapIdx) + ".txt";
            std::ofstream indMapFile(individualMapFilename);

            if (indMapFile.is_open())
            {
                indMapFile << W << " " << H << "\n";

                indMapFile << 1 << "\n";

                indMapFile << startX << " " << startY << "\n";
                for(int y = 0; y < H; y++)
                {
                    for(int x = 0; x < W; x++)
                    {
                        int status = levelPool[mapIdx].getStatus(x, y);
                        if (status == 2)      indMapFile << "# ";
                        else if (status == 1) indMapFile << "V ";
                        else                  indMapFile << ". ";
                    }
                    indMapFile << "\n";
                }
                indMapFile.close();
            }

            // run AIs
            for(BaseAgent* agent : myAIList)
            {
                // copy map
                Map testMap = levelPool[mapIdx];
                OutbreakEngine testEngine;
                for(int y = 0; y < testMap.getHeight(); y++)
                {
                    for(int x = 0; x < testMap.getWidth(); x++)
                    {
                        if(testMap.getStatus(x, y) == 1)
                        {
                            testMap.setStatus(x, y, 0);
                            testEngine.addInfected(testMap, {x, y});
                        }
                    }
                }

                // single game loop
                while(!testEngine.isFinished())
                {
                    pair<int,int> wall = agent->getBlockPoint(testMap, testEngine.getQueue());
                    if(wall.first != -1) testMap.setStatus(wall.first, wall.second, 2);

                    testEngine.spread(testMap);

                    if(MAP_DISPLAY_ALL)
                    {
                        testMap.setStatus(startX, startY, 3);
                        testMap.display();
                    }
                }
                // game ended condition
                cout << agent->getAgentName() << " in map " << mapIdx;
                cout << " : Infected: " << testMap.infectedCount << " blocks" << endl;
                csvFile << "," << testMap.infectedCount;

                if(MAP_DISPLAY)
                {
                    testMap.setStatus(startX, startY, 3);
                    testMap.display();
                }
            }
            csvFile << "\n";
            csvFile << std::flush;
            mapLogFile << std::flush;
        }
        // release
        for(auto agent : myAIList) delete agent;
        csvFile.close();
    }
    catch(const exception& e)
    {
        cerr << "An error occurred: " << e.what() << endl;
    }

    return 0;
}
