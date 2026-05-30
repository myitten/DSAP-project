#include <bits/stdc++.h>
#include "constants.h"

using namespace std;

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
    if(m.isValid(point.first, point.second) && m.getStatus(point.first, point.second) == 0)
    {
        m.setStatus(point.first, point.second, 1);
        infectedQueue.push(point);
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

void roundDisplay(int round)
{
    int digit = 0, r = round;
    while(r != 0)
    {
        digit += 1;
        r /= 10;
    }

    if(WIDTH > 3) for(int i=3+((digit-1)/2);i<WIDTH;i++) cout << "=";
    cout << " Round " << round << " ";
    if(WIDTH > 3) for(int i=3+((digit)/2);i<WIDTH;i++) cout << "=";
    cout << "\n";

}


int main()
{
    try
    {
        cout << "Enter 1 to set map parameters, or enter other to use default settings.";
        string command;
        cin >> command;
        system("cls");
        if(command == "1")
        {
            cout << "Enter following parameters." << "\n";
            cout << "Map width (integer): ";
            cin >> WIDTH;
            cout << "\n" << "Map height (integer): ";
            cin >> HEIGHT;
            cout << "\n" << "Map wall density (double): ";
            cin >> WALL_DENSITY;
        }

        // generate map
        Map myMap = MapGenerator::generateRandom(WIDTH, HEIGHT, WALL_DENSITY);
        OutbreakEngine engine;

        static mt19937 gen(static_cast<unsigned int>(time(0)));

        // set infection source
        // use normal distribution
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



        if(SOURCE_NEIGHBOR_WALL_REMOVE)
        {
            for(int i=0;i<4;i++)
            {
                if(myMap.isValid(startX+dx[i], startY+dy[i])) myMap.setStatus(startX+dx[i], startY+dy[i], 0);
            }
        }

        // pause
        cin.ignore();

        cout << "Press Enter to start the game.";
        cin.get();

        // start spreading
        int round = 1;
        while(true)
        {
            system("cls");
            cout << "\n  ";
            roundDisplay(round);
            myMap.display();

            // place wall
            int wx, wy;
            cout << "Infected: " << myMap.infectedCount << " blocks" << endl;
            cout << "Enter wall coordinates (x y) or (-1 -1) to skip: ";
            if(!(cin >> wx >> wy))
            {
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
