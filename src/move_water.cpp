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

			/* If out of bounds; continue */
			if (pos.x >= WIDTH || pos.y >= HEIGHT) { continue; }

			float u_delta_h = M->at((j * (WIDTH + 1)) + i + 1).m_position.z 
							- M->at((j * (WIDTH + 1)) + i).m_position.z;
			float v_delta_h = M->at(((j + 1) * WIDTH) + i).m_position.z 
							- M->at((j * WIDTH) + i).m_position.z;

			/* We add 1 to make it the boundary from this square to the next one. We wont go out of bounds becuse of the limits in the for loop. */
			u->change_at_pos_by(i + 1, j, -u_delta_h);
			v->change_at_pos_by(i, j + 1, -v_delta_h);
		}
	}
	
	/* It is very important to remember that u has width + 1, and v has height + 1. 
	When setting the new values for all this we must keep that in mind, 
	along with that when we have position i+0.5 for u, this is ceil + 1*/
	/*std::vector<float> new_u = u->get_data_values();
	std::vector<float> new_v = v->get_data_values();*/

	/* delta_t = size of 1 grid cell*/
	float delta_t = 1 / std::max(1.f, ceil(std::max(u->max_value(), v->max_value())));

	for (float t = 0.f; t < 1.f; t += delta_t) {

		std::vector<float> new_u = u->get_data_values();
		std::vector<float> new_v = v->get_data_values();


		for (int j = 0; j < HEIGHT; j++) {
			for (int i = 0; i < WIDTH; i++) {
				float A = pow(u->get_at_pos(i, j), 2)
					- pow(u->get_at_pos(i + 1, j), 2)
					+ (u->get_at_pos(i + 0.5f, j - 0.5f) * v->get_at_pos(i + 0.5f, j - 0.5f))
					- (u->get_at_pos(i + 0.5f, j + 0.5f) * v->get_at_pos(i + 0.5f, j + 0.5f));
				float B = u->get_at_pos(i + 1.5f, j)
					+ u->get_at_pos(i - 0.5f, j)
					+ u->get_at_pos(i + 0.5f, j + 1.f)
					+ u->get_at_pos(i + 0.5f, j - 1.f)
					- 4 * u->get_at_pos(i + 0.5f, j);
				
				new_u[j * (WIDTH + 1) + (i + 1)] = u->get_at_pos(i + 0.5f, j) 
					+ delta_t * (A - (viscosity * B) + get_from_grid(p, WIDTH, HEIGHT, i, j) - get_from_grid(p, WIDTH, HEIGHT, i + 1, j) - (viscous_drag * u->get_at_pos(i + 0.5f, j)));

				new_u[j * (WIDTH + 1) + (i + 1)] = std::max(-25.f, std::min(25.f, new_u[j * (WIDTH + 1) + (i + 1)]));

				A = pow(v->get_at_pos(i, j), 2)
					- pow(v->get_at_pos(i, j + 1), 2) 
					+ (u->get_at_pos(i - 0.5, j + 0.5) * v->get_at_pos(i - 0.5, j + 0.5)) 
					- (u->get_at_pos(i + 0.5, j + 0.5) * v->get_at_pos(i + 0.5, j + 0.5));
				B = v->get_at_pos(i + 1, j + 0.5f) 
					+ v->get_at_pos(i - 1, j + 0.5) 
					+ v->get_at_pos(i, j + 1.5) 
					+ v->get_at_pos(i, j - 0.5) 
					- 4 * v->get_at_pos(i, j + 0.5);

				new_v[(j + 1) * WIDTH + i] = v->get_at_pos(i, j + 0.5) 
					+ delta_t * (A - (viscosity * B) + get_from_grid(p, WIDTH, HEIGHT, i, j) - get_from_grid(p, WIDTH, HEIGHT, i, j + 1) - (viscous_drag * v->get_at_pos(i, j + 0.5)));

				new_v[(j + 1) * WIDTH + i] = std::max(-25.f, std::min(25.f, new_v[(j + 1) * WIDTH + i]));
			}
		}
		u->set_new_data_values(new_u);
		v->set_new_data_values(new_v);
		EnforceBoundaryConditions(M, u, v);
	}
}

/**
 * Makes all the boundaries set to 0 when they are outisde of the wet areas.
 */
void EnforceBoundaryConditions(std::vector<Cell>* M, Staggered_Grid* u, Staggered_Grid* v)
{
	for (int i = 0; i < M->size(); i++) {
		Cell* cell = &(M->at(i));

		/* If the cell is not wet, we zero the surrounding velocities. We have a buffer for what exactly is 'not wet'. */
		if (cell->m_waterConc < minimum_wetness) {
			u->zero_at_pos(cell->m_position.x, cell->m_position.y);
			v->zero_at_pos(cell->m_position.x, cell->m_position.y);
		}
	}
}

/**
 * `
 */
void RelaxDivergence(Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p)
{
	int t = 0;


	/* Putting the constants in this function as it isn't really a global constant */
	int N = 25;
	float tolerance = 0.01; /* tau */
	
	/* Being honest I don't really know if this one had an affect so ill just leave both versions as comments so we can see it later or something if it is a concern. */
	//float some_multiplier_idk_man = 0.1; /* xi */
	float some_multiplier_idk_man = -0.1; /* xi */

	float delta_max = 0;
	
	/* Putting these out of the loop for efficiency */
	std::vector<float> new_u = u->get_data_values();
	std::vector<float> new_v = v->get_data_values();
	do {
		/* It is very important to remember that u has width + 1, and v has height + 1. When setting the new values for all this we must keep that in mind, along with that when we have position i+0.5 for u, this is ceil + 1*/
		delta_max = 0;

		for (int j = 0; j < HEIGHT - 1; j++) {
			for (int i = 0; i < WIDTH - 1; i++) {
				float delta = some_multiplier_idk_man * (u->get_at_pos(i + 0.5f, j) - u->get_at_pos(i - 0.5f, j) + v->get_at_pos(i, j + 0.5f) - v->get_at_pos(i, j - 0.5f));
				p->at(j * WIDTH + i) += delta;
				new_u.at(j * (WIDTH + 1) + i + 1) += delta;  /* u[i + 0.5, j] */
				new_u.at(j * (WIDTH + 1) + i)     -= delta;  /* u[i - 0.5, j] */
				new_v.at((j + 1) * WIDTH + i)     += delta;  /* v[i, j + 0.5] */
				new_v.at(j * WIDTH + i)           -= delta;  /* v[i, j - 0.5] */
				
				delta_max = std::max(delta_max, abs(delta));
			}
		}
		
		u->set_new_data_values(new_u);
		v->set_new_data_values(new_v);
		t++;
	} while (delta_max > tolerance && t < N);
}

/**
 * Removes some water and makes the pigment flow toward the edges of the wet area
 */
void FlowOutward(std::vector<Cell>* M, std::vector<float>* p)
{
	/* Constants for the function */
	float eta = 0.05f;
	int kernel_size = 5;

	std::vector<Cell> gaussianGrid = GaussianCellFilter(M, kernel_size);

	for (int j = 0; j < HEIGHT; j++) {
		for (int i = 0; i < WIDTH; i++) {
			int location = j * WIDTH + i;
			p->at(location) = p->at(location) - (eta * (1 - gaussianGrid.at(location).m_waterConc) * M->at(location).m_waterConc);
		}
	}
}

/**
 * Makes a gaussian cell filter for the water concentration of the wet mask of the cells
 */
std::vector<Cell> GaussianCellFilter(std::vector<Cell>* M, int gaussian_radius)
{
	std::vector<Cell> res;
	for (int j = 0; j < HEIGHT; j++) {
		for (int i = 0; i < WIDTH; i++) {
			Cell current_cell = M->at(j * WIDTH + i);
			float current_water_conc = 0.f;
			int size_of_grid = 0;
			for (int y = j - (gaussian_radius / 2); y <= j + (gaussian_radius / 2); y++) {
				for (int x = i - (gaussian_radius / 2); x <= i + (gaussian_radius / 2); x++) {
					int local_x = x;
					int local_y = y;

					/* If the gaussian goes out of range, we edge extend it essentially. */
					if (y < 0) { local_y = 0; }  
					else if (y >= HEIGHT) { local_y = HEIGHT - 1; }

					if (x < 0) { local_x = 0; }
					else if (x >= WIDTH) { local_x = WIDTH - 1; }

					current_water_conc += M->at(local_y * WIDTH + local_x).m_waterConc;
					size_of_grid++;
				}
			}

			/* Just an additional safeguard */
			if (size_of_grid == 0) {
				current_cell.m_waterConc = 0;
			}
			else {
				current_cell.m_waterConc = current_water_conc / size_of_grid;
			}
			

			res.push_back(current_cell);
		}
	}
	return res;
}
