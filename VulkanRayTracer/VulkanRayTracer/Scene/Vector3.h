#pragma once

/// <summary>
/// A custom Vector3 structure, operating on float types.
/// </summary>

struct Vector3
{
	float x, y, z;

	Vector3();
	Vector3(float, float, float);

	// Operator overloads
	Vector3 operator + (const Vector3 &rhs) const;
	Vector3 operator - (const Vector3 &rhs) const;
	Vector3 operator * (float rhs) const;
	Vector3 operator * (const Vector3 & rhs) const;
	Vector3 operator / (float rhs) const;
	Vector3 operator / (Vector3 rhs) const;

	float Magnitude();
	Vector3& Normalized();
	
	static float Dot(const Vector3 &lhs, const Vector3 &rhs);
	static Vector3 Cross(const Vector3 &lhs, const Vector3 &rhs);
};