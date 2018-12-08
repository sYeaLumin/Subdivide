// Class: TrackBall, OpenGL
#ifndef __TRACKBALL_H__
#define __TRACKBALL_H__

#include <iostream>
#include "Quaternion.h"
#include "vector3.h"
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
	void ScreenToWorld(const float & x, const float & y, Vector3f & vec);

public:
	Vector3f m_axis;
	float angle;
	//float m_angularVelocity;

	Vector3f lastPos3D;
};



#endif