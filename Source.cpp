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
#include "object.h"

const double EULER = 2.71828182845904523536;

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

bool check_shadow(std::vector<std::unique_ptr<sphere>>& obj, const vec& hit_point, std::unique_ptr<light> &light, const vec& N)
{
	vec rev_light_dir = -light->light_direction(hit_point).normalize();
	vec shadow_orig = dot(rev_light_dir, N) < 0 ? hit_point - N * 1e-3f : hit_point + N * 1e-3f;
	float dist_to_light = rev_light_dir.lenght();
	ray shadow_ray(shadow_orig, rev_light_dir);
	int loc1;
	float t = closest_intersection(obj, shadow_ray, loc1);
	if (t == std::numeric_limits<float>::max())
		return false;
	else
	{
		if (light->id_tag() == 0)//checks wheter point light or direct light
		{
			float dist_to_obj = shadow_ray.point(t).lenght();
			if (dist_to_obj < dist_to_light)//so that light doesn't overshoot
				return true;
			else
				return false;
		}
		else
			return true;
	}
}

vec refract(const vec& I, const vec& N, const float& ior, const float& env = 1.0f)
{
	float cosi = std::max(-1.0f, std::min(1.0f, dot(I, N)));
	float etai = env, etat = ior;
	vec n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { std::swap(etai, etat); n = -N; }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	return k < 0 ? I - 2.0 * dot(N, I) * N : eta * I + (eta * cosi - sqrtf(k)) * n;
}

float shlicks_approx(const vec& I, const vec& N, const float& ior, const float& env = 1.0f)
{
	float kr;
	float cosi = std::max(-1.0f, std::min(1.0f, dot(I, N)));
	float etai = env, etat = ior;
	if (cosi > 0) { std::swap(etai, etat); }
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
	if (sint >= 1) {
		kr = 1;
	}
	else {
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
	return kr;
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
			if (obj[pos]->material == type::METAL || obj[pos]->material == type::MIRROR)
			{
				vec ref_direction = r.direction - 2.0 * dot(normal, r.direction) * normal;
				ray reflection_ray(hit_point, ref_direction.normalize());
				total += scene(obj, lights, reflection_ray, depth + 1);//only metals are a 100% reflective
			}
			if (obj[pos]->material == type::TRANSPARENT)
			{
				vec ref_direction = refract(r.r_direction(), normal, obj[pos]->ior).normalize();//absolute refractive index cant be < 1, need to add surrounding medium refractive index
				vec refract_orig = dot(ref_direction, normal) < 0 ? hit_point - normal * 1e-3f : hit_point + normal * 1e-3f;
				ray refraction_ray(refract_orig, ref_direction);
				total += scene(obj, lights, refraction_ray, depth + 1) * (1.0f - 0.05f);//(1 - absorbance)
			}

			if (obj[pos]->material == type::DIELECRIC || obj[pos]->material == type::GLASS)
			{
				vec refr_color, refl_color;
				float kr = shlicks_approx(r.direction, normal, obj[pos]->ior);
				if (kr < 1)
				{
					vec ref_direction = refract(r.r_direction(), normal, obj[pos]->ior).normalize();
					vec refract_orig = dot(ref_direction, normal) < 0 ? hit_point - normal * 1e-3f : hit_point + normal * 1e-3f;
					ray refraction_ray(refract_orig, ref_direction);
					refr_color = scene(obj, lights, refraction_ray, depth + 1);
				}
				vec ref_direction = r.direction - 2.0 * dot(normal, r.direction) * normal;
				ray reflection_ray(hit_point, ref_direction.normalize());
				refl_color += scene(obj, lights, reflection_ray, depth + 1);
				total += refl_color * kr + refr_color * (1 - kr);
			}

		}
		for (size_t i = 0; i < lights.size(); i++)
		{
			if (check_shadow(obj, hit_point, lights[i], normal))
				total += vec(0, 0, 0);
			else
			{
				vec l_dir = lights[i]->light_direction(hit_point).normalize();
				vec diffuse = std::max(0.0f, dot(-lights[i]->light_direction(hit_point), normal)) * obj[pos]->color;
				vec ref_direction = 2.0 * dot(normal, l_dir) * normal - l_dir;
				vec specular = vec(1, 1, 1) * std::pow(dot(-r.direction, ref_direction), obj[pos]->s_h);

				if (obj[pos]->material == type::MIRROR || obj[pos]->material == type::TRANSPARENT || obj[pos]->material == type::GLASS)
					diffuse = vec(0, 0, 0);

				total += (diffuse + specular) * lights[i]->light_intensity();
			}
		}
		return total;
	}
	else
		return vec(0.0f, 0.0f, 0.0f);
}

int main()
{
	const int SCR_WIDTH = 400, SCR_HEIGHT = 300;
	std::ofstream file("img.ppm", std::ios::out | std::ios::binary);
	file << "P6\n" << SCR_WIDTH << " " << SCR_HEIGHT << "\n255\n";

	std::vector<std::unique_ptr<sphere>> objects;
	std::vector<std::unique_ptr<light>> lights;

	lights.emplace_back(std::make_unique<directlight>(vec(1, 4, 4), 0.4f));
	lights.emplace_back(std::make_unique<pointlight>(vec(2, 1, 0), 0.4f));
	lights.emplace_back(std::make_unique<directlight>(vec(0, -1, 0), 0.6f));

	objects.emplace_back(std::make_unique<sphere>(vec(1, 0, 0), vec(0, -1, -3), 1.0f, type::GLOSSY, 32));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 0, 1), vec(-2, 1, -5), 1.0f, type::GLOSSY));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 1, 1), vec(-2, 1, -3), 1.0f, type::GLASS, 256, 1.3));
	objects.emplace_back(std::make_unique<sphere>(vec(0.97, 0.96, 0.91), vec(2, 1, -3), 1.0f, type::METAL));
	objects.emplace_back(std::make_unique<sphere>(vec(1, 1, 0), vec(0, -5001, 0), 5000.0f, type::GLOSSY, 32));

	vec lookfrom = vec(3, 3, 2);
	vec lookat = vec(0, 0, -3);
	float aspect_ratio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
	float aperature = 2.0f;

	camera cam(lookfrom, lookat, 45.0f, aspect_ratio, aperature, (lookfrom-lookat).lenght());

	for (int j = 0; j < SCR_HEIGHT; ++j)
	{
		vec color(0, 0, 0);
		for (int i = 0; i < SCR_WIDTH; ++i)
		{
			float u = float(i) / float(SCR_WIDTH);
			float v = float(j) / float(SCR_HEIGHT);
			ray r = cam.c_get_ray(u, v);
			vec color = scene(objects, lights, r);
			unsigned char red = (unsigned char)(std::max(0.0f, std::min(1.0f, std::pow(color.xCoord(), 1))) * 255);
			unsigned char green = (unsigned char)(std::max(0.0f, std::min(1.0f, std::pow(color.yCoord(), 1))) * 255);
			unsigned char blue = (unsigned char)(std::max(0.0f, std::min(1.0f, std::pow(color.zCoord(), 1))) * 255);
			file << red << green << blue;
		}
	}
	file.close();
	return 0;
}