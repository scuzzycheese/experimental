#ifndef CVEC2_H
#define CVEC2_H

#include <math.h>

class cVec2
{
	public:
		float x, y;
		inline void set(float _x, float _y)
		{
			x = _x;
			y = _y;
		}

		cVec2(float _x, float _y) : x(_x), y(_y)
		{
		}
		cVec2() : x(0.0f), y(0.0f)
		{
		}
		cVec2(const cVec2 &v) : x(v.x), y(v.y)
		{
		}
		inline cVec2 operator *(const float s) const
		{
			return cVec2(x * s, y * s);
		}
		inline float operator *(const cVec2 &v) const
		{
			return v.x * x + v.y * y;
		}
		inline cVec2 operator +(const cVec2 &v) const
		{
			return cVec2(v.x + x, v.y + y);
		}
		inline cVec2 operator -(const cVec2 &v) const
		{
			return cVec2(x - v.x, y - v.y);
		}
		inline cVec2 operator -() const
		{
			return cVec2(-x, -y);
		}
		inline cVec2 operator ^(const cVec2 &n) const
		{
			const float len = 2.0f * (*this * n);
			const cVec2 newVec = n * len;
			return *this - newVec;
		}
		inline cVec2 &operator ^=(const cVec2 &n)
		{
			*this = *this ^ n;
			return *this;
		}
		//TODO: understand vector projection properly
		inline cVec2 project(const cVec2 &v)
		{
			const float dp = *this * v;
			cVec2 vec;
			vec.x = (dp / (v.x * v.x + v.y * v.y)) * v.x;
			vec.y = (dp / (v.x * v.x + v.y * v.y)) * v.y;
			return vec;
		}
		inline cVec2 operator /(const float div) const
		{
			const float idiv = 1.0f / div;
			return cVec2(x * idiv, y * idiv);
		}
		inline cVec2 &operator +=(const cVec2 &v)
		{
			*this = *this + v;
			return *this;
		}
		inline cVec2 &operator -=(const cVec2 &v)
		{
			*this = *this - v;
			return *this;
		}
		inline cVec2 &operator *=(const float s)
		{
			*this = *this * s;
			return *this;
		}
		inline cVec2 &operator /=(const float s)
		{
			*this = *this / s;
			return *this;
		}
		inline float lengthSqr()
		{
			return x * x + y * y;
		}
		inline float length()
		{
			return sqrtf(lengthSqr());
		}

		inline cVec2 &normalise()
		{
			float length = lengthSqr();
			if(length > 0)
			{
				length = sqrtf(length);
				*this /= length;
			}
			return *this;
		}

		inline cVec2 normalised()
		{
			float length = lengthSqr();
			cVec2 retVec = *this;
			if(length > 0)
			{
				length = sqrtf(length);
				retVec /= length;
			}
			return retVec;
		}

};

#endif
