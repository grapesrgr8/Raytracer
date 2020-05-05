#include "object.h"

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