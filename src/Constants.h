#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

// constants
// directions
static const int dx[] = {0, 0, -1, 1};
static const int dy[] = {1, -1, 0, 0};

// model parameters
// map generation
const double WALL_DENSITY = 0.25; // generate map wall density
const int WIDTH = 20, HEIGHT = 20;
const int MAP_GENERATE_NUM = 10;

// remove the walls touched by source
const bool SOURCE_NEIGHBOR_WALL_REMOVE = 1;

// load map mode
const bool LOAD_MAP_MODE = 0;

// display map
const bool MAP_DISPLAY = 0;
const bool MAP_DISPLAY_ALL = 0;

#endif // CONSTANTS_H_INCLUDED
