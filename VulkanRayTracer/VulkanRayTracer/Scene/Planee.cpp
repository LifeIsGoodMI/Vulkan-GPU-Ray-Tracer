#include "Planee.h"
#include <math.h>

Planee::Planee() { distance = 1; }
Planee::Planee(Vector3 normal, float distance) : normal(normal), distance(distance) {}