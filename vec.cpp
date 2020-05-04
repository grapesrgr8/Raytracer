#include "vec.h"

float vec::lenght() const
{
	return sqrt(x * x + y * y + z * z);
}

vec vec::operator - () const
{
	return vec(-x, -y, -z);
}

vec vec::normalize() const
{
	return (*this) / (*this).lenght();
}

vec operator + (const vec& v1, const vec& v2)
{
	return vec(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

vec operator - (const vec& v1, const vec& v2)
{
	return vec(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

vec operator * (const vec& v1, float f)
{
	return vec(v1.x * f, v1.y * f, v1.z * f);
}

vec operator * (float f, const vec& v1)
{
	return vec(v1.x * f, v1.y * f, v1.z * f);
}

vec operator / (const vec& v1, float f)
{
	return vec(v1.x * 1/f, v1.y * 1/f, v1.z * 1/f);
}

float dot (const vec& v1, const vec& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vec cross(const vec& v1, const vec& v2)
{
	return vec((v1.y * v2.z - v1.z * v2.y), (v1.z * v2.x - v1.x * v2.z), (v1.x * v2.y - v1.y * v2.x));
}