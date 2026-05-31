#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include <bits/stdc++.h>
#include "Constants.h"
using namespace std;

class Map
{
private:
    int width, height;
    vector<vector<int>> grid; // 0 -> healthy, 1 -> infected, 2 -> wall, 3 -> source
public:
    int infectedCount = 0;
    Map() : width(0), height(0) {}
    Map(int w, int h) : width(w), height(h), grid(h, vector<int>(w, 0)) {}
    Map(int w, int h, vector<vector<int>> g) : width(w), height(h), grid(g) {}
    void setStatus(int x, int y, int status);
    int getStatus(int x, int y) const;
    bool isValid(int x, int y) const;
    void display() const;
    int getWidth() const {return width;}
    int getHeight() const {return height;}
};
void Map::setStatus(int x, int y, int status)
{
    if(!isValid(x, y))
    {
        throw out_of_range("Map coordinates out of range at (" + to_string(x) + ", " + to_string(y) + "). (set)");
    }
    if(grid[y][x] == 0 && status == 1) this->infectedCount++;
    grid[y][x] = status;
}
int Map::getStatus(int x, int y) const
{
    if(!isValid(x, y))
    {
        throw out_of_range("Map coordinates out of range at (" + to_string(x) + ", " + to_string(y) + "). (get)");
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
            else if (status == 3) cout << "V "; // Source
            else cout << ". ";                  // Healthy
        }
        cout << "|" << endl;
    }

    cout << "  +";
    for (int x = 0; x < width; x++) cout << "--";
    cout << "+" << endl;
}

class MapGenerator
{
public:
    static Map generateRandom(int w, int h, double density = 0.2)
    {
        Map newMap(w, h);

        static random_device rd;
        static mt19937 gen(42);
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

#endif // MAP_H_INCLUDED
