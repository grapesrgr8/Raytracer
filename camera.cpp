#include "camera.h"

double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max - min) * random_double();
}

double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    static std::function<double()> rand_generator =
        std::bind(distribution, generator);
    return rand_generator();
}

vec random_in_unit_disk() {
    while (true) {
        auto p = vec(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.lenght2() >= 1) continue;
        return p;
    }
}

camera::camera(vec lookfrom, vec lookat, float fov, float a_r, float aperature, float focus_dist, vec v_up)
{
	float half_height = tan(fov / 2 * PI / 180.0);
	float half_width = half_height * a_r;

    origin = lookfrom;

    lens_radius = aperature / 2;

    vec w = (lookfrom - lookat).normalize();
    vec u = (cross(v_up, w)).normalize();
    vec v = cross(u, w);

    lower_left_corner = origin - half_width * u * focus_dist - half_height * v * focus_dist - w * focus_dist;

    horizontal = 2 * half_width * u * focus_dist;
    vertical = 2 * half_height * v * focus_dist;
}
ray camera::c_get_ray(float s, float t)
{
    vec rd = lens_radius * random_in_unit_disk();
    vec offset = u * rd.x + v * rd.y;
    return ray(origin + offset, (lower_left_corner + s * horizontal + t * vertical - origin - offset).normalize());
}