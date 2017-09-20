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
/// @ref gtx_constants
/// @file glm/gtx/constants.hpp
/// @date 2011-09-30 / 2011-09-30
/// @author Christophe Riccio
///
/// @see core (dependence)
/// @see gtc_half_float (dependence)
///
/// @defgroup gtx_constants GLM_GTX_constants: Provide build-in constants
/// @ingroup gtx
/// 
/// @brief Allow to perform bit operations on integer values
/// 
/// <glm/gtx/constants.hpp> need to be included to use these functionalities.
///////////////////////////////////////////////////////////////////////////////////

#ifndef GLM_GTX_constants
#define GLM_GTX_constants GLM_VERSION

// Dependency:
#include "../glm.hpp"
#include "../gtc/half_float.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#	pragma message("GLM: GLM_GTX_constants extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_constants
	/// @{

	/// Return the epsilon constant for floating point types.
	/// @todo Implement epsilon for half-precision floating point type.
	/// @see gtx_constants
	template <typename T>
	T epsilon();

	/// Return 0.
	/// @see gtx_constants
	template <typename T>
	T zero();

	/// Return 1.
	/// @see gtx_constants
	template <typename T>
	T one();

	/// Return the pi constant.
	/// @see gtx_constants
	template <typename T>
	T pi();

	/// Return square root of pi.
	/// @see gtx_constants
	template <typename T>
	T root_pi();

	/// Return pi / 2.
	/// @see gtx_constants
	template <typename T>
	T half_pi();

	/// Return pi / 4.
	/// @see gtx_constants
	template <typename T>
	T quarter_pi();

	/// Return 1 / pi.
	/// @see gtx_constants
	template <typename T>
	T one_over_pi();

	/// Return 2 / pi.
	/// @see gtx_constants
	template <typename T>
	T two_over_pi();

	/// Return 2 / sqrt(pi).
	/// @see gtx_constants
	template <typename T>
	T two_over_root_pi();

	/// Return 1 / sqrt(2).
	/// @see gtx_constants
	template <typename T>
	T one_over_root_two();

	/// Return sqrt(pi / 2).
	/// @see gtx_constants
	template <typename T>
	T root_half_pi();

	/// Return sqrt(2 * pi).
	/// @see gtx_constants
	template <typename T>
	T root_two_pi();

	/// Return sqrt(ln(4)).
	/// @see gtx_constants
	template <typename T>
	T root_ln_four();

	/// Return e constant.
	/// @see gtx_constants
	template <typename T>
	T e();

	/// Return Euler's constant.
	/// @see gtx_constants
	template <typename T>
	T euler();

	/// Return sqrt(2).
	/// @see gtx_constants
	template <typename T>
	T root_two();

	/// Return sqrt(3).
	/// @see gtx_constants
	template <typename T>
	T root_three();

	/// Return sqrt(5).
	/// @see gtx_constants
	template <typename T>
	T root_five();

	/// Return ln(2).
	/// @see gtx_constants
	template <typename T>
	T ln_two();

	/// Return ln(10).
	/// @see gtx_constants
	template <typename T>
	T ln_ten();

	/// Return ln(ln(2)).
	/// @see gtx_constants
	template <typename T>
	T ln_ln_two();

	/// Return 1 / 3.
	/// @see gtx_constants
	template <typename T>
	T third();

	/// Return 2 / 3.
	/// @see gtx_constants
	template <typename T>
	T two_thirds();

	/// Return the golden ratio constant.
	/// @see gtx_constants
	template <typename T>
	T golden_ratio();

	/// @}
} //namespace glm

#include "constants.inl"

#endif//GLM_GTX_constants
