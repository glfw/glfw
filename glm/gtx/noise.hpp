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
/// @ref gtx_noise
/// @file glm/gtx/noise.hpp
/// @date 2011-04-21 / 2011-06-07
/// @author Christophe Riccio
///
/// @see core (dependence)
///
/// @defgroup gtx_noise GLM_GTX_noise: Procedural noise functions
/// @ingroup gtx
/// 
/// Defines 2D, 3D and 4D procedural noise functions 
/// Based on the work of Stefan Gustavson and Ashima Arts on "webgl-noise": 
/// https://github.com/ashima/webgl-noise 
/// Following Stefan Gustavson's paper "Simplex noise demystified": 
/// http://www.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
/// Defines the half-precision floating-point type, along with various typedefs for vectors and matrices.
/// <glm/gtx/noise.hpp> need to be included to use these functionalities.
///////////////////////////////////////////////////////////////////////////////////

#ifndef GLM_GTX_noise
#define GLM_GTX_noise GLM_VERSION

// Dependency:
#include "../glm.hpp"
#include "../gtc/noise.hpp"

#if(defined(GLM_MESSAGES) && !defined(glm_ext))
#	pragma message("GLM: GLM_GTX_noise extension included")
#endif

namespace glm
{
	/// @addtogroup gtx_noise
	/// @{

	/// @}
}//namespace glm

#include "noise.inl"

#endif//glm_gtx_noise
