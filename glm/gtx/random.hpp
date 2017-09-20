///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Mathematics (glm.g-truc.net)
///
/// Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref gtx_random
/// @file glm/gtx/random.hpp
/// @date 2006-01-16 / 2011-06-07
/// @author Christophe Riccio
///
/// @see core (dependence)
/// @see gtc_half_float (dependence)
///
/// @defgroup gtx_random GLM_GTX_random: Random
/// @ingroup gtx
/// 
/// @brief Generate random number from various distribution methods
/// 
/// <glm/gtx/random.hpp> need to be included to use these functionalities.
///////////////////////////////////////////////////////////////////////////////////

#ifndef GLM_GTX_random
#define GLM_GTX_random GLM_VERSION

// Dependency:
#include "../glm.hpp"
#include "../gtc/random.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#	pragma message("GLM: GLM_GTX_random extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_random
	/// @{

	/// Generate a random number in the interval [-1, 1], according a linear distribution.
	/// From GLM_GTX_random extension.
    template <typename T> T signedRand1();
	
	template <> float signedRand1(); //!< \brief Generate a random number in the interval [-1, 1], according a linear distribution (From GLM_GTX_random extension)
    template <> double signedRand1(); //!< \brief Generate a random number in the interval [-1, 1], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> signedRand2(); //!< \brief Generate 2 random numbers in the interval [-1, 1], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> signedRand3(); //!< \brief Generate 3 random numbers in the interval [-1, 1], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> signedRand4(); //!< \brief Generate 4 random numbers in the interval [-1, 1], according a linear distribution (From GLM_GTX_random extension)
    
	template <typename T> detail::tvec2<T> normalizedRand2(); //!< \brief Generate a normalized 2D vector regulary distribute on a circle (From GLM_GTX_random extension)
	template <typename T> detail::tvec2<T> normalizedRand2(T Min, T Max); //!< \brief Generate a scaled and normalized 2D vector regulary distribute on a circle (From GLM_GTX_random extension)
	template <typename T> detail::tvec3<T> normalizedRand3(); //!< \brief Generate a normalized 3D vector regulary distribute on a sphere (From GLM_GTX_random extension)
	template <typename T> detail::tvec3<T> normalizedRand3(T Min, T Max); //!< \brief Generate a scaled and normalized 3D vector regulary distribute on a sphere (From GLM_GTX_random extension)

    template <typename T> T compRand1(); //!< \brief Generate a random number in the interval [0, 1], according a linear distribution (From GLM_GTX_random extension)
	template <> float compRand1(); //!< \brief Generate a random number in the interval [0, 1], according a linear distribution (From GLM_GTX_random extension)
    template <> double compRand1(); //!< \brief Generate a random number in the interval [0, 1], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> T compRand1(T Min, T Max); //!< \brief Generate a random number in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> compRand2(T Min, T Max); //!< \brief Generate 2 random numbers in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> compRand3(T Min, T Max); //!< \brief Generate 3 random numbers in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> compRand4(T Min, T Max); //!< \brief Generate 4 random numbers in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> compRand2(const detail::tvec2<T>& Min, const detail::tvec2<T>& Max); //!< \brief Generate 2 random numbers in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> compRand3(const detail::tvec3<T>& Min, const detail::tvec3<T>& Max); //!< \brief Generate 3 random numbers in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> compRand4(const detail::tvec4<T>& Min, const detail::tvec4<T>& Max); //!< \brief Generate 4 random numbers in the interval [Min, Max], according a linear distribution (From GLM_GTX_random extension)

    template <typename T> detail::tvec2<T> vecRand2(); //!< \brief Generate a random normalized 2 component vector. It's a spherical uniform distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> vecRand2(T MinRadius, T MaxRadius); //!< \brief Generate a random normalized 2 component vector. It's a spherical uniform distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> vecRand3(); //!< \brief Generate a random normalized 3 component vector. It's a spherical uniform distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> vecRand3(T MinRadius, T MaxRadius); //!< \brief Generate a random normalized 3 component vector. It's a spherical uniform distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> vecRand4(); //!< \brief Generate a random normalized 4 component vector. It's a spherical uniform distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> vecRand4(T MinRadius, T MaxRadius); //!< \brief Generate a random normalized 4 component vector. It's a spherical uniform distribution. (From GLM_GTX_random extension)

    template <typename T> T gaussRand1(T mean, T std_deviation); //!< \brief Gererate a random floating number according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> gaussRand2(T mean, T std_deviation); //!< \brief Gererate 2 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> gaussRand3(T mean, T std_deviation); //!< \brief Gererate 3 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> gaussRand4(T mean, T std_deviation); //!< \brief Gererate 4 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> gaussRand2(const detail::tvec2<T>& mean, T std_deviation); //!< \brief Gererate 2 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> gaussRand3(const detail::tvec3<T>& mean, T std_deviation); //!< \brief Gererate 3 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> gaussRand4(const detail::tvec4<T>& mean, T std_deviation); //!< \brief Gererate 4 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> gaussRand2(T  mean, const detail::tvec2<T>& std_deviation); //!< \brief Gererate 2 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> gaussRand3(T  mean, const detail::tvec3<T>& std_deviation); //!< \brief Gererate 3 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> gaussRand4(T  mean, const detail::tvec4<T>& std_deviation); //!< \brief Gererate 4 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec2<T> gaussRand2(const detail::tvec2<T>& mean, const detail::tvec2<T>& std_deviation); //!< \brief Gererate 2 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec3<T> gaussRand3(const detail::tvec3<T>& mean, const detail::tvec3<T>& std_deviation); //!< \brief Gererate 3 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)
    template <typename T> detail::tvec4<T> gaussRand4(const detail::tvec4<T>& mean, const detail::tvec4<T>& std_deviation); //!< \brief Gererate 4 random floating numbers according a Gauss distribution. (From GLM_GTX_random extension)

	/// @}
}//namespace glm

#include "random.inl"

#endif//GLM_GTX_random
