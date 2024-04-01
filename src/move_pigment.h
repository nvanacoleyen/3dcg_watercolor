#ifndef MOVE_PIGMENT_H
#define MOVE_PIGMENT_H


#pragma once
#include "cell.h"
#include "staggered_grid.h"
#include <vector>


void movePigment(std::vector<Cell>* Grid, Staggered_Grid* u, Staggered_Grid* v);

#endif // !MOVE_PIGMENT_H
