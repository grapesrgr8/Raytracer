#include "vec.h"
#include "ray.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <limits>
#include <algorithm>
#include "light.h"
#include "camera.h"

struct sphere
{
public:
	float radius;
	vec color, position;
	sphere(vec col, vec pos, float r) :color{ col }, position{ pos }, radius{ r }{};
	float sphere_intersection(const ray& r);
};

float sphere::sphere_intersection(const ray& r)
{
	vec L = position - r.r_origin();
	float tca = dot(L, r.r_direction());
    if (tca < 0) return false;
	float d2 = dot(L, L) - tca * tca;
	if (d2 > radius* radius) return false;
	float thc = sqrt(radius * radius - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;
	if (t0 < 0 && t1 < 0) return -1;
	if (t0 < 0) return t1;
	if (t1 < 0) return t0;
	return t0 < t1 ? t0 : t1;
}

float closest_intersection(std::vector<std::unique_ptr<sphere>>& obj, const ray& r, int& loc)
{
	loc = 0;
	float t_far = std::numeric_limits<float>::max();
	double t_near = 1e-12;
	for (size_t i = 0; i < obj.size(); i++)
	{
		float int_t = obj[i]->sphere_intersection(r);
		if (int_t < t_far && int_t > t_near)
		{
			t_far = int_t;
			loc = i;
		}
	}
	return t_far;
}

bool check_shadow(std::vector<std::unique_ptr<sphere>>& obj, const vec& hit_point, std::unique_ptr<light> &l)
{
	ray shadow_ray(hit_point, -l->light_direction(hit_point).normalize());
	int loc1;
	float t = closest_intersection(obj, shadow_ray, loc1);
	if (t == std::numeric_limits<float>::max())
		return false;
	else
		return true;
}

vec scene(std::vector<std::unique_ptr<sphere>> &obj, std::vector<std::unique_ptr<light>> &lights, const ray& r, int depth = 0)
{
	int pos;
	vec total(0, 0, 0);
	float closest_t = closest_intersection(obj, r, pos);
	vec hit_point = r.point(closest_t);
	vec normal = (hit_point - obj[pos]->position).normalize();
	if (closest_t != std::numeric_limits<float>::max())
	{
		if (depth < 3)
		{
		//compute reflection
			if (obj[pos]->radius == 0.8f)
			{
				vec ref_direction = r.direction - 2.0 * dot(normal, r.direction) * normal;
				ray reflection_ray(hit_point, ref_direction.normalize());
				total += scene(obj, lights, reflection_ray, depth + 1);
			}
		}
		for (size_t i = 0; i < lights.size(); i++)
		{
			vec diffuse = std::max(0.0f, dot(-lights[i]->light_direction(hit_point), normal)) * obj[pos]->color * lights[i]->light_intensity();
			total += diffuse;
		}
		return total;
	}
	else
		return vec(0, 0, 0);
}

int main()
{
	const int SCR_WIDTH = 200, SCR_HEIGHT = 100;
	std::ofstream file("img.ppm", std::ios::out | std::ios::binary);
	file << "P6\n" << SCR_WIDTH << " " << SCR_HEIGHT << "\n255\n";
	std::vector<std::unique_ptr<sphere>> objects;
	std::vector<std::unique_ptr<light>> lights;

	lights.emplace_back(std::make_unique<directlight>(vec(1, 4, 4), 0.4));
	lights.emplace_back(std::make_unique<pointlight>(vec(2, 1, 0), 0.6));
	lights.emplace_back(std::make_unique<directlight>(vec(0, -1, 0), 0.6));

	objects.emplace_back(std::make_unique<sphere>(vec(1, 0, 0), vec(0, -1, -3), 1.0f));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 0, 1), vec(-2, 1, -3), 0.8f));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 1, 0), vec(2, 1, -3), 1.0f));
	objects.emplace_back(std::make_unique<sphere>(vec(1, 1, 0), vec(0, -5001, 0), 5000.0f));

	camera cam(vec(0, 0, 0), vec(0, 0, -1), 60.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT);
	for (int j = 0; j < SCR_HEIGHT; j++)
	{
		for (int i = 0; i < SCR_WIDTH; i++)
		{
			float u = float(i) / float(SCR_WIDTH);
			float v = float(j) / float(SCR_HEIGHT);
			ray r = cam.c_get_ray(u, v);
			vec color = scene(objects, lights, r);
			unsigned char red = (unsigned char)(std::max(0.0f, std::min(1.0f, color.xCoord())) * 255);
			unsigned char green = (unsigned char)(std::max(0.0f, std::min(1.0f, color.yCoord())) * 255);
			unsigned char blue = (unsigned char)(std::max(0.0f, std::min(1.0f, color.zCoord())) * 255);
			file << red << green << blue;
		}
	}
	file.close();
	return 0;
}