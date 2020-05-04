#include "vec.h"
#include "ray.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <limits>
#include <algorithm>
struct pointlight
{
public:
	vec origin;
	float intensity;
	pointlight(vec o, float i) :origin{ o }, intensity{ i }{}
	vec light_direction(const vec& hit) { return hit - origin; }//not normalized need it for other things
};

struct directlight
{
public:
	vec direction;
	float intensity;
	directlight(vec dir, float i) :direction{ dir }, intensity{ i }{}
	vec light_direction(const vec& hit) { return direction; }
};

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
	if (t0 < 0) return t1;
	if (t1 < 0) return t0;
	return t0 < t1 ? t0 : t1;
}

float closest_intersection(std::vector<std::unique_ptr<sphere>>& obj, const ray& r, int& loc)
{
	loc = 0;
	float t_far = std::numeric_limits<float>::max();
	float t_near = 1e-12;
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

vec scene(std::vector<std::unique_ptr<sphere>> &obj, const ray& r)
{
	int pos;
	directlight p{ vec(3,2,-1), 0.8f };
	float closest_t = closest_intersection(obj, r, pos);
	vec hit_point = r.point(closest_t);
	if (closest_t != std::numeric_limits<float>::max())
	{
		vec normal = (hit_point - obj[pos]->position).normalize();
		vec diffuse = std::max(0.0f, dot(-p.light_direction(hit_point), normal)) * obj[pos]->color * p.intensity;
		return diffuse;
	}
	else
		return vec(0, 0, 0);
}

int main()
{
	const int SCR_WIDTH = 200, SCR_HEIGHT = 100;
	std::ofstream file("img.ppm", std::ios::out | std::ios::binary);
	file << "P6\n" << SCR_WIDTH << " " << SCR_HEIGHT << "\n255\n";
	vec origin(0, 0, 0);
	vec horizontal(4, 0, 0);
	vec vertical(0, 2, 0);
	vec lower_left = origin - horizontal / 2 - vertical / 2 - vec(0, 0, 1);
	std::vector<std::unique_ptr<sphere>> objects;
	objects.emplace_back(std::make_unique<sphere>(vec(1, 0, 0), vec(0, 0, -1), 0.5f));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 1, 0), vec(0, 1, -3), 2));
	for (int j = 0; j < SCR_HEIGHT; j++)
	{
		for (int i = 0; i < SCR_WIDTH; i++)
		{
			float u = float(i) / float(SCR_WIDTH);
			float v = float(j) / float(SCR_HEIGHT);
			ray r(origin, (lower_left + u * horizontal + v * vertical).normalize());
			vec color = scene(objects, r);
			unsigned char red = (unsigned char)(std::max(0.0f, std::min(1.0f, color.xCoord())) * 255);
			unsigned char green = (unsigned char)(std::max(0.0f, std::min(1.0f, color.yCoord())) * 255);
			unsigned char blue = (unsigned char)(std::max(0.0f, std::min(1.0f, color.zCoord())) * 255);
			file << red << green << blue;
		}
	}
	file.close();
	return 0;
}
