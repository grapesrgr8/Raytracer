#pragma once
#include <cmath>
class vec
{
public:
	float x, y, z;
	vec(float a, float b, float c) :x{ a }, y{ b }, z{ c } {}
	float xCoord() { return x; }
	float yCoord() { return y; }
	float zCoord() { return z; }
	float lenght() const;
	vec normalize() const;
	vec operator - () const;
};

vec operator + (const vec& v1, const vec& v2);

vec operator - (const vec& v1, const vec& v2);

vec operator * (const vec& v1, float f);

vec operator * (float f, const vec& v1);

vec operator / (const vec& v1, float f);

float dot(const vec& v1, const vec& v2);

vec cross(const vec& v1, const vec& v2);
