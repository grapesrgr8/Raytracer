#pragma once
#include "ray.h"
#include "vec.h"
struct light
{
public:
	virtual vec light_direction(const vec& hit) = 0;
	virtual float light_intensity() = 0;
};

struct pointlight : public light
{
public:
	vec origin;
	float intensity;
	int tag = 0;
	pointlight(vec o, float i) :origin{ o }, intensity{ i }{}
	vec light_direction(const vec& hit) { return hit - origin; }//not normalized need it for other things
	float light_intensity() { return intensity; }
};

struct directlight : public light
{
public:
	vec direction;
	float intensity;
	int tag = 1;
	directlight(vec dir, float i) :direction{ dir }, intensity{ i }{}
	vec light_direction(const vec& hit) { return direction; }
	float light_intensity() { return intensity; }
};
