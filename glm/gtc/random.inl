//////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
//////////////////////////////////////////////////////////////////////////////////
// Created : 2011-09-19
// Updated : 2011-09-19
// Licence : This source is under MIT License
// File    : glm/gtc/random.inl
//////////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <cassert>
#include "../core/_vectorize.hpp"

namespace glm{
namespace detail
{
	struct compute_linearRand
	{
		template <typename T>
		GLM_FUNC_QUALIFIER T operator() (T const & Min, T const & Max) const;
/*
		{
			GLM_STATIC_ASSERT(0, "'linearRand' invalid template parameter type. GLM_GTC_random only supports floating-point template types.");
			return Min;
		}
*/
	};
    
	template <>
	GLM_FUNC_QUALIFIER half compute_linearRand::operator()<half> (half const & Min, half const & Max) const
	{
		return half(float(std::rand()) / float(RAND_MAX) * (float(Max) - float(Min)) + float(Min));
	}

	template <>
	GLM_FUNC_QUALIFIER float compute_linearRand::operator()<float> (float const & Min, float const & Max) const
	{
		return float(std::rand()) / float(RAND_MAX) * (Max - Min) + Min;
	}

	template <>
	GLM_FUNC_QUALIFIER double compute_linearRand::operator()<double> (double const & Min, double const & Max) const
	{
		return double(std::rand()) / double(RAND_MAX) * (Max - Min) + Min;
	}
    
	template <>
	GLM_FUNC_QUALIFIER long double compute_linearRand::operator()<long double> (long double const & Min, long double const & Max) const
	{
		return (long double)(std::rand()) / (long double)(RAND_MAX) * (Max - Min) + Min;
	}
}//namespace detail

	template <typename genType> 
	GLM_FUNC_QUALIFIER genType linearRand
	(
		genType const & Min, 
		genType const & Max
	)
	{
		return detail::compute_linearRand()(Min, Max);
	}

	VECTORIZE_VEC_VEC(linearRand)

	template <typename genType> 
	GLM_FUNC_QUALIFIER genType gaussRand
	(
		genType const & Mean,	
		genType const & Deviation
	)
	{
		genType w, x1, x2;
	
		do
		{
			x1 = linearRand(genType(-1), genType(1));
			x2 = linearRand(genType(-1), genType(1));
		
			w = x1 * x1 + x2 * x2;
		} while(w > genType(1));
	
		return x2 * Deviation * Deviation * sqrt((genType(-2) * log(w)) / w) + Mean;
	}

	VECTORIZE_VEC_VEC(gaussRand)

	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec2<T> diskRand
	(
		T const & Radius
	)
	{		
		detail::tvec2<T> Result(T(0));
		T LenRadius(T(0));
		
		do
		{
			Result = linearRand(detail::tvec2<T>(-Radius), detail::tvec2<T>(Radius));
			LenRadius = length(Result);
		}
		while(LenRadius > Radius);
		
		return Result;
	}
	
	template <typename T>
	GLM_FUNC_QUALIFIER detail::tvec3<T> ballRand
	(
		T const & Radius
	)
	{		
		detail::tvec3<T> Result(T(0));
		T LenRadius(T(0));
		
		do
		{
			Result = linearRand(detail::tvec3<T>(-Radius), detail::tvec3<T>(Radius));
			LenRadius = length(Result);
		}
		while(LenRadius > Radius);
		
		return Result;
	}
	
	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec2<T> circularRand
	(
		T const & Radius
	)
	{
		T a = linearRand(T(0), T(6.283185307179586476925286766559f));
		return detail::tvec2<T>(cos(a), sin(a)) * Radius;		
	}
	
	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> sphericalRand
	(
		T const & Radius
	)
	{
		T z = linearRand(T(-1), T(1));
		T a = linearRand(T(0), T(6.283185307179586476925286766559f));
	
		T r = sqrt(T(1) - z * z);
	
		T x = r * cos(a);
		T y = r * sin(a);
	
		return detail::tvec3<T>(x, y, z) * Radius;	
	}
}//namespace glm
