#include "move_water.h"
#include "global_constants.h"


/**
 * std::vector<std::shared_ptr<Cell>> M: I think this is wet area mask M
 * staggered_grid u: x velocity
 * staggered_grid v: y velocity
 * std::vector<float> p: water_pressure grid
 */
void UpdateVelocities(std::vector<std::shared_ptr<Cell>> M, staggered_grid u, staggered_grid v, std::vector<float> p)
{
	/* I don't really know how to do the first one just yet */

	float delta_t = 1 / std::max(u.max_value(), v.max_value());
	for (float t = 0.f; t < 1.f; t += delta_t) {

		/* It is very important to remember that u has width + 1, and v has height + 1. When setting the new values for all this we must keep that in mind, along with that when we have position i+0.5 for u, this is ceil + 1*/
		std::vector<float> new_u = u.get_data_values();
		std::vector<float> new_v = v.get_data_values();

		for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				float A = pow(u.get_at_pos(i, j), 2) - pow(u.get_at_pos(i + 1, j), 2) + (u.get_at_pos(i + 0.5, j - 0.5) * v.get_at_pos(i + 0.5, j - 0.5)) - (u.get_at_pos(i + 0.5, j + 0.5) * v.get_at_pos(i + 0.5, j + 0.5));
				float B = u.get_at_pos(i + 1.5, j) + u.get_at_pos(i - 0.5, j) + u.get_at_pos(i + 0.5, j + 1) + u.get_at_pos(i + 0.5, j - 1) - 4 * u.get_at_pos(i + 0.5, j);
				new_u.at(j * (WIDTH + 1) + (i + 1)) = u.get_at_pos(i + 0.5, j) + delta_t * (A - viscosity * B + p.at(j * WIDTH + i) - p.at(j * WIDTH + i + 1) + viscous_drag * u.get_at_pos(i + 0.5f, j));

				A = pow(v.get_at_pos(i, j), 2) - pow(v.get_at_pos(i, j + 1), 2) + (u.get_at_pos(i + 0.5, j - 0.5) * v.get_at_pos(i + 0.5, j - 0.5)) - (u.get_at_pos(i + 0.5, j + 0.5) * v.get_at_pos(i + 0.5, j + 0.5));
				B = v.get_at_pos(i + 1, j + 0.5f) + v.get_at_pos(i - 0.5, j) + v.get_at_pos(i + 0.5, j + 1) + v.get_at_pos(i + 0.5, j - 1) - 4 * v.get_at_pos(i + 0.5, j);
				new_v.at((j + 1) * WIDTH + i) = v.get_at_pos(i, j + 0.5) + delta_t * (A - viscosity * B + p.at(j * (WIDTH + 1) + i) - p.at((j + 1) * (WIDTH + 1) + i) - viscous_drag * v.get_at_pos(i, j + 0.5));

				/* Ok so my current problem is the positioning of all this, it is unclear if the grid goes to -0.5 or if the borders at position 0 just don't really exist. I think it probably treats position 0 as if it doesn't exist but we still need to discuss it somewhat. */

			}
		}
		u.set_new_data_values(new_u);
		v.set_new_data_values(new_v);

		EnforceBoundaryConditions(M, u, v);
	}
}

/**
 * Makes all the boundaries set to 0 when they are outisde of the wet areas.
 */
void EnforceBoundaryConditions(std::vector<std::shared_ptr<Cell>> M, staggered_grid u, staggered_grid v)
{
	for (int i = 0; i < M.size(); i++) {
		std::shared_ptr<Cell> cell = M.at(i);

		/* If the cell is not wet, we zero the surrounding velocities. */
		if (!cell->is_wet) {
			u.zero_at_pos(cell->m_position.x, cell->m_position.y);
			v.zero_at_pos(cell->m_position.x, cell->m_position.y);
		}
	}
}
