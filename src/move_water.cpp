#include "move_water.h"
#include "global_constants.h"


/**
 * std::vector<std::shared_ptr<Cell>> M: I think this is wet area mask M
 * Staggered_Grid u: x velocity
 * Staggered_Grid v: y velocity
 * std::vector<float> p: water_pressure grid
 */
void UpdateVelocities(std::vector<Cell>* M, Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p)
{
	/* I don't really know how to do the first one just yet */
	/* TODO: Add the (u,v) = (u,v) - delta H section */
	for (int j = 0; j < HEIGHT - 1; j++) {
		for (int i = 0; i < WIDTH - 1; i++) {
			glm::vec2 pos = M->at((j * WIDTH) + i).m_position;
			if (pos.x >= WIDTH || pos.y >= HEIGHT) { continue; }

			float u_delta_h = M->at((j * WIDTH) + i).m_height - M->at((j * WIDTH) + i + 1).m_height;
			float v_delta_h = M->at((j * WIDTH) + i).m_height - M->at(((j + 1) * WIDTH) + i).m_height;

			/* We add 1 to make it the boundary from this square to the next one. We wont go out of bounds becuse of the limits in the for loop. */
			u->change_at_pos_by(i + 1, j, u_delta_h);
			v->change_at_pos_by(i, j + 1, v_delta_h);
		}
	}
	//Staggered_Grid u = *u_pointer;
	//Staggered_Grid v = *v_pointer;

	float delta_t = 1 / ceil(std::max(u->max_value(), v->max_value()));
	for (float t = 0.f; t < 1.f; t += delta_t) {

		/* It is very important to remember that u has width + 1, and v has height + 1. When setting the new values for all this we must keep that in mind, along with that when we have position i+0.5 for u, this is ceil + 1*/
		std::vector<float> new_u = u->get_data_values();
		std::vector<float> new_v = v->get_data_values();

		for (int j = 0; j < HEIGHT; j++) {
			for (int i = 0; i < WIDTH; i++) {
				float A = pow(u->get_at_pos(i, j), 2) - pow(u->get_at_pos(i + 1, j), 2) + (u->get_at_pos(i + 0.5f, j - 0.5f) * v->get_at_pos(i + 0.5f, j - 0.5f)) - (u->get_at_pos(i + 0.5f, j + 0.5f) * v->get_at_pos(i + 0.5f, j + 0.5f));
				float B = u->get_at_pos(i + 1.5f, j) + u->get_at_pos(i - 0.5f, j) + u->get_at_pos(i + 0.5f, j + 1.f) + u->get_at_pos(i + 0.5f, j - 1.f) - 4 * u->get_at_pos(i + 0.5f, j);
				new_u[j * (WIDTH + 1) + (i + 1)] = u->get_at_pos(i + 0.5f, j) + delta_t * (A - viscosity * B + get_from_grid(p, WIDTH, HEIGHT, i, j) - get_from_grid(p, WIDTH, HEIGHT, i + 1, j) + viscous_drag * u->get_at_pos(i + 0.5f, j));

				A = pow(v->get_at_pos(i, j), 2) - pow(v->get_at_pos(i, j + 1), 2) + (u->get_at_pos(i + 0.5, j - 0.5) * v->get_at_pos(i + 0.5, j - 0.5)) - (u->get_at_pos(i + 0.5, j + 0.5) * v->get_at_pos(i + 0.5, j + 0.5));
				B = v->get_at_pos(i + 1, j + 0.5f) + v->get_at_pos(i - 0.5, j) + v->get_at_pos(i + 0.5, j + 1) + v->get_at_pos(i + 0.5, j - 1) - 4 * v->get_at_pos(i + 0.5, j);
				new_v[(j + 1) * WIDTH + i] = v->get_at_pos(i, j + 0.5) + delta_t * (A - viscosity * B + get_from_grid(p, WIDTH, HEIGHT, i, j) - get_from_grid(p, WIDTH, HEIGHT, i, j + 1) - viscous_drag * v->get_at_pos(i, j + 0.5));

				/* Ok so my current problem is the positioning of all this, it is unclear if the grid goes to -0.5 or if the borders at position 0 just don't really exist. I think it probably treats position 0 as if it doesn't exist but we still need to discuss it somewhat. */

			}
		}
		u->set_new_data_values(new_u);
		v->set_new_data_values(new_v);

		EnforceBoundaryConditions(M, u, v);
	}

	//u_pointer->set_new_data_values(u.get_data_values());
	//v_pointer->set_new_data_values(v.get_data_values());
}

/**
 * Makes all the boundaries set to 0 when they are outisde of the wet areas.
 */
void EnforceBoundaryConditions(std::vector<Cell>* M, Staggered_Grid* u, Staggered_Grid* v)
{
	int count = 0;
	for (int i = 0; i < M->size(); i++) {
		Cell* cell = &(M->at(i));

		/* If the cell is not wet, we zero the surrounding velocities. */
		if (!cell->is_wet) {
			u->zero_at_pos(cell->m_position.x, cell->m_position.y);
			v->zero_at_pos(cell->m_position.x, cell->m_position.y);
		}
		if (cell->is_wet) {
			count++;
		}
	}
	count++;
}

/**
 * `
 */
void RelaxDivergence(Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p)
{
	int t = 0;


	/* Putting the constants in this function as it isn't really a global constant */
	int N = 50;
	float tolerance = 0.01; /* tau */
	float some_multiplier_idk_man = 0.1; /* xi */

	float delta_max = 0;
	do {
		/* It is very important to remember that u has width + 1, and v has height + 1. When setting the new values for all this we must keep that in mind, along with that when we have position i+0.5 for u, this is ceil + 1*/
		std::vector<float> new_u = u->get_data_values();
		std::vector<float> new_v = v->get_data_values();

		for (int j = 0; j < HEIGHT; j++) {
			for (int i = 0; i < WIDTH; i++) {
				float delta = some_multiplier_idk_man * (u->get_at_pos(i + 0.5f, j) - u->get_at_pos(i - 0.5f, j) + v->get_at_pos(i, j + 0.5) - v->get_at_pos(i, j - 0.5));
				p->at(j * WIDTH + i) += delta;
				new_u.at(j * (WIDTH + 1) + i + 1) += delta;
				new_u.at(j * (WIDTH + 1) + i)     -= delta;
				new_v.at((j + 1) * WIDTH + i)     += delta;
				new_v.at(j * WIDTH + i)           -= delta;
				
				delta_max = std::max(delta_max, delta);
			}
		}
		
		u->set_new_data_values(new_u);
		v->set_new_data_values(new_v);
		t++;
	} while (delta_max >= tolerance && t < N);
}
