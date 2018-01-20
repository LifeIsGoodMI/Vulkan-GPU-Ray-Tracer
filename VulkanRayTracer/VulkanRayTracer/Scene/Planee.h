#pragma once

#include "Material.h"
#include "Vector3.h"

/// <summary>
/// Provides all information required to render a plane.
/// </summary>

struct Planee
{
	Vector3 normal;
	float distance;

	Material mat;

	Planee();
	Planee(Vector3 normal, float distance);
};
