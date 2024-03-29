#include "cell.h"
#include "staggered_grid.h"


void UpdateVelocities(std::vector<std::shared_ptr<Cell>> M, staggered_grid u, staggered_grid v, std::vector<float> p);
void EnforceBoundaryConditions(std::vector<std::shared_ptr<Cell>> M, staggered_grid u, staggered_grid v);