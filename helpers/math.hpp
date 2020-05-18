#pragma once

#include "../valve_sdk/sdk.hpp"

#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

namespace Math
{
	float VectorDistance(const Vector& v1, const Vector& v2);
	float VectorLength(const Vector& v);
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] > 180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
	float NormalizeYaw(float yaw);
	void ClampAngles(QAngle& angles);
	Vector CrossProduct2(const Vector& a, const Vector& b);
	float RadToDeg(float flRadian);
	float DegToRad(float flDegree);
	void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	void AngleVectors(const QAngle& angles, Vector& forward);
	void AngleVectors(const QAngle& angles, Vector& forward, Vector& right, Vector& up);
	void VectorAngles(const Vector& forward, QAngle& angles);
	void VectorAngles(const Vector& forward, Vector& up, QAngle& angles);
	bool WorldToScreen(const Vector& in, Vector& out);
	float RandomFloat(float min, float max);
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *(float*)&i;
	}
	inline float DistancePointToLine(Vector Point, Vector LineOrigin, Vector Dir)
	{
		auto PointDir = Point - LineOrigin;

		auto TempOffset = PointDir.Dot(Dir) / (Dir.x * Dir.x + Dir.y * Dir.y + Dir.z * Dir.z);
		if (TempOffset < 0.000001f)
			return FLT_MAX;

		auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

		return (Point - PerpendicularPoint).Length();
	}
	inline float AngleDiff(float destAngle, float srcAngle)
	{
		float delta;

		delta = fmodf(destAngle - srcAngle, 360.0f);
		if (destAngle > srcAngle)
		{
			if (delta >= 180)
				delta -= 360;
		}
		else
		{
			if (delta <= -180)
				delta += 360;
		}
		return delta;
	}
	inline float anglemod(float a)
	{
		a = (360.f / 65536) * ((int)(a * (65536.f / 360.0f)) & 65535);
		return a;
	}
	inline float ApproachAngle(float target, float value, float speed)
	{
		target = anglemod(target);
		value = anglemod(value);

		float delta = target - value;

		// Speed is assumed to be positive
		if (speed < 0)
			speed = -speed;

		if (delta < -180)
			delta += 360;
		else if (delta > 180)
			delta -= 360;

		if (delta > speed)
			value += speed;
		else if (delta < -speed)
			value -= speed;
		else
			value = target;

		return value;
	}
	inline bool SanitizeAngle(QAngle& angles)
	{
		QAngle temp = angles;
		Normalize3(temp);
		ClampAngles(temp);

		if (!isfinite(temp.pitch) ||
			!isfinite(temp.yaw) ||
			!isfinite(temp.roll))
			return false;

		angles = temp;

		return true;
	}
}