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
/// @ref gtx_epsilon
/// @file glm/gtx/epsilon.hpp
/// @date 2007-05-21 / 2011-06-07
/// @author Christophe Riccio
/// 
/// @see core (dependence)
/// @see gtc_half_float (dependence)
/// @see gtc_quaternion (dependence)
///
/// @defgroup gtx_epsilon GLM_GTX_epsilon: Epsilon comparison
/// @ingroup gtx
/// 
/// @brief Comparison functions for a user defined epsilon values.
/// 
/// <glm/gtx/epsilon.hpp> need to be included to use these functionalities.
///////////////////////////////////////////////////////////////////////////////////

#ifndef GLM_GTX_epsilon
#define GLM_GTX_epsilon GLM_VERSION

// Dependency:
#include "../glm.hpp"
#include "../gtc/half_float.hpp"
#include "../gtc/quaternion.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#	pragma message("GLM: GLM_GTX_epsilon extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_epsilon
	/// @{

	/// Returns the component-wise compare of |x - y| < epsilon.
	/// @see gtx_epsilon
	template <typename genTypeT, typename genTypeU> 
	bool equalEpsilon(
		genTypeT const & x, 
		genTypeT const & y, 
		genTypeU const & epsilon);
		
	/// Returns the component-wise compare of |x - y| >= epsilon.
	/// @see gtx_epsilon
	template <typename genTypeT, typename genTypeU>
	bool notEqualEpsilon(
		genTypeT const & x, 
		genTypeT const & y, 
		genTypeU const & epsilon);

	/// @}
}//namespace glm

#include "epsilon.inl"

#endif//GLM_GTX_epsilon
