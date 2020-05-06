#pragma once
#include <cmath>
class vec
{
public:
	float x, y, z;
	vec() :x{ 0 }, y{ 0 }, z{ 0 }{}
	vec(float a, float b, float c) :x{ a }, y{ b }, z{ c } {}
	float xCoord() { return x; }
	float yCoord() { return y; }
	float zCoord() { return z; }
	float lenght2() const;
	float lenght() const;
	vec normalize() const;
	vec operator - () const;
	vec& operator += (const vec& v1);
};

vec operator + (const vec& v1, const vec& v2);

vec operator - (const vec& v1, const vec& v2);

vec operator * (const vec& v1, float f);

vec operator * (float f, const vec& v1);

vec operator * (const vec& v1, const vec& v2);

vec operator / (const vec& v1, float f);

float dot(const vec& v1, const vec& v2);

vec cross(const vec& v1, const vec& v2);
