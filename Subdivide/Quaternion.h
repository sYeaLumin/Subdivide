// Class: Quaternion, written by XiaojieXu, 20160427
#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include <iostream>
using namespace std;

#ifndef PI
#define PI 3.1415926535898
#endif

class Quaternion
{
public:
	void LoadIdentity();
	void CreateByAngleAxis(float angle, float x, float y, float z);
	void RotateVector(float x, float y, float z, float & newX, float & newY, float & newZ); // roate [x y z]' by this quaternion, and get the new vector [newX newY newZ]
	Quaternion GetConjugate();
	void GetMatrix(float * matrix);
	void GetAxisAngle(float &axisX, float &axisY, float &axisZ, float &angle);
	Quaternion operator* (const Quaternion &rq) const;
	Quaternion & operator=(const Quaternion &quat);

	Quaternion();
	Quaternion(const Quaternion & quat);
	Quaternion(const float & x, const float & y, const float & z, const float & w);
	~Quaternion();

public:
	float x, y, z, w;

private:
	void PutXYZtoQuaternion(const float & x, const float & y, const float & z);
	void Normalize();
};
#endif

