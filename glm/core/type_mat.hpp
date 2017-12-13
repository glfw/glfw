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
/// @ref core
/// @file glm/core/type_mat.hpp
/// @date 2010-01-26 / 2011-06-15
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#ifndef glm_core_type_mat
#define glm_core_type_mat

#include "type_gentype.hpp"

namespace glm{
namespace detail
{
	//template 
	//<
	//	typename T, 
	//	template <typename> class C, 
	//	template <typename> class R
	//>
	//struct matType
	//{
	//	enum ctor{null};
	//	typedef T value_type;
	//	typedef std::size_t size_type;
	//	typedef C<T> col_type;
	//	typedef R<T> row_type;
	//	static size_type const col_size;
	//	static size_type const row_size;
	//};

	//template 
	//<
	//	typename T, 
	//	template <typename> class C, 
	//	template <typename> class R
	//>
	//typename matType<T, C, R>::size_type const 
	//matType<T, C, R>::col_size = matType<T, C, R>::col_type::value_size;

	//template 
	//<
	//	typename T, 
	//	template <typename> class C, 
	//	template <typename> class R
	//>
	//typename matType<T, C, R>::size_type const 
	//matType<T, C, R>::row_size = matType<T, C, R>::row_type::value_size;

}//namespace detail
}//namespace glm

#endif//glm_core_type_mat
