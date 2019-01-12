#pragma once
#include <iostream>
#include "Quaternion.h"
#include "point3.h"
using namespace std;
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