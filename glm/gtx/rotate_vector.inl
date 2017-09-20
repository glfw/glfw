///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2006-11-02
// Updated : 2009-02-19
// Licence : This source is under MIT License
// File    : glm/gtx/rotate_vector.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace glm
{
	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec2<T> rotate
	(
		detail::tvec2<T> const & v, 
		T const & angle
	)
	{
		detail::tvec2<T> Result;
		T const Cos = cos(radians(angle));
		T const Sin = sin(radians(angle));
		Result.x = v.x * Cos - v.y * Sin;
		Result.y = v.x * Sin + v.y * Cos;
		return Result;
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> rotate
	(
		detail::tvec3<T> const & v, 
		T const & angle, 
		detail::tvec3<T> const & normal
	)
	{
		return detail::tmat3x3<T>(glm::rotate(angle, normal)) * v;
	}
	/*
	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> rotateGTX(
		const detail::tvec3<T>& x, 
		T angle, 
		const detail::tvec3<T>& normal)
	{
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		return x * Cos + ((x * normal) * (T(1) - Cos)) * normal + cross(x, normal) * Sin;
	}
	*/
	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec4<T> rotate
	(
		detail::tvec4<T> const & v, 
		T const & angle, 
		detail::tvec3<T> const & normal
	)
	{
		return rotate(angle, normal) * v;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec3<T> rotateX
	(
		detail::tvec3<T> const & v, 
		T const & angle
	)
	{
		detail::tvec3<T> Result = v;
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		Result.y = v.y * Cos - v.z * Sin;
		Result.z = v.y * Sin + v.z * Cos;
		return Result;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec3<T> rotateY
	(
		detail::tvec3<T> const & v, 
		T const & angle
	)
	{
		detail::tvec3<T> Result = v;
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		Result.x =  v.x * Cos + v.z * Sin;
		Result.z = -v.x * Sin + v.z * Cos;
		return Result;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec3<T> rotateZ
	(
		detail::tvec3<T> const & v, 
		T const & angle
	)
	{
		detail::tvec3<T> Result = v;
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		Result.x = v.x * Cos - v.y * Sin;
		Result.y = v.x * Sin + v.y * Cos;
		return Result;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec4<T> rotateX
	(
		detail::tvec4<T> const & v, 
		T const & angle
	)
	{
		detail::tvec4<T> Result = v;
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		Result.y = v.y * Cos - v.z * Sin;
		Result.z = v.y * Sin + v.z * Cos;
		return Result;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec4<T> rotateY
	(
		detail::tvec4<T> const & v, 
		T const & angle
	)
	{
		detail::tvec4<T> Result = v;
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		Result.x =  v.x * Cos + v.z * Sin;
		Result.z = -v.x * Sin + v.z * Cos;
		return Result;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec4<T> rotateZ
	(
		detail::tvec4<T> const & v, 
		T const & angle
	)
	{
		detail::tvec4<T> Result = v;
		const T Cos = cos(radians(angle));
		const T Sin = sin(radians(angle));
		Result.x = v.x * Cos - v.y * Sin;
		Result.y = v.x * Sin + v.y * Cos;
		return Result;
	}

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tmat4x4<T> orientation
	(
		detail::tvec3<T> const & Normal, 
		detail::tvec3<T> const & Up
	)
	{
		if(all(equal(Normal, Up)))
			return detail::tmat4x4<T>(T(1));

		detail::tvec3<T> RotationAxis = cross(Up, Normal);
		T Angle = degrees(acos(dot(Normal, Up)));
		return rotate(Angle, RotationAxis);
	}
}//namespace glm
