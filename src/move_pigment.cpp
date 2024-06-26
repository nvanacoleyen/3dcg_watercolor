#include "move_pigment.h"
#include "global_constants.h"
#include <cmath>

/* For some reason M is never used despite the algorithm saying that it should be included in it. */
void movePigment(std::vector<Cell>* Grid, Staggered_Grid* u, Staggered_Grid* v)
{

	/* Create new grid of pigments */
	std::vector<Cell> new_Grid = *Grid;
	std::vector<Cell> current_Grid = *Grid;

	float delta_t = 1 / std::max(1.f, std::ceil(std::max(u->max_value(), v->max_value())));
	for (float t = 0.f; t <= 1.f; t += delta_t) {

		/* Loop through all cells and adjust pigments */
		for (int j = 1; j < HEIGHT - 2; j++) {
			for (int i = 1; i < WIDTH - 2; i++) {
				new_Grid[WIDTH * (j + 1) + (i + 1) + 1].m_pigmentConc += delta_t * std::max(0.f, (u->get_at_pos(i + 0.5, j) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc));
				new_Grid[WIDTH * (j + 1) + (i - 1) + 1].m_pigmentConc += delta_t * std::max(0.f, (-u->get_at_pos(i - 0.5, j) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc));
				new_Grid[WIDTH * (j + 2) + i + 1].m_pigmentConc += delta_t * std::max(0.f, (v->get_at_pos(i, j + 0.5) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc));
				new_Grid[WIDTH * (j) + i + 1].m_pigmentConc += delta_t * std::max(0.f, (-v->get_at_pos(i, j - 0.5) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc));

				new_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc -= delta_t * (std::max(0.f, (u->get_at_pos(i + 0.5, j) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc)) +
					std::max(0.f, (-u->get_at_pos(i - 0.5, j) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc)) +
					std::max(0.f, (v->get_at_pos(i, j + 0.5) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc)) +
					std::max(0.f, (-v->get_at_pos(i, j - 0.5) * current_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc)));

				new_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc = std::max(0.f, new_Grid[WIDTH * (j + 1) + i + 1].m_pigmentConc);

			}
		}

		current_Grid = new_Grid;

	}

	*Grid = new_Grid;

	//return new_Grid;
}

void updateColors(std::vector<paperVertex>& vertices, std::vector<Cell>& Grid, float& brush_radius, GLuint& VBO, const char* colour_mode, Staggered_Grid* u, Staggered_Grid* v, std::vector<float>* p)
{

	for (size_t i = 0; i < vertices.size(); i++)
	{
		if (strcmp(colour_mode, "Water velocity") == 0) {
			if (Grid[i].m_waterConc <= minimum_wetness) {
				vertices[i].color = glm::vec3(0);
			}
			else {
				float x_velocity_factor = (std::min(25.f, std::max(-25.f, u->get_from_total(i))) + 25) / 50;
				float y_velocity_factor = (std::min(25.f, std::max(-25.f, v->get_from_total(i))) + 25) / 50;
			
				glm::vec3 color = glm::vec3(x_velocity_factor, y_velocity_factor, 0.5);

				vertices[i].color = color;
			}
			
		}
		else if (strcmp(colour_mode, "X Water velocity") == 0) {
			if (Grid[i].m_waterConc <= minimum_wetness) {
				vertices[i].color = glm::vec3(0);
			}
			else {
				float x_velocity_factor = (std::min(25.f, std::max(-25.f, u->get_from_total(i))) + 25) / 50;

				glm::vec3 color = glm::vec3(x_velocity_factor, 0.5, 0.5);

				vertices[i].color = color;
			}

		}
		else if (strcmp(colour_mode, "Y Water velocity") == 0) {
			if (Grid[i].m_waterConc <= minimum_wetness) {
				vertices[i].color = glm::vec3(0);
			}
			else {
				float y_velocity_factor = (std::min(25.f, std::max(-25.f, v->get_from_total(i))) + 25) / 50;

				glm::vec3 color = glm::vec3(0.5, y_velocity_factor, 0.5);

				vertices[i].color = color;
			}

		}
		else if (strcmp(colour_mode, "Water pressure") == 0) {
			if (Grid[i].m_waterConc <= minimum_wetness) {
				vertices[i].color = glm::vec3(0.3);
			}
			else {
				float pressure_factor = (std::min(10.f, std::max(-10.f, p->at(i))) + 10) / 20;
				glm::vec3 color = glm::vec3(pressure_factor);

				vertices[i].color = color;
			}
		}
		// The default other option is pigment mode.
		else {
			// Color with/without water/pigment concentration
			// For every vertex in the square:
			glm::vec3 color;
			if (Grid[i].m_waterConc >= minimum_wetness && Grid[i].m_pigmentConc <= 0.005) {
				color = glm::vec3(0.8);
			}
			else if (Grid[i].m_pigmentConc > 0.0005) {
				float pigment_factor = std::min(1.f, std::max(0.f, Grid[i].m_pigmentConc));
				color = glm::vec3(std::max(0.0, 0.65 - (1.2 * pigment_factor)), std::max(0.0, 0.65 - (1.2 * pigment_factor)), 0.8 + (0.2 * pigment_factor));
			}
			else {
				color = glm::vec3(1);
			}
			vertices[i].color = color;
		}
		
	}
}