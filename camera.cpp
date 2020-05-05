#include "camera.h"

camera::camera(vec lookfrom, vec lookat, float fov, float a_r, vec v_up)
{
	float half_height = tan(fov / 2 * PI / 180.0);
	float half_width = half_height * a_r;

    origin = lookfrom;

    vec w = (lookfrom - lookat).normalize();
    vec u = (cross(w, v_up)).normalize();
    vec v = cross(w, u);

    lower_left_corner = origin - half_width * u - half_height * v - w;

    horizontal = 2 * half_width * u;
    vertical = 2 * half_height * v;
}
ray camera::c_get_ray(float s, float t)
{
    return ray(origin, (lower_left_corner + s * horizontal + t * vertical - origin).normalize());
}