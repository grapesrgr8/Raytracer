#pragma once
#include "vec.h"

class ray
{
public:
	vec origin, direction;
	ray(vec o, vec d) : origin{ o }, direction{ d }{}
	vec r_direction() const { return direction; }
	vec r_origin() const{ return origin; }
	vec point(float t) { return origin + t * direction; }
};

