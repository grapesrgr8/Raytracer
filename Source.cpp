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
//do you add diffuse for dielectric materials?

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
	vec shadow_orig = dot(rev_light_dir, N) < 0 ? hit_point - N * 1e-3 : hit_point + N * 1e-3;
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
	return k < 0 ? vec(0, 0, 0) : eta * I + (eta * cosi - sqrtf(k)) * n;
}

float fresnel(const vec& I, const vec& N, const float& ior, const float& env = 1.0f)
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
float shlicks_approx(const vec& I, const vec& N, const float& ior, const float& env = 1.0f)
{
	float r0 = (env - ior) / (env + ior);
	float kr;
	float cosine = std::max(-1.0f, std::min(1.0f, dot(I, N)));
	r0 = r0 * r0;
	return r0 + (1 - r0) * std::pow((1 - cosine), 5);
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
				//compute reflection
				vec ref_direction = r.direction - 2.0 * dot(normal, r.direction) * normal;
				ray reflection_ray(hit_point, ref_direction.normalize());
				total += scene(obj, lights, reflection_ray, depth + 1) * 0.34;//only metals are a 100% reflective
			}
			if (obj[pos]->material == type::TRANSPARENT)
			{
				//compute refraction
				vec ref_direction = refract(r.r_direction(), normal, obj[pos]->ior);//absolute refractive index cant be < 1, need to add surrounding medium refractive index
				vec refract_orig = dot(ref_direction, normal) < 0 ? hit_point - normal * 1e-3 : hit_point + normal * 1e-3;
				ray refraction_ray(refract_orig, ref_direction);
				total += scene(obj, lights, refraction_ray, depth + 1) * (1 - 0.18);
			}
			if (obj[pos]->material == type::DIELECRIC || obj[pos]->material == type::GLASS)
			{
				//compute dielectric
				vec reflection_color(0, 0, 0), refraction_color(0, 0, 0);
				float kr = fresnel(r.direction, normal, obj[pos]->ior);
				if (obj[pos]->material == type::GLASS)
				{
					kr = shlicks_approx(r.direction, normal, obj[pos]->ior, 1.0);
				}
				float kt = kr - 1;
				if (kr < 1)
				{
					vec ref_direction = refract(r.r_direction(), normal, obj[pos]->ior);
					vec refract_orig = dot(ref_direction, normal) < 0 ? hit_point - normal * 1e-3 : hit_point + normal * 1e-3;
					ray refraction_ray(refract_orig, ref_direction);
					reflection_color += scene(obj, lights, refraction_ray, depth + 1);
				}
				vec ref_direction = r.direction - 2.0 * dot(normal, r.direction) * normal;
				ray reflection_ray(hit_point, ref_direction.normalize());
				reflection_color += scene(obj, lights, reflection_ray, depth + 1);
				total += kr * reflection_color + refraction_color * (1 - kr) * obj[pos]->color;
			}
		}
		for (size_t i = 0; i < lights.size(); i++)
		{
			if (check_shadow(obj, hit_point, lights[i], normal))
			{
				total += vec(0, 0, 0);
			}
			else
			{
				//float scaleS = 5; // scale of the pattern 
				//float pattern = (sin(hitTexCoordinates.x * 2 * M_PI * scaleS) + 1) * 0.5;
				vec l_dir = lights[i]->light_direction(hit_point).normalize();
				vec diffuse = std::max(0.0f, dot(-lights[i]->light_direction(hit_point), normal)) * obj[pos]->color;
				vec ref_direction = 2.0 * dot(normal, l_dir) * normal - l_dir;
				vec specular = vec(1, 1, 1) * std::pow(dot(-r.direction, ref_direction), obj[pos]->s_h);

				if (/*obj[pos]->material == type::DIELECRIC || */
					obj[pos]->material == type::MIRROR || obj[pos]->material == type::TRANSPARENT || 
					obj[pos]->material == type::GLASS)
					diffuse = vec(0, 0, 0);

				total += (diffuse + specular) * lights[i]->light_intensity();
			}
		}
		return total;
	}
	else
		return vec(0.529, 0.808, 0.98);
}

int main()
{
	const int SCR_WIDTH = 500, SCR_HEIGHT = 400;
	std::ofstream file("img.ppm", std::ios::out | std::ios::binary);
	file << "P6\n" << SCR_WIDTH << " " << SCR_HEIGHT << "\n255\n";

	std::vector<std::unique_ptr<sphere>> objects;
	std::vector<std::unique_ptr<light>> lights;

	lights.emplace_back(std::make_unique<directlight>(vec(1, 4, 4), 0.4f));
	lights.emplace_back(std::make_unique<pointlight>(vec(2, 1, 0), 0.4f));
	lights.emplace_back(std::make_unique<directlight>(vec(0, -1, 0), 0.6f));

	objects.emplace_back(std::make_unique<sphere>(vec(1, 0, 0), vec(0, -1, -3), 1.0f, type::GLOSSY, 32));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 0, 1), vec(-2, 1, -5), 1.0f, type::GLOSSY));
	objects.emplace_back(std::make_unique<sphere>(vec(1, 0, 0), vec(-2, 1, -3), 1.0f, type::TRANSPARENT, 256));
	objects.emplace_back(std::make_unique<sphere>(vec(0, 1, 0), vec(2, 1, -3), 1.0f, type::METAL));
	objects.emplace_back(std::make_unique<sphere>(vec(1, 1, 0), vec(0, -5001, 0), 5000.0f, type::GLOSSY, 32));

	camera cam(vec(0, 0, 1), vec(0, 0, -1), 60.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT);

	for (int j = 0; j < SCR_HEIGHT; ++j)
	{
		for (int i = 0; i < SCR_WIDTH; ++i)
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