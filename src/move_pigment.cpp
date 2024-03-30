#include "move_pigment.h"
#include "global_constants.h"
#include <cmath>

std::vector<std::shared_ptr<Cell>> movePigment(std::vector<std::shared_ptr<Cell>> M, staggered_grid u, staggered_grid v, std::vector<std::shared_ptr<Cell>> Grid)
{

	/* Create new grid of pigments */
	std::vector<std::shared_ptr<Cell>> new_Grid = Grid;
	std::vector<std::shared_ptr<Cell>> current_Grid = Grid;

	float delta_t = 1 / std::ceil(std::max(u.max_value(), v.max_value()));
	for (float t = 0.f; t <= 1.f; t += delta_t) {

		/* Loop through all cells and adjust pigments */
		for (int j = 1; j < HEIGHT - 1; j++) {
			for (int i = 1; i < WIDTH - 1; i++) {

				new_Grid[WIDTH * j + (i+1)]->m_pigmentConc += std::max(0, (u.get_at_pos(i + 0.5, j) * current_Grid[WIDTH * j + i]->m_pigmentConc));
				new_Grid[WIDTH * j + (i-1)]->m_pigmentConc += std::max(0, (-u.get_at_pos(i - 0.5, j) * current_Grid[WIDTH * j + i]->m_pigmentConc));
				new_Grid[WIDTH * (j+1) + i]->m_pigmentConc += std::max(0, (v.get_at_pos(i, j + 0.5) * current_Grid[WIDTH * j + i]->m_pigmentConc));
				new_Grid[WIDTH * (j-1) + i]->m_pigmentConc += std::max(0, (-v.get_at_pos(i, j - 0.5) * current_Grid[WIDTH * j + i]->m_pigmentConc));

				new_Grid[WIDTH * j + i]->m_pigmentConc -= std::max(0, (u.get_at_pos(i + 0.5, j) * current_Grid[WIDTH * j + i]->m_pigmentConc)) +
														  std::max(0, (-u.get_at_pos(i - 0.5, j) * current_Grid[WIDTH * j + i]->m_pigmentConc)) +
														  std::max(0, (v.get_at_pos(i, j + 0.5) * current_Grid[WIDTH * j + i]->m_pigmentConc)) +
														  std::max(0, (-v.get_at_pos(i, j - 0.5) * current_Grid[WIDTH * j + i]->m_pigmentConc));

			}
		}

		current_Grid = new_Grid;

	}

	return new_Grid;
}