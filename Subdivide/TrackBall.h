// Class: TrackBall, OpenGL
#ifndef __TRACKBALL_H__
#define __TRACKBALL_H__

#include <iostream>
#include "Quaternion.h"
#include "point3.h"
using namespace std;

// the coordinate should be NCC;

class TrackBall
{
public:
	TrackBall();

	void Push(const float &x,const float &y);
	void Move(const float &x, const float &y);
public:
	Quaternion m_rotation;
private:
	void ScreenToWorld(const float & x, const float & y, Point3f & vec);

public:
	Point3f m_axis;
	float angle;
	//float m_angularVelocity;

	Point3f lastPos3D;
};



#endif