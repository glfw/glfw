///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2005-12-21
// Updated : 2011-06-07
// Licence : This source is under MIT License
// File    : glm/gtx/epsilon.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace glm
{
	template <typename genType>
	GLM_FUNC_QUALIFIER bool equalEpsilon
	(
		genType const & x, 
		genType const & y, 
		genType const & epsilon
	)
	{
		return abs(x - y) < epsilon;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER bool notEqualEpsilon
	(
		genType const & x, 
		genType const & y, 
		genType const & epsilon
	)
	{
		return abs(x - y) >= epsilon;
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec2<bool> equalEpsilon
	(
		detail::tvec2<valType> const & x, 
		detail::tvec2<valType> const & y, 
		valType const & epsilon)
	{
		return detail::tvec2<bool>(
			abs(x.x - y.x) < epsilon,
			abs(x.y - y.y) < epsilon);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec3<bool> equalEpsilon
	(
		detail::tvec3<valType> const & x, 
		detail::tvec3<valType> const & y, 
		valType const & epsilon)
	{
		return detail::tvec3<bool>(
			abs(x.x - y.x) < epsilon,
			abs(x.y - y.y) < epsilon,
			abs(x.z - y.z) < epsilon);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec4<bool> equalEpsilon
	(
		detail::tvec4<valType> const & x, 
		detail::tvec4<valType> const & y, 
		valType const & epsilon
	)
	{
		return detail::tvec4<bool>(
			abs(x.x - y.x) < epsilon,
			abs(x.y - y.y) < epsilon,
			abs(x.z - y.z) < epsilon,
			abs(x.w - y.w) < epsilon);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec2<bool> notEqualEpsilon
	(
		detail::tvec2<valType> const & x, 
		detail::tvec2<valType> const & y, 
		valType const & epsilon
	)
	{
		return detail::tvec2<bool>(
			abs(x.x - y.x) >= epsilon,
			abs(x.y - y.y) >= epsilon);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec3<bool> notEqualEpsilon
	(
		detail::tvec3<valType> const & x, 
		detail::tvec3<valType> const & y, 
		valType const & epsilon
	)
	{
		return detail::tvec3<bool>(
			abs(x.x - y.x) >= epsilon,
			abs(x.y - y.y) >= epsilon,
			abs(x.z - y.z) >= epsilon);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec4<bool> notEqualEpsilon
	(
		detail::tvec4<valType> const & x, 
		detail::tvec4<valType> const & y, 
		valType const & epsilon
	)
	{
		return detail::tvec4<bool>(
			abs(x.x - y.x) >= epsilon,
			abs(x.y - y.y) >= epsilon,
			abs(x.z - y.z) >= epsilon,
			abs(x.w - y.w) >= epsilon);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec2<bool> equalEpsilon
	(
		detail::tvec2<valType> const & x, 
		detail::tvec2<valType> const & y, 
		detail::tvec2<valType> const & epsilon
	)
	{
		return detail::tvec2<bool>(
			abs(x.x - y.x) < epsilon.x,
			abs(x.y - y.y) < epsilon.y);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec3<bool> equalEpsilon
	(
		detail::tvec3<valType> const & x, 
		detail::tvec3<valType> const & y, 
		detail::tvec3<valType> const & epsilon
	)
	{
		return detail::tvec3<bool>(
			abs(x.x - y.x) < epsilon.x,
			abs(x.y - y.y) < epsilon.y,
			abs(x.z - y.z) < epsilon.z);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec4<bool> equalEpsilon
	(
		detail::tvec4<valType> const & x, 
		detail::tvec4<valType> const & y, 
		detail::tvec4<valType> const & epsilon
	)
	{
		return detail::tvec4<bool>(
			abs(x.x - y.x) < epsilon.x,
			abs(x.y - y.y) < epsilon.y,
			abs(x.z - y.z) < epsilon.z,
			abs(x.w - y.w) < epsilon.w);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec4<bool> equalEpsilon
	(
		detail::tquat<valType> const & x, 
		detail::tquat<valType> const & y, 
		detail::tquat<valType> const & epsilon
	)
	{
		return detail::tvec4<bool>(
			abs(x.x - y.x) < epsilon.x,
			abs(x.y - y.y) < epsilon.y,
			abs(x.z - y.z) < epsilon.z,
			abs(x.w - y.w) < epsilon.w);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec2<bool> notEqualEpsilon
	(
		detail::tvec2<valType> const & x, 
		detail::tvec2<valType> const & y, 
		detail::tvec2<valType> const & epsilon
	)
	{
		return detail::tvec2<bool>(
			abs(x.x - y.x) >= epsilon.x,
			abs(x.y - y.y) >= epsilon.y);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec3<bool> notEqualEpsilon
	(
		detail::tvec3<valType> const & x, 
		detail::tvec3<valType> const & y, 
		detail::tvec3<valType> const & epsilon
	)
	{
		return detail::tvec3<bool>(
			abs(x.x - y.x) >= epsilon.x,
			abs(x.y - y.y) >= epsilon.y,
			abs(x.z - y.z) >= epsilon.z);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec4<bool> notEqualEpsilon
	(
		detail::tvec4<valType> const & x, 
		detail::tvec4<valType> const & y, 
		detail::tvec4<valType> const & epsilon
	)
	{
		return detail::tvec4<bool>(
			abs(x.x - y.x) >= epsilon.x,
			abs(x.y - y.y) >= epsilon.y,
			abs(x.z - y.z) >= epsilon.z,
			abs(x.w - y.w) >= epsilon.w);
	}

	template <typename valType>
	GLM_FUNC_QUALIFIER detail::tvec4<bool> notEqualEpsilon
	(
		detail::tquat<valType> const & x, 
		detail::tquat<valType> const & y, 
		detail::tquat<valType> const & epsilon
	)
	{
		return detail::tvec4<bool>(
			abs(x.x - y.x) >= epsilon.x,
			abs(x.y - y.y) >= epsilon.y,
			abs(x.z - y.z) >= epsilon.z,
			abs(x.w - y.w) >= epsilon.w);
	}
}//namespace glm
