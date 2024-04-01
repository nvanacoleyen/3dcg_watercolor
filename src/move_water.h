#include "cell.h"
#include "staggered_grid.h"


void UpdateVelocities(std::vector<Cell>* M, Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p);
void EnforceBoundaryConditions(std::vector<Cell>* M, Staggered_Grid* u, Staggered_Grid* v);

void RelaxDivergence(Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p);

void FlowOutward(std::vector<Cell>* M, std::vector<float>* p);
std::vector<Cell> GaussianCellFilter(std::vector<Cell>* M, int gaussian_radius);