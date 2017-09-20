///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2007-03-06
// Updated : 2009-05-01
// Licence : This source is under MIT License
// File    : glm/gtx/polar_coordinates.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace glm
{
	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> polar
	(
		detail::tvec3<T> const & euclidean
	)
	{
		T length = length(euclidean);
		detail::tvec3<T> tmp = euclidean / length;
		T xz_dist = sqrt(tmp.x * tmp.x + tmp.z * tmp.z);

		return detail::tvec3<T>(
			degrees(atan(xz_dist, tmp.y)),	// latitude
			degrees(atan(tmp.x, tmp.z)),		// longitude
			xz_dist);									// xz distance
	}

	template <typename T> 
	GLM_FUNC_QUALIFIER detail::tvec3<T> euclidean
	(
		detail::tvec3<T> const & polar
	)
	{
		T latitude = radians(polar.x);
		T longitude = radians(polar.y);
		return detail::tvec3<T>(
			cos(latitude) * sin(longitude),
			sin(latitude),
			cos(latitude) * cos(longitude));
	}

}//namespace glm
