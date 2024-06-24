#ifndef MOVE_PIGMENT_H
#define MOVE_PIGMENT_H


#pragma once
#include "cell.h"
#include "staggered_grid.h"
#include "paper/terrain.h"
#include <vector>


void movePigment(std::vector<Cell>* Grid, Staggered_Grid* u, Staggered_Grid* v);
void updateColors(std::vector<paperVertex>& vertices, std::vector<Cell>& Grid, float& brush_radius, GLuint& VBO, const char* colour_mode, Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p);

#endif // !MOVE_PIGMENT_H
