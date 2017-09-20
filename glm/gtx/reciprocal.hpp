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
/// @ref gtx_reciprocal
/// @file glm/gtx/reciprocal.hpp
/// @date 2008-10-09 / 2011-06-07
/// @author Christophe Riccio
///
/// @see core (dependence)
///
/// @defgroup gtx_reciprocal GLM_GTX_reciprocal: Reciprocal
/// @ingroup gtx
/// 
/// @brief Define secant, cosecant and cotangent functions.
/// 
/// <glm/gtx/reciprocal.hpp> need to be included to use these functionalities.
///////////////////////////////////////////////////////////////////////////////////

#ifndef GLM_GTX_reciprocal
#define GLM_GTX_reciprocal GLM_VERSION

// Dependency:
#include "../glm.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#	pragma message("GLM: GLM_GTX_reciprocal extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_reciprocal
	/// @{

	//! Secant function. 
	//! hypotenuse / adjacent or 1 / cos(x)
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType sec(genType const & angle);

	//! Cosecant function. 
	//! hypotenuse / opposite or 1 / sin(x)
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType csc(genType const & angle);
		
	//! Cotangent function. 
	//! adjacent / opposite or 1 / tan(x)
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType cot(genType const & angle);

	//! Inverse secant function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType asec(genType const & x);

	//! Inverse cosecant function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType acsc(genType const & x);
		
	//! Inverse cotangent function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType acot(genType const & x);

	//! Secant hyperbolic function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType sech(genType const & angle);

	//! Cosecant hyperbolic function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType csch(genType const & angle);
		
	//! Cotangent hyperbolic function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType coth(genType const & angle);

	//! Inverse secant hyperbolic function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType asech(genType const & x);

	//! Inverse cosecant hyperbolic function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType acsch(genType const & x);
		
	//! Inverse cotangent hyperbolic function. 
	//! From GLM_GTX_reciprocal extension.
	template <typename genType> 
	genType acoth(genType const & x);

	/// @}
}//namespace glm

#include "reciprocal.inl"

#endif//GLM_GTX_reciprocal
