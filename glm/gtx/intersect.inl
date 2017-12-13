///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2007-04-03
// Updated : 2009-01-20
// Licence : This source is under MIT licence
// File    : glm/gtx/intersect.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cfloat>
#include <limits>

namespace glm
{
	template <typename genType>
	GLM_FUNC_QUALIFIER bool intersectRayTriangle
	(
		genType const & orig, genType const & dir,
		genType const & v0, genType const & v1, genType const & v2,
		genType & baryPosition
	)
	{
		genType e1 = v1 - v0;
		genType e2 = v2 - v0;

		genType p = glm::cross(dir, e2);

		typename genType::value_type a = glm::dot(e1, p);

		typename genType::value_type Epsilon = std::numeric_limits<typename genType::value_type>::epsilon();
		if(a < Epsilon)
			return false;

		typename genType::value_type f = typename genType::value_type(1.0f) / a;

		genType s = orig - v0;
		baryPosition.x = f * glm::dot(s, p);
		if(baryPosition.x < typename genType::value_type(0.0f))
			return false;
		if(baryPosition.x > typename genType::value_type(1.0f))
			return false;

		genType q = glm::cross(s, e1);
		baryPosition.y = f * glm::dot(dir, q);
		if(baryPosition.y < typename genType::value_type(0.0f))
			return false;
		if(baryPosition.y + baryPosition.x > typename genType::value_type(1.0f))
			return false;

		baryPosition.z = f * glm::dot(e2, q);

		return baryPosition.z >= typename genType::value_type(0.0f);
	}

	//template <typename genType>
	//GLM_FUNC_QUALIFIER bool intersectRayTriangle
	//(
	//	genType const & orig, genType const & dir,
	//	genType const & vert0, genType const & vert1, genType const & vert2,
	//	genType & position
	//)
	//{
	//	typename genType::value_type Epsilon = std::numeric_limits<typename genType::value_type>::epsilon();
	//
	//	genType edge1 = vert1 - vert0;
	//	genType edge2 = vert2 - vert0;
	//
	//	genType pvec = cross(dir, edge2);
	//
	//	float det = dot(edge1, pvec);
	//	if(det < Epsilon)
	//		return false;
	//
	//	genType tvec = orig - vert0;
	//
	//	position.y = dot(tvec, pvec);
	//	if (position.y < typename genType::value_type(0) || position.y > det)
	//		return typename genType::value_type(0);
	//
	//	genType qvec = cross(tvec, edge1);
	//
	//	position.z = dot(dir, qvec);
	//	if (position.z < typename genType::value_type(0) || position.y + position.z > det)
	//		return typename genType::value_type(0);
	//
	//	position.x = dot(edge2, qvec);
	//	position *= typename genType::value_type(1) / det;
	//
	//	return typename genType::value_type(1);
	//}

	template <typename genType>
	GLM_FUNC_QUALIFIER bool intersectLineTriangle
	(
		genType const & orig, genType const & dir,
		genType const & vert0, genType const & vert1, genType const & vert2,
		genType & position
	)
	{
		typename genType::value_type Epsilon = std::numeric_limits<typename genType::value_type>::epsilon();

		genType edge1 = vert1 - vert0;
		genType edge2 = vert2 - vert0;

		genType pvec = cross(dir, edge2);

		float det = dot(edge1, pvec);

		if (det > -Epsilon && det < Epsilon)
			return false;
		float inv_det = typename genType::value_type(1) / det;

		genType tvec = orig - vert0;

		position.y = dot(tvec, pvec) * inv_det;
		if (position.y < typename genType::value_type(0) || position.y > typename genType::value_type(1))
			return false;

		genType qvec = cross(tvec, edge1);

		position.z = dot(dir, qvec) * inv_det;
		if (position.z < typename genType::value_type(0) || position.y + position.z > typename genType::value_type(1))
			return false;

		position.x = dot(edge2, qvec) * inv_det;

		return true;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER bool intersectRaySphere
	(
		genType const & rayStarting, genType const & rayDirection,
		genType const & sphereCenter, typename genType::value_type sphereRadius,
		genType & position, genType & normal
	)
	{
		typename genType::value_type Epsilon = std::numeric_limits<typename genType::value_type>::epsilon();

		typename genType::value_type a = dot(rayDirection, rayDirection);
		typename genType::value_type b = typename genType::value_type(2) * dot(rayStarting, rayDirection);
		typename genType::value_type c = dot(rayStarting, rayStarting) - sphereRadius * sphereRadius;
		typename genType::value_type d = b * b - typename genType::value_type(4) * a * c;
		typename genType::value_type e = sqrt(d);
		typename genType::value_type x1 = (-b - e) / (typename genType::value_type(2) * a);
		typename genType::value_type x2 = (-b + e) / (typename genType::value_type(2) * a);

		if(x1 > Epsilon)
		{
			position = rayStarting + rayDirection * sphereRadius;
			normal = (position - sphereCenter) / sphereRadius;
			return true;
		}
		else if(x2 > Epsilon)
		{
			position = rayStarting + rayDirection * sphereRadius;
			normal = (position - sphereCenter) / sphereRadius;
			return true;
		}
		return false;
	}

	template <typename genType>
	GLM_FUNC_QUALIFIER bool intersectLineSphere
	(
		genType const & point0, genType const & point1,
		genType const & center, typename genType::value_type radius,
		genType & position, genType & normal
	)
	{
		typename genType::value_type Epsilon = std::numeric_limits<typename genType::value_type>::epsilon();

		genType dir = point1 - point0;
		typename genType::value_type a = dot(dir, dir);
		typename genType::value_type b = typename genType::value_type(2) * dot(center, dir);
		typename genType::value_type c = dot(center, center) - radius * radius;
		typename genType::value_type d = b * b - typename genType::value_type(4) * a * c;
		typename genType::value_type e = sqrt(d);
		typename genType::value_type x1 = (-b - e) / (typename genType::value_type(2) * a);
		typename genType::value_type x2 = (-b + e) / (typename genType::value_type(2) * a);

		if(x1 > Epsilon)
		{
			position = center + dir * radius;
			normal = (position - center) / radius;
			return true;
		}
		else if(x2 > Epsilon)
		{
			position = center + dir * radius;
			normal = (position - center) / radius;
			return true;
		}
		return false;
	}
}//namespace glm
