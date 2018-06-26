#pragma once
#include <limits>
#include "lxnet\base\platform_config.h"

const float  MinFloat = (std::numeric_limits<float>::min)();

inline bool isEqual(float a, float b)
{
	if (fabs(a - b) < 1E-12)
	{
		return true;
	}

	return false;
}

struct Vector3D
{
	float x;
	float y;
	float z;

	Vector3D() :x(0.0), y(0.0), z(0.0) {}
	Vector3D(float a, float b, float c) :x(a), y(b), z(c) {}

	// 归零
	void Zero() { x = 0.0; y = 0.0; z = 0.0; }

	// 是否为0
	bool isZero()const { return (x * x + y * y + z * z) < MinFloat; }

	// 长度
	inline float Length()const;

	// 平方长度
	inline float LengthSq()const;

	// 向量归一
	inline void Normalize();

	// 点乘
	inline float Dot(const Vector3D& v2)const;

	// 是否顺时针方向
	inline int32 Sign(const Vector3D& v2)const;

	// 垂直向量
	inline Vector3D Perp()const;

	// 矢量最大长度
	inline void Truncate(float max);

	// 距离
	inline float Distance(const Vector3D &v2)const;

	// 距离的平方
	inline float DistanceSq(const Vector3D &v2)const;
	
	// 相反向量
	inline Vector3D GetReverse()const;


	const Vector3D& operator+=(const Vector3D &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;

		return *this;
	}

	const Vector3D& operator-=(const Vector3D &rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;

		return *this;
	}

	const Vector3D& operator*=(const float& rhs)
	{
		x *= rhs;
		y *= rhs;
		z *= rhs;

		return *this;
	}

	const Vector3D& operator/=(const float& rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;

		return *this;
	}

	bool operator==(const Vector3D& rhs)const
	{
		return (isEqual(x, rhs.x) && isEqual(y, rhs.y) && isEqual(z, rhs.z));
	}

	bool operator!=(const Vector3D& rhs)const
	{
		return (fabs(x - rhs.x) > MinFloat) || (fabs(y - rhs.y) > MinFloat) || (fabs(z - rhs.z) > MinFloat);
	}

};

inline Vector3D operator*(const Vector3D &lhs, float rhs);
inline Vector3D operator*(float lhs, const Vector3D &rhs);
inline Vector3D operator-(const Vector3D &lhs, const Vector3D &rhs);
inline Vector3D operator+(const Vector3D &lhs, const Vector3D &rhs);
inline Vector3D operator/(const Vector3D &lhs, float val);

inline float Vector3D::Length()const
{
	return sqrt(x * x + y * y + z * z);
}

inline float Vector3D::LengthSq()const
{
	return (x * x + y * y + z * z);
}

inline float Vector3D::Dot(const Vector3D &v2)const
{
	return x * v2.x + y * v2.y + z * v2.z;
}

enum { clockwise = 1, anticlockwise = -1 };

inline int Vector3D::Sign(const Vector3D& v2)const
{
	if (y*v2.x > x*v2.y)
	{
		return anticlockwise;
	}
	else
	{
		return clockwise;
	}
}

inline Vector3D Vector3D::Perp()const
{
	Vector3D vecReturn;
	if (fabs(y) < fabs(z))
	{
		vecReturn.x = z;
		vecReturn.y = 0.0;
		vecReturn.z = -x;
	}
	else
	{
		vecReturn.x = -y;
		vecReturn.y = x;
		vecReturn.z = 0.0;
	}
	return vecReturn;
}

inline float Vector3D::Distance(const Vector3D &v2)const
{
	float ySeparation = v2.y - y;
	float xSeparation = v2.x - x;
	float zSeparation = v2.z - z;

	return sqrt(ySeparation*ySeparation + xSeparation * xSeparation + zSeparation * zSeparation);
}

inline float Vector3D::DistanceSq(const Vector3D &v2)const
{
	float ySeparation = v2.y - y;
	float xSeparation = v2.x - x;
	float zSeparation = v2.z - z;

	return ySeparation * ySeparation + xSeparation * xSeparation + zSeparation * zSeparation;
}

inline void Vector3D::Truncate(float max)
{
	if (this->Length() > max)
	{
		this->Normalize();

		*this *= max;
	}
}

inline Vector3D Vector3D::GetReverse()const
{
	return Vector3D(-this->x, -this->y, -this->z);
}

inline void Vector3D::Normalize()
{
	float vector_length = this->Length();

	if (vector_length > std::numeric_limits<float>::epsilon())
	{
		this->x /= vector_length;
		this->y /= vector_length;
		this->z /= vector_length;
	}
}

inline Vector3D Vec3DNormalize(const Vector3D &v)
{
	Vector3D vec = v;

	float vector_length = vec.Length();

	if (vector_length > std::numeric_limits<float>::epsilon())
	{
		vec.x /= vector_length;
		vec.y /= vector_length;
		vec.z /= vector_length;
	}

	return vec;
}


inline float Vec3DDistance(const Vector3D &v1, const Vector3D &v2)
{

	float ySeparation = v2.y - v1.y;
	float xSeparation = v2.x - v1.x;
	float zSeparation = v2.z - v1.z;

	return sqrt(ySeparation*ySeparation + xSeparation * xSeparation + zSeparation * zSeparation);
}

inline float Vec3DDistanceSq(const Vector3D &v1, const Vector3D &v2)
{

	float ySeparation = v2.y - v1.y;
	float xSeparation = v2.x - v1.x;
	float zSeparation = v2.z - v1.z;

	return ySeparation * ySeparation + xSeparation * xSeparation + zSeparation * zSeparation;
}

inline float Vec3DLength(const Vector3D& v)
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline float Vec3DLengthSq(const Vector3D& v)
{
	return (v.x*v.x + v.y*v.y + v.z*v.z);
}

inline Vector3D operator*(const Vector3D &lhs, float rhs)
{
	Vector3D result(lhs);
	result *= rhs;
	return result;
}

inline Vector3D operator*(float lhs, const Vector3D &rhs)
{
	Vector3D result(rhs);
	result *= lhs;
	return result;
}

inline Vector3D operator-(const Vector3D &lhs, const Vector3D &rhs)
{
	Vector3D result(lhs);
	result.x -= rhs.x;
	result.y -= rhs.y;
	result.z -= rhs.z;

	return result;
}

inline Vector3D operator+(const Vector3D &lhs, const Vector3D &rhs)
{
	Vector3D result(lhs);
	result.x += rhs.x;
	result.y += rhs.y;
	result.z += rhs.z;

	return result;
}

inline Vector3D operator/(const Vector3D &lhs, float val)
{
	Vector3D result(lhs);
	result.x /= val;
	result.y /= val;
	result.z /= val;

	return result;
}
