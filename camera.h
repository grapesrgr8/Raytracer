#pragma once
#include <functional>
#include <random>
#include "vec.h"
#include "ray.h"
#include <cmath>

#define PI 3.14159265358979323846

double random_double(double min, double max);

double random_double();

class camera
{
public:
	vec lower_left_corner, horizontal, vertical, origin, u, v, w;
	float lens_radius;
	camera(vec lookfrom, vec lookat, float fov, float a_r, float lens, float focus_dist, vec v_up = vec(0, 1, 0));
	ray c_get_ray(float, float);
};

