#pragma once
#include "vec.h"
#include"ray.h"

typedef enum class type { METAL, MIRROR, DIELECRIC, TRANSPARENT, GLOSSY, GLASS} type;

class object
{

};

class sphere
{
public:
	float radius, ior, s_h;
	vec color, position;
	type material;
	sphere(vec col, vec pos, float r, type mat, float val = 32, float _ior = 1.0f) :color{ col }, position{ pos }, 
		radius{ r }, material{ mat }, ior{ _ior }, s_h{ val }{};
	float sphere_intersection(const ray& r);
};