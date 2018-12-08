#include "Quaternion.h"

Quaternion::Quaternion()
{
}

Quaternion::Quaternion(const Quaternion & quat)
{
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
	this->w = quat.w;
}

Quaternion::Quaternion(const float & x, const float & y, const float & z, const float & w)
	:x(x), y(y), z(z), w(w)
{
}

Quaternion::~Quaternion()
{
}

void Quaternion::LoadIdentity()
{
	x = y = z = 0.0f;
	w = 1.0f;
}

void Quaternion::CreateByAngleAxis(float angle, float x, float y, float z)
{
	float len = sqrt(x*x + y*y + z*z);
	float radian = (float)(angle*PI / 180.0f);
	float ccc = cos(0.5f*radian);
	float sss = sin(0.5f*radian);


	x /= len;
	y /= len;
	z /= len;
	
	this->x = sss*x;
	this->y = sss*y;
	this->z = sss*z;
	this->w = ccc;
}

void Quaternion::RotateVector(float x, float y, float z, float & newX, float & newY, float & newZ)
{
	Quaternion p,newP;
	p.PutXYZtoQuaternion(x, y, z);
	Quaternion quat_tmp = p*this->GetConjugate();
	newP = (*this)*quat_tmp;
	newX = newP.x;
	newY = newP.y;
	newZ = newP.z;
}

Quaternion  Quaternion::GetConjugate()
{
	return Quaternion(-x, -y, -z, w);
}

// Convert to Matrix
void Quaternion::GetMatrix(float * matrix)
{
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;


	// This calculation would be a lot more complicated for non-unit length quaternions
	// Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
	//   OpenGL

	matrix[0] = 1.0f - 2.0f * (y2 + z2);
	matrix[1] = 2.0f * (xy - wz);
	matrix[2] = 2.0f * (xz + wy);
	matrix[3] = 0.0f;

	matrix[4] = 2.0f * (xy + wz);
	matrix[5] = 1.0f - 2.0f * (x2 + z2);
	matrix[6] = 2.0f * (yz - wx);
	matrix[7] = 0.0f;

	matrix[8] = 2.0f * (xz - wy);
	matrix[9] = 2.0f * (yz + wx);
	matrix[10] = 1.0f - 2.0f * (x2 + y2);
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;
	/*
		1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
		2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
		2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	*/
}

void Quaternion::GetAxisAngle(float &axisX, float &axisY, float &axisZ, float &angle)
{
	float scale = sqrt(x * x + y * y + z * z);
	axisX = x / scale;
	axisY = y / scale;
	axisZ = z / scale;
	angle = (float)(acos(w) * 2.0f * 180.0f / PI);
}

Quaternion Quaternion::operator* (const Quaternion &rq) const
{
	// the constructor takes its arguments as (x, y, z, w)
	return Quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
		w * rq.y + y * rq.w + z * rq.x - x * rq.z,
		w * rq.z + z * rq.w + x * rq.y - y * rq.x,
		w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}

Quaternion & Quaternion::operator=(const Quaternion &quat)
{

	if (this == &quat)
	{
		return *this;
	}

	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
	this->w = quat.w;

	return *this;
}

void Quaternion::PutXYZtoQuaternion(const float & x, const float & y, const float & z)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = 0;
	//Normalize();
}

void Quaternion::Normalize()
{
	float len = sqrt(x*x + y*y + z*z + w*w);

	x /= len;
	y /= len;
	z /= len;
	w /= len;

}

