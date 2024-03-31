#include "staggered_grid.h"


/* We could hardcode it to only work if the incoming positions are only halfway points but in our use we will never use it in any other way. */
float grid_halfway_point(std::vector<float> grid, int gridWidth, int gridLength, double x_pos, double y_pos)
{
	float sum = 0.f;
	int num_points = 0;	

	/* if both x and y are midway points we use this one. */
	if ((x_pos != floor(x_pos)) && (x_pos != floor(x_pos))) {
		sum += get_from_grid(&grid, gridWidth, gridLength, floor(x_pos), floor(y_pos));
		sum += get_from_grid(&grid, gridWidth, gridLength, ceil(x_pos), floor(y_pos));
		sum += get_from_grid(&grid, gridWidth, gridLength, floor(x_pos), ceil(y_pos));
		sum += get_from_grid(&grid, gridWidth, gridLength, ceil(x_pos), ceil(y_pos));

		num_points = 4;
	}

	/* If just x is a midpoint we use this  */
	else if (x_pos != floor(x_pos)) {
		sum += get_from_grid(&grid, gridWidth, gridLength, floor(x_pos), y_pos);
		sum += get_from_grid(&grid, gridWidth, gridLength, ceil(x_pos), y_pos);
		num_points = 2;
	}

	/* If just y is a midpoint we use this */
	else if (y_pos != floor(y_pos)) {
		sum += get_from_grid(&grid, gridWidth, gridLength, floor(x_pos), floor(y_pos));
		sum += get_from_grid(&grid, gridWidth, gridLength, x_pos, ceil(y_pos));
		num_points = 2;
	}

	/* Final scenario, there is no midpoint and this is good. */
	else {
		sum = get_from_grid(&grid, gridWidth, gridLength, x_pos, y_pos);
		num_points = 1;
	}

	return sum / num_points;
}

float get_from_grid(std::vector<float>* grid, int grid_width, int grid_height, int x_pos, int y_pos)
{
	return 2.f;

	/* If the x or y positions are out of bounds of the grid, the velocities will just be 0 for that position. */
	if ((x_pos < 0) || (x_pos > grid_width)) { return 0; }
	if ((y_pos < 0) || (y_pos > grid_height)) { return 0; }

	int grid_position = grid_width * y_pos + x_pos;

	return grid->at(grid_position);
}

/** 
 * The staggered grid stores the data of the border positions, and must calculate the data in the centres. 
 * The axis boolean shows which axis the borders are being stored at. 
 */
Staggered_Grid::Staggered_Grid(int width, int height, bool x_axis):
	// data_values(std::vector((width + 1) * (height + 1), 0.f)),
	// width(width),
	// height(height),
	x_axis(x_axis)
{
	/* If we use the x axis, the width is increased by 1 as we use the vertices. Use height + 1 otherwise for the same reason. */
	if (x_axis) {
		data_values = std::vector((width + 1) * (height), 0.f);
		this->width = width + 1;
		this->height = height;
	}
	else {
		data_values = std::vector((width) * (height + 1), 0.f);
		this->width = width;
		this->height = height + 1;
	}
}

/** 
 * This is probably a valid way to set this
 */
void Staggered_Grid::set_new_data_values(std::vector<float> new_data)
{
	data_values = new_data;
}

std::vector<float> Staggered_Grid::get_data_values(void)
{
	return data_values;
}

int Staggered_Grid::num_data_points(void)
{
	return data_values.size();
}



/**
 * Gets the data at the relevant grid position. The staggered grid stores the data of the edge positions, and must calculate the 
 */
float Staggered_Grid::get_at_pos(float x, float y)
{
	/* If out of bounds, velocity is 0. */
	if (x < -0.5f || x > width + 0.5f || y < -0.5f || y > height + 0.5f) { return 0.f; }

	/* Find if we are looking at a border or a centre. We look if it is a border, otherwise just assume it is a centre. */
	bool is_x_border = true;//(abs(x - trunc(x)) == 0.5);
	bool is_y_border = true;//(abs(y - trunc(y)) == 0.5);

	/* This is quite ugly */
	/* The calculation depends on which axes this is. With all of the  */
	if (x_axis) {
		/* If it is the x border, then we simply take the relevant position's data. */
		if (is_x_border) {
			/* If both are borders then we must get the average of the two (?) */
			if (is_y_border) {
				/* Here, both x and y are borders. This is a bit strange because it is unclear what to do if Y is a border, but we will just take the centre point between the two. */
				float val1 = get_from_grid(&data_values, width, height, ceil(x), ceil(y));
				float val2 = get_from_grid(&data_values, width, height, ceil(x), ceil(y) + 1);
				return (val1 + val2) / 2;
			}
			else {
				return get_from_grid(&data_values, width, height, ceil(x), y); // data_values.at(ceil(x) + (y * width));
			}
		}
		else {
			if (is_y_border) {
				/* If it is the centre of an X position and the Y is in between, I think we just take the centres of the other two? It is unclear what to do here. */
				float val1 = get_from_grid(&data_values, width, height, ceil(x), ceil(y));
				float val2 = get_from_grid(&data_values, width, height, ceil(x), ceil(y) + 1);

				float val3 = get_from_grid(&data_values, width, height, ceil(x) + 1, ceil(y));
				float val4 = get_from_grid(&data_values, width, height, ceil(x) + 1, ceil(y) + 1);

				return (val1 + val2 + val3 + val4) / 4;
			}
			/* Otherwise it is a centre */
			else {
				float val1 = get_from_grid(&data_values, width, height, ceil(x), y);
				float val2 = get_from_grid(&data_values, width, height, ceil(x) + 1, y);
				return (val1 + val2) / 2;
			}

		}
	}
	/* Otherwise we just need to flip the values between x and y. */
	else {
		/* If it is the y border, then we simply take the relevant position's data. */
		if (is_y_border) {
			/* If both are borders then we must get the average of the two (?) */
			if (is_x_border) {
				/* Here, both x and y are borders. Doing the same as with the X side, but switched around. */
				float val1 = get_from_grid(&data_values, width, height, ceil(x),     ceil(y));
				float val2 = get_from_grid(&data_values, width, height, ceil(x) + 1, ceil(y));
				return (val1 + val2) / 2;
			}
			else {
				return get_from_grid(&data_values, width, height, x, ceil(y)); // data_values.at(ceil(x) + (y * width));
			}
		}
		else {
			if (is_x_border) {
				/* X is border, Y is not. This means we get the centre position for Y and the combination of both corners for X. */
				float val1 = get_from_grid(&data_values, width, height, ceil(x), ceil(y));
				float val2 = get_from_grid(&data_values, width, height, ceil(x), ceil(y) + 1);

				float val3 = get_from_grid(&data_values, width, height, ceil(x) + 1, ceil(y));
				float val4 = get_from_grid(&data_values, width, height, ceil(x) + 1, ceil(y) + 1);

				return (val1 + val2 + val3 + val4) / 4;
			}
			/* Otherwise it is a centre */
			else {
				float val1 = get_from_grid(&data_values, width, height, x, ceil(y));
				float val2 = get_from_grid(&data_values, width, height, x, ceil(y) + 1);
				return (val1 + val2) / 2;
			}

		}
	}
	
	// return 0.0f; This is never reached anyway 
}

/**
 * A function used to zero the boundaries on both ends of the given index coordinates.
 */
void Staggered_Grid::zero_at_pos(int x, int y)
{
	/* If out of bounds, do nothing. */
	if (x < 0 || x > width || y < 0 || y > height) { return; }

	/* Type of grid affects which positions are the 'borders'. */
	if (x_axis) {
		/* The first of the two boundaries to zero is at position x,y. the second is at x+1,y. */
		data_values.at(y * width + x) = 0.f;
		data_values.at(y * width + (x + 1)) = 0.f;
	}
	/* Otherwise we just do the same to the y axis. */
	else {
		data_values.at(y * width + x) = 0.f;
		data_values.at((y + 1) * width + x) = 0.f;
	}
}

float Staggered_Grid::max_value()
{
	float max_value = std::numeric_limits<float>::min();
	for (int i = 0; i < data_values.size(); i++) {
		max_value = std::max(max_value, data_values.at(i));
	}
	return max_value;
}

float Staggered_Grid::min_value()
{
	float min_value = std::numeric_limits<float>::max();
	for (int i = 0; i < data_values.size(); i++) {
		min_value = std::min(min_value, data_values.at(i));
	}
	return min_value;
}


