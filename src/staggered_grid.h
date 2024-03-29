#include <vector>

float grid_halfway_point(std::vector<float> grid, int grid_width, int grid_height, double x_pos, double y_pos);

float get_from_grid(std::vector<float> grid, int grid_width, int grid_height, int x_pos, int y_pos);

class staggered_grid {
public:
	std::vector<float> data_values;
	int width;
	int height;
	bool x_axis;

	staggered_grid(int width, int height, bool x_axis);

	void set_new_data_values(std::vector<float> new_data);
	std::vector<float> get_data_values(void);
	float get_at_pos(float x, float y);
	float max_value();
	float min_value();
};