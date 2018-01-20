#pragma once

#include "Material.h"
#include "Vector3.h"

/// <summary>
/// Provides all information required to render a sphere.
/// </summary>

struct Sphere
{
	Vector3 position;
	float radius;

	Material mat;

	Sphere();
	Sphere(Vector3 position, float radius);
};

