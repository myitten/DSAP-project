#include <bits/stdc++.h>

using namespace std;

const double WALL_DENSITY = 0.28; // generate map wall density
const int WIDTH = 20, HEIGHT = 20;


class Map
{
private:
    int width, height;
    vector<vector<int>> grid; // 0 -> healthy, 1 -> infected, 2 -> wall
public:
    int infectedCount = 0;
    Map() : width(0), height(0) {}
    Map(int w, int h) : width(w), height(h), grid(h, vector<int>(w, 0)) {}
    Map(int w, int h, vector<vector<int>> g) : width(w), height(h), grid(g) {}
    void setStatus(int x, int y, int status);
    int getStatus(int x, int y) const;
    bool isValid(int x, int y) const;
    void display() const;
};
void Map::setStatus(int x, int y, int status)
{
    if(!isValid(x, y))
    {
        throw out_of_range("Map coordinates out of range at (" + to_string(x) + ", " + to_string(y) + ").");
    }
    if(grid[y][x] == 0 && status == 1) this->infectedCount++;
    grid[y][x] = status;
}
int Map::getStatus(int x, int y) const
{
    if(!isValid(x, y))
    {
        throw out_of_range("Map coordinates out of range at (" + to_string(x) + ", " + to_string(y) + ").");
    }
    return grid[y][x];
}
bool Map::isValid(int x, int y) const
{
    if(x < 0 || x > width - 1) return false;
    if(y < 0 || y > height - 1) return false;
    return true;
}
void Map::display() const
{
    cout << "    ";
    for (int x = 0; x < width; x++)
    {
        cout << x % 10 << " ";
    }
    cout << endl;

    cout << "  +";
    for (int x = 0; x < width; x++) cout << "--";
    cout << "+" << endl;

    for (int y = 0; y < height; y++)
    {
        printf("%2d| ", y);

        for (int x = 0; x < width; x++)
        {
            int status = grid[y][x];
            if (status == 1) cout << "X ";      // Infected
            else if (status == 2) cout << "# "; // Wall
            else cout << ". ";                  // Healthy
        }
        cout << "|" << endl;
    }

    cout << "  +";
    for (int x = 0; x < width; x++) cout << "--";
    cout << "+" << endl;
}

class OutbreakEngine
{
private:
    queue<pair<int, int>> infectedQueue;
public:
    void addInfected(Map& m, pair<int, int> point);
    void spread(Map& m);
    bool isFinished() const;
};
void OutbreakEngine::addInfected(Map& m, pair<int, int> point)
{
    if(m.getStatus(point.first, point.second) == 0)
    {
        m.setStatus(point.first, point.second, 1);
        infectedQueue.push(point);
    }
    else
    {
        // not decided yet
    }
}
void OutbreakEngine::spread(Map& m)
{
    int curNumOfPoint = infectedQueue.size();

    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {1, -1, 0, 0};

    for(int i=0;i<curNumOfPoint;i++)
    {
        pair<int ,int> curPoint = infectedQueue.front();
        infectedQueue.pop();

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

class MapGenerator
{
public:
    static Map generateRandom(int w, int h, double density = 0.2)
    {
        Map newMap(w, h);

        static random_device rd;
        static mt19937 gen(time(nullptr));
        uniform_real_distribution<> dis(0.0, 1.0);

        for(int y = 0; y < h; y++)
        {
            for(int x = 0; x < w; x++)
            {
                if(dis(gen) < density) newMap.setStatus(x, y, 2);
            }
        }
        return newMap;
    }
};


int main()
{
    try
    {
        vector<Map> levelPool;

        // generate map pool (20 here)
        for (int i = 0; i < 20; i++) {
            levelPool.push_back(MapGenerator::generateRandom(WIDTH, HEIGHT, WALL_DENSITY));
        }

        // random take one
        static mt19937 gen(static_cast<unsigned int>(time(0)));
        uniform_int_distribution<> dis(0, 19);

        int selectedIndex = dis(gen);
        Map myMap = levelPool[selectedIndex];

        /*
        can just random one here, but generate a pool to prepare for algorithm test
        */

        cout << "Selected Map Index: " << selectedIndex << endl;

        // set infection source
        // use normal distribution
        OutbreakEngine engine;

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
            if(myMap.isValid(startX, startY)) break;
        }

        myMap.setStatus(startX, startY, 0);
        engine.addInfected(myMap, {startX, startY});


        // pause
        cout << "Press Enter to start the game.";
        cin.get();

        // start spreading
        int round = 1;
        while(true)
        {

            system("cls");

            cout << "\n================ Round " << round << " ================" << endl;
            myMap.display();

            // place wall
            int wx, wy;
            cout << "Infected: " << myMap.infectedCount << " blocks" << endl;
            cout << "Enter wall coordinates (x y) or (-1 -1) to skip: ";
            if(!(cin >> wx >> wy)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }


            if(wx != -1 && wy != -1)
            {
                try
                {
                    myMap.setStatus(wx, wy, 2);
                    cout << "Wall placed at (" << wx << ", " << wy << ")" << endl;
                }
                catch(const out_of_range& e)
                {
                    cout << "Invalid coordinates, skipping wall placement." << endl;
                }
            }

            // virus spread
            cout << "Virus is spreading..." << endl;
            engine.spread(myMap);

            // check game ended condition
            if(engine.isFinished())
            {
                cout << "CONGRATULATIONS! Virus is contained." << endl;
                cout << "Infected: " << myMap.infectedCount << " blocks" << endl;
                break;
            }

            round++;

            // pause
            cout << "Press Enter to continue to next round...";
            cin.ignore();
            cin.get();
        }
    }
    catch(const exception& e)
    {
        cerr << "An error occurred: " << e.what() << endl;
    }

    return 0;
}
