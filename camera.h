#pragma once
#include "vec.h"
#include "ray.h"
#include <cmath>
#define PI 3.14159265358979323846

class camera
{
public:
	vec lower_left_corner, horizontal, vertical, origin;
	camera(vec lookfrom, vec lookat, float fov, float a_r, vec v_up = vec(0, 1, 0));
	ray c_get_ray(float, float);
};

