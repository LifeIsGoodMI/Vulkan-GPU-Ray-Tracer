#pragma once

#include "Vector3.h"

/// <summary>
/// Provides all information about the optical composition of any geometry.
/// </summary>

struct Material
{
	Vector3 color;
	int type;

	Material();
	Material(Vector3, int);
};

