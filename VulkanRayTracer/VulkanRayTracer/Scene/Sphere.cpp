#include "Sphere.h"
#include <math.h>

Sphere::Sphere() { radius = 1; }
Sphere::Sphere(Vector3 position, float radius) : position(position), radius(radius) {}