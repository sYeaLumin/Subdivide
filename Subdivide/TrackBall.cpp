#include "TrackBall.h"

TrackBall::TrackBall()
{
	m_rotation.LoadIdentity();
}


void TrackBall::ScreenToWorld(const float & x, const float & y, Vector3f & vec)
{
	vec[0] = x;
	vec[1] = y;
	vec[2] = 0.0f;

	float sqrZ = 1 - vec.Dot(vec);
	if (sqrZ > 0)
	{
		vec[2] = sqrt(sqrZ);
	}
	else
	{
		vec.Normalize();
	}
}

void TrackBall::Push(const float & x, const float & y)
{
	ScreenToWorld(x, y, lastPos3D);
}

void TrackBall::Move(const float & x, const float & y)
{
	Vector3f currentPos3D;
	ScreenToWorld(x, y, currentPos3D);

	m_axis = lastPos3D.Cross(currentPos3D);
	angle = 90 * m_axis.L2Norm();
	if (angle > 0)
	{
		m_axis.Normalize();
		Quaternion quat;
		quat.CreateByAngleAxis(angle, m_axis[0], m_axis[1], m_axis[2]);
		m_rotation = m_rotation*quat.GetConjugate();					// why?

		lastPos3D = currentPos3D;
	}
}