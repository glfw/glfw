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
/// @ref gtx_ulp
/// @file glm/gtx/ulp.hpp
/// @date 2011-02-21 / 2011-06-07
/// @author Christophe Riccio
///
/// @see core (dependence)
///
/// @defgroup gtx_ulp GLM_GTX_ulp: Accuracy measurement
/// @ingroup gtx
/// 
/// @brief Allow the measurement of the accuracy of a function against a reference 
/// implementation. This extension works on floating-point data and provide results 
/// in ULP.
/// <glm/gtx/ulp.hpp> need to be included to use these functionalities.
///////////////////////////////////////////////////////////////////////////////////

#ifndef GLM_GTX_ulp
#define GLM_GTX_ulp GLM_VERSION

// Dependency:
#include "../glm.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#	pragma message("GLM: GLM_GTX_ulp extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_ulp
	/// @{

	//! Return the next ULP value(s) after the input value(s).
	//! From GLM_GTX_ulp extension.
    template <typename genType>
    genType next_float(genType const & x);
        
	//! Return the previous ULP value(s) before the input value(s).
	//! From GLM_GTX_ulp extension.
    template <typename genType>
    genType prev_float(genType const & x);

	//! Return the value(s) ULP distance after the input value(s).
	//! From GLM_GTX_ulp extension.
    template <typename genType>
    genType next_float(genType const & x, uint const & Distance);
        
    //! Return the value(s) ULP distance before the input value(s).
	//! From GLM_GTX_ulp extension.
    template <typename genType>
    genType prev_float(genType const & x, uint const & Distance);
        
    //! Return the distance in the number of ULP between 2 scalars.
	//! From GLM_GTX_ulp extension.
    template <typename T>
    uint float_distance(T const & x, T const & y);
        
    //! Return the distance in the number of ULP between 2 vectors.
	//! From GLM_GTX_ulp extension.
    template<typename T, template<typename> class vecType>
    vecType<uint> float_distance(vecType<T> const & x, vecType<T> const & y);
        
	/// @}
}// namespace glm

#include "ulp.inl"

#endif//GLM_GTX_ulp

