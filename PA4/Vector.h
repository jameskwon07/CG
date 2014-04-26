#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include <string>

#include "Point.h"

// For efficiently calculating vectors
class Vector
{
public:
	Vector()
		: i(0), j(0), k(0)
	{}
	Vector(double i_value, double j_value, double k_value)
		: i(i_value), j(j_value), k(k_value)
	{}
	Vector(const Point& p1, const Point& p2)
	{
		i = p2.x - p1.x;
		j = p2.y - p1.y;
		k = p2.z - p1.z;
	}

	virtual ~Vector()
	{}

	// Debugging print
	void print(std::string str)
	{
		printf("%s\t Vector i : %f\t j:%f\t k:%f\n", str.c_str(), i, j, k);
	}
	void print()
	{
		print("");
	}

	// initialize with two points
	void calculate_two_points(const Point& p1, const Point& p2)
	{
		i = p2.x - p1.x;
		j = p2.y - p1.y;
		k = p2.z - p1.z;
	}

	// return length
	double length()
	{
		return sqrt(i*i + j*j + k*k);
	}	
	
	// normalize
	void normalize()
	{
		i /= length();
		j /= length();
		k /= length();
	}

	// multiply scalar
	Vector& operator*=(const double rhs)
	{
		this->i *= rhs;
		this->j *= rhs;
		this->k *= rhs;

		return *this;
	}

	// static cross functions
	static void cross(const Vector& v1, const Vector& v2, Vector* result)
	{
		result->i = v1.j * v2.k - v1.k * v2.j;	
		result->j = v1.k * v2.i - v1.i * v2.k;	
		result->k = v1.i * v2.j - v1.j * v2.i;	
	}

	double i;
	double j;
	double k;
};

inline Vector operator*(Vector lhs, const double rhs)
{
	lhs *= rhs;
	return lhs;	
}

#endif
