#pragma once

#ifndef __min
#define __min(a,b) ((a)<(b)?(a):(b);
#endif

#ifndef __max
#define __max(a,b) ((a)>(b)?(a):(b);
#endif

#include <cmath>
#include <iostream>
using namespace std;

template <class C> class Point3;
typedef Point3<float> Point3f;
typedef Point3<double> Point3d;
typedef Point3<int> Point3i;

template <class C>
class Point3 
{
private:
	C v[3];

public:
	// constructors
	Point3() { v[0] = v[1]= v[2] = 0; }
	Point3(const C & value) { v[0] = v[1] = v[2] = value; }
	Point3(const C & a, const C & b, const C & c) 
	{
		v[0] = a;
		v[1] = b;
		v[2] = c;
	}
	Point3(C arr[]) 
	{
		v[0] = arr[0];
		v[1] = arr[1];
		v[2] = arr[2];
	}
	Point3(const Point3<C> & right) 
	{
		v[0] = right.v[0];
		v[1] = right.v[1];
		v[2] = right.v[2];
	}

	const C & X() const { return v[0]; }
	const C & Y() const { return v[1]; }
	const C & Z() const { return v[2]; }
	C & X() { return v[0]; }
	C & Y() { return v[1]; }
	C & Z() { return v[2]; }

	C & operator[] (int index) { return v[index]; }
	const C & operator[] (int index) const { return v[index]; }

	Point3<C> operator+ (const Point3<C> & r) const { return Point3<C>(v[0]+r[0], v[1]+r[1], v[2]+r[2]); }
	Point3<C> operator- (const Point3<C> & r) const { return Point3<C>(v[0]-r[0], v[1]-r[1], v[2]-r[2]); }
	Point3<C> operator* (const C & r) const { return Point3<C>(v[0]*r, v[1]*r, v[2]*r); }
	friend Point3<C> operator*(const C & l, const Point3<C> & r);
	Point3<C> operator/ (const C & r) const { return Point3<C>(v[0]/r, v[1]/r, v[2]/r); }
	Point3<C> operator-() const { return Point3<C>(-v[0], -v[1], -v[2]); }
	Point3<C> & operator= (const Point3<C> & r) { v[0]=r[0]; v[1]=r[1]; v[2]=r[2]; return *this; }
	Point3<C> & operator+= (const Point3<C> & r) { return (*this) = (*this) + r; }
	Point3<C> & operator-= (const Point3<C> & r) { return (*this) = (*this) - r; }
	Point3<C> & operator*= (const C & r) { return (*this) = (*this) * r; }
	Point3<C> & operator/= (const C & r) { return (*this) = (*this) / r; }

	C Dot(const Point3<C> & r) const { return v[0]*r[0] + v[1]*r[1] + v[2]*r[2]; }
	Point3<C> Cross(const Point3<C> & r) const 
	{ 
		return Point3<C>(	v[1] * r[2] - v[2] * r[1],
							v[2] * r[0] - v[0] * r[2],
							v[0] * r[1] - v[1] * r[0] );
	}
	C L1Norm() const { return fabs(v[0]) + fabs(v[1]) + fabs(v[2]); }
	C L2Norm() const { return sqrt(Dot(*this)); }
	C Distance(const Point3<C> & r) const { return (*this-r).L2Norm(); }
	
	void Normalize() 
	{
		C denom = this->L2Norm();
		v[0] /= denom;
		v[1] /= denom;
		v[2] /= denom;
	}

	Point3<C> Min(const Point3<C> & r) 
	{
		return Point3<C>(__min(v[0], r[0]), __min(v[1], r[1]), __min(v[2], r[2]));
	}
	
	Point3<C> Max(const Point3<C> & r) 
	{
		return Point3<C>(__max(v[0], r[0]), __max(v[1], r[1]), __max(v[2], r[2]));
	}

	const C * ToArray() const { return v; }

	friend Point3<C> operator*(const C & l, const Point3<C> & r) 
	{
		return Point3<C>(l*r[0], l*r[1], l*r[2]);
	}
	
	friend ostream & operator<< (ostream & out, const Point3<C> & r) 
	{
		return out << r[0] << " " << r[1] << " " << r[2];
	}
};
