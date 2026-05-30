#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

// constants
// directions
static const int dx[] = {0, 0, -1, 1};
static const int dy[] = {1, -1, 0, 0};

// model parameters
// map generation
double WALL_DENSITY = 0.25; // generate map wall density
int WIDTH = 20, HEIGHT = 20;

// remove the walls touched by source
const bool SOURCE_NEIGHBOR_WALL_REMOVE = 1;

#endif // CONSTANTS_H_INCLUDED
