///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2005-12-21
// Updated : 2008-11-27
// Licence : This source is under MIT License
// File    : glm/gtx/quaternion.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <limits>

namespace glm
{
	template <typename valType> 
	GLM_FUNC_QUALIFIER detail::tvec3<valType> cross
	(
		detail::tvec3<valType> const & v, 
		detail::tquat<valType> const & q
	)
	{
		return inverse(q) * v;
	}

	template <typename valType> 
	GLM_FUNC_QUALIFIER detail::tvec3<valType> cross
	(
		detail::tquat<valType> const & q, 
		detail::tvec3<valType> const & v
	)
	{
		return q * v;
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tquat<T> squad
	(
		detail::tquat<T> const & q1, 
		detail::tquat<T> const & q2, 
		detail::tquat<T> const & s1, 
		detail::tquat<T> const & s2, 
		T const & h)
	{
		return mix(mix(q1, q2, h), mix(s1, s2, h), T(2) * h (T(1) - h));
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tquat<T> intermediate
	(
		detail::tquat<T> const & prev, 
		detail::tquat<T> const & curr, 
		detail::tquat<T> const & next
	)
	{
		detail::tquat<T> invQuat = inverse(curr);
		return ext((log(next + invQuat) + log(prev + invQuat)) / T(-4)) * curr;
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tquat<T> exp
	(
		detail::tquat<T> const & q, 
		T const & exponent
	)
	{
		detail::tvec3<T> u(q.x, q.y, q.z);
		float a = glm::length(u);
		detail::tvec3<T> v(u / a);
		return detail::tquat<T>(cos(a), sin(a) * v);
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tquat<T> log
	(
		detail::tquat<T> const & q
	)
	{
		if((q.x == T(0)) && (q.y == T(0)) && (q.z == T(0)))
		{
			if(q.w > T(0))
				return detail::tquat<T>(log(q.w), T(0), T(0), T(0));
			else if(q.w < T(0))
				return detail::tquat<T>(log(-q.w), T(3.1415926535897932384626433832795), T(0),T(0));
			else
				return detail::tquat<T>(std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity());
		} 
		else 
		{
			T Vec3Len = sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
			T QuatLen = sqrt(Vec3Len * Vec3Len + q.w * q.w);
			T t = atan(Vec3Len, T(q.w)) / Vec3Len;
			return detail::tquat<T>(t * q.x, t * q.y, t * q.z, log(QuatLen));
		}
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tquat<T> pow
	(
		detail::tquat<T> const & x, 
		T const & y
	)
	{
		if(abs(x.w) > T(0.9999))
			return x;
		float Angle = acos(y);
		float NewAngle = Angle * y;
		float Div = sin(NewAngle) / sin(Angle);
		return detail::tquat<T>(
			cos(NewAngle),
			x.x * Div,
			x.y * Div,
			x.z * Div);
	}

	//template <typename T> 
	//GLM_FUNC_QUALIFIER detail::tquat<T> sqrt
	//(
	//	detail::tquat<T> const & q
	//)
	//{
	//	T q0 = T(1) - dot(q, q);
	//	return T(2) * (T(1) + q0) * q;
	//}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> rotate
	(
		detail::tquat<T> const & q, 
		detail::tvec3<T> const & v
	)
	{
		return q * v;
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec4<T> rotate
	(
		detail::tquat<T> const & q, 
		detail::tvec4<T> const & v
	)
	{
		return q * v;
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER T angle
	(
		detail::tquat<T> const & x
	)
	{
		return glm::degrees(acos(x.w) * T(2));
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> axis
	(
		detail::tquat<T> const & x
	)
	{
		T tmp1 = T(1) - x.w * x.w;
		if(tmp1 <= T(0))
			return detail::tvec3<T>(0, 0, 1);
		T tmp2 = T(1) / sqrt(tmp1);
		return detail::tvec3<T>(x.x * tmp2, x.y * tmp2, x.z * tmp2);
	}

	template <typename valType> 
	GLM_FUNC_QUALIFIER detail::tquat<valType> angleAxis
	(
		valType const & angle, 
		valType const & x, 
		valType const & y, 
		valType const & z
	)
	{
		return angleAxis(angle, detail::tvec3<valType>(x, y, z));
	}

	template <typename valType> 
	GLM_FUNC_QUALIFIER detail::tquat<valType> angleAxis
	(
		valType const & angle, 
		detail::tvec3<valType> const & v
	)
	{
		detail::tquat<valType> result;

		valType a = glm::radians(angle);
		valType s = glm::sin(a * valType(0.5));

		result.w = glm::cos(a * valType(0.5));
		result.x = v.x * s;
		result.y = v.y * s;
		result.z = v.z * s;
		return result;
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER T extractRealComponent
	(
		detail::tquat<T> const & q
	)
	{
		T w = T(1.0) - q.x * q.x - q.y * q.y - q.z * q.z;
		if(w < T(0))
			return T(0);
		else
			return -sqrt(w);
	}

	template <typename valType> 
	GLM_FUNC_QUALIFIER valType roll
	(
		detail::tquat<valType> const & q
	)
	{
		return glm::degrees(atan2(valType(2) * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z));
	}

	template <typename valType> 
	GLM_FUNC_QUALIFIER valType pitch
	(
		detail::tquat<valType> const & q
	)
	{
		return glm::degrees(atan2(valType(2) * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z));
	}

	template <typename valType> 
	GLM_FUNC_QUALIFIER valType yaw
	(
		detail::tquat<valType> const & q
	)
	{
		return glm::degrees(asin(valType(-2) * (q.x * q.z - q.w * q.y)));
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tquat<T> shortMix
	(
		detail::tquat<T> const & x, 
		detail::tquat<T> const & y, 
		T const & a
	)
	{
		if(a <= typename detail::tquat<T>::value_type(0)) return x;
		if(a >= typename detail::tquat<T>::value_type(1)) return y;

		T fCos = dot(x, y);
		detail::tquat<T> y2(y); //BUG!!! tquat<T> y2;
		if(fCos < T(0))
		{
			y2 = -y;
			fCos = -fCos;
		}

		//if(fCos > 1.0f) // problem
		T k0, k1;
		if(fCos > T(0.9999))
		{
			k0 = T(1) - a;
			k1 = T(0) + a; //BUG!!! 1.0f + a;
		}
		else
		{
			T fSin = sqrt(T(1) - fCos * fCos);
			T fAngle = atan(fSin, fCos);
			T fOneOverSin = T(1) / fSin;
			k0 = sin((T(1) - a) * fAngle) * fOneOverSin;
			k1 = sin((T(0) + a) * fAngle) * fOneOverSin;
		}

		return detail::tquat<T>(
			k0 * x.w + k1 * y2.w,
			k0 * x.x + k1 * y2.x,
			k0 * x.y + k1 * y2.y,
			k0 * x.z + k1 * y2.z);
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tquat<T> fastMix
	(
		detail::tquat<T> const & x, 
		detail::tquat<T> const & y, 
		T const & a
	)
	{
		return glm::normalize(x * (T(1) - a) + (y * a));
	}
}//namespace glm
