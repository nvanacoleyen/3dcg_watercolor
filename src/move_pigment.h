#pragma once
#include "cell.h"
#include "staggered_grid.h"
#include <vector>


std::vector<std::shared_ptr<Cell>> movePigment(std::vector<std::shared_ptr<Cell>> M, staggered_grid u, staggered_grid v, std::vector<std::shared_ptr<Cell>> Grid);
