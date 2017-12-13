///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2008-10-09
// Updated : 2011-10-14
// Licence : This source is under MIT License
// File    : glm/gtx/reciprocal.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "../core/_vectorize.hpp"

namespace glm
{
	// sec
	template <typename genType>
	GLM_FUNC_QUALIFIER genType sec
	(
		genType const & angle
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'sec' only accept floating-point values");

		return genType(1) / glm::cos(angle);
	}

	VECTORIZE_VEC(sec)

	// csc
	template <typename genType>
	GLM_FUNC_QUALIFIER genType csc
	(
		genType const & angle
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'csc' only accept floating-point values");

		return genType(1) / glm::sin(angle);
	}

	VECTORIZE_VEC(csc)

	// cot
	template <typename genType>
	GLM_FUNC_QUALIFIER genType cot
	(
		genType const & angle
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'cot' only accept floating-point values");

		return genType(1) / glm::tan(angle);
	}

	VECTORIZE_VEC(cot)

	// asec
	template <typename genType>
	GLM_FUNC_QUALIFIER genType asec
	(
		genType const & x
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'asec' only accept floating-point values");
	
		return acos(genType(1) / x);
	}

	VECTORIZE_VEC(asec)

	// acsc
	template <typename genType>
	GLM_FUNC_QUALIFIER genType acsc
	(
		genType const & x
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'acsc' only accept floating-point values");

		return asin(genType(1) / x);
	}

	VECTORIZE_VEC(acsc)

	// acot
	template <typename genType>
	GLM_FUNC_QUALIFIER genType acot
	(
		genType const & x
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'acot' only accept floating-point values");

		genType const pi_over_2 = genType(3.1415926535897932384626433832795 / 2.0);
		return pi_over_2 - atan(x);
	}

	VECTORIZE_VEC(acot)

	// sech
	template <typename genType>
	GLM_FUNC_QUALIFIER genType sech
	(
		genType const & angle
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'sech' only accept floating-point values");

		return genType(1) / glm::cosh(angle);
	}

	VECTORIZE_VEC(sech)

	// csch
	template <typename genType>
	GLM_FUNC_QUALIFIER genType csch
	(
		genType const & angle
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'csch' only accept floating-point values");

		return genType(1) / glm::sinh(angle);
	}

	VECTORIZE_VEC(csch)

	// coth
	template <typename genType>
	GLM_FUNC_QUALIFIER genType coth
	(
		genType const & angle
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'coth' only accept floating-point values");

		return glm::cosh(angle) / glm::sinh(angle);
	}

	VECTORIZE_VEC(coth)

	// asech
	template <typename genType>
	GLM_FUNC_QUALIFIER genType asech
	(
		genType const & x
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'asech' only accept floating-point values");

		return acosh(genType(1) / x);
	}

	VECTORIZE_VEC(asech)

	// acsch
	template <typename genType>
	GLM_FUNC_QUALIFIER genType acsch
	(
		genType const & x
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'acsch' only accept floating-point values");

		return asinh(genType(1) / x);
	}

	VECTORIZE_VEC(acsch)

	// acoth
	template <typename genType>
	GLM_FUNC_QUALIFIER genType acoth
	(
		genType const & x
	)
	{
		GLM_STATIC_ASSERT(detail::type<genType>::is_float, "'acoth' only accept floating-point values");

		return atanh(genType(1) / x);
	}

	VECTORIZE_VEC(acoth)
}//namespace glm
