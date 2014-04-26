#ifndef POINT_H
#define POINT_H

class Vector;

// For efficiently calculating points
class Point
{
public:
	Point()
		: x(0), y(0), z(0)
	{}

	Point(double x_value, double y_value, double z_value)
		: x(x_value), y(y_value), z(z_value)
	{}

	virtual ~Point()
	{}

	// Debugging print
	void print()
	{
		printf("Point x : %f\t y:%f\t z:%f\n", x, y, z);
	}

	double x;
	double y;
	double z;	
};
#endif
