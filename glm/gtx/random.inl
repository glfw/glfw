//////////////////////////////////////////////////////////////////////////////////
// OpenGL Mathematics Copyright (c) 2005 - 2012 G-Truc Creation (www.g-truc.net)
//////////////////////////////////////////////////////////////////////////////////
// Created : 2006-01-16
// Updated : 2006-01-16
// Licence : This source is under MIT License
// File    : glm/gtx/random.inl
//////////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <cassert>

namespace glm{

template <>
GLM_FUNC_QUALIFIER float signedRand1()
{
	#if(GLM_COMPILER & GLM_COMPILER_VC)// && (GLM_COMPILER < GLM_COMPILER_VC2010)
	#define RAND_SHIFT_NUM 5
	#else
	#define RAND_SHIFT_NUM 0
	#endif
	return float((std::rand() - (RAND_MAX >> 1) - 1) << 1) / float(RAND_MAX - RAND_SHIFT_NUM);
}

template <>
GLM_FUNC_QUALIFIER double signedRand1()
{
	return double(signedRand1<float>());
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> signedRand2()
{
	return detail::tvec2<T>(
		signedRand1<float>(),
		signedRand1<float>());
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> signedRand3()
{
	return detail::tvec3<T>(
		signedRand1<float>(),
		signedRand1<float>(),
		signedRand1<float>());
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> signedRand4()
{
	return detail::tvec4<T>(
		signedRand1<float>(),
		signedRand1<float>(),
		signedRand1<float>(),
		signedRand1<float>());
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> normalizedRand2(
	T Min, 
	T Max)
{
    return normalizedRand2<T>() * compRand1(Min, Max);
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> normalizedRand2()
{
    T a = compRand1<T>(T(0), T(6.283185307179586476925286766559f));
    return detail::tvec2<T>(cos(a), sin(a));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> normalizedRand3(
	T Min, 
	T Max)
{
    return normalizedRand3<T>() * compRand1(Min, Max);
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> normalizedRand3()
{
	T z = compRand1(T(-1), T(1));
	T a = compRand1(T(0), T(6.283185307179586476925286766559f));

    T r = sqrt(T(1) - z * z);

	T x = r * cos(a);
	T y = r * sin(a);

    return detail::tvec3<T>(x, y, z);
}

template <>
GLM_FUNC_QUALIFIER float compRand1()
{
	return float(std::rand()) / float(RAND_MAX);
}

template <>
GLM_FUNC_QUALIFIER double compRand1()
{
	return double(std::rand()) / double(RAND_MAX);
}

GLM_FUNC_QUALIFIER detail::half compRand1(
	detail::half Min, 
	detail::half Max)
{
	return compRand1<detail::half>() * (Max - Min) + Min;
}

GLM_FUNC_QUALIFIER float compRand1(
	float Min, 
	float Max)
{
	return compRand1<float>() * (Max - Min) + Min;
}

GLM_FUNC_QUALIFIER double compRand1(
	double Min, 
	double Max)
{
	return compRand1<double>() * (Max - Min) + Min;
}

template <typename T>
GLM_FUNC_QUALIFIER T compRand1(
	T Min, 
	T Max)
{
	return T(compRand1<double>() * double(Max - Min) + double(Min));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> compRand2(
	T Min, 
	T Max)
{
	return detail::tvec2<T>(
		compRand1(Min, Max),
		compRand1(Min, Max));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> compRand3(
	T Min, 
	T Max)
{
	return detail::tvec3<T>(
		compRand1(Min, Max),
		compRand1(Min, Max),
		compRand1(Min, Max));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> compRand4(
	T Min, 
	T Max)
{
	return detail::tvec4<T>(
		compRand1(Min, Max),
		compRand1(Min, Max),
		compRand1(Min, Max),
		compRand1(Min, Max));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> compRand2(
	T Min, 
	const detail::tvec2<T>& Max)
{
	return detail::tvec2<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> compRand3(
	T Min, 
	const detail::tvec3<T>& Max)
{
	return detail::tvec3<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y),
		compRand1(Min.z, Max.z));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> compRand4(
	T Min, 
	const detail::tvec4<T>& Max)
{
	return detail::tvec4<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y),
		compRand1(Min.z, Max.z),
		compRand1(Min.w, Max.w));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> compRand2(
	const detail::tvec2<T>& Min, 
	T Max)
{
	return detail::tvec2<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> compRand3(
	const detail::tvec3<T>& Min, 
	T Max)
{
	return detail::tvec3<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y),
		compRand1(Min.z, Max.z));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> compRand4(
	const detail::tvec4<T>& Min, 
	T Max)
{
	return detail::tvec4<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y),
		compRand1(Min.z, Max.z),
		compRand1(Min.w, Max.w));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> compRand2(
	const detail::tvec2<T>& Min, 
	const detail::tvec2<T>& Max)
{
	return detail::tvec2<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> compRand3(
	const detail::tvec3<T>& Min, 
	const detail::tvec3<T>& Max)
{
	return detail::tvec3<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y),
		compRand1(Min.z, Max.z));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> compRand4(
	const detail::tvec4<T>& Min, 
	const detail::tvec4<T>& Max)
{
	return detail::tvec4<T>(
		compRand1(Min.x, Max.x),
		compRand1(Min.y, Max.y),
		compRand1(Min.z, Max.z),
		compRand1(Min.w, Max.w));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<float> vecRand2()
{
	detail::tvec2<float> result(float(0));
	do
	{
		result = compRand2(float(-1), float(1));
	} while (length(result) > float(1));

    return result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<double> vecRand2()
{
	detail::tvec2<double> result(double(0));
	do
	{
		result = compRand2(double(-1), double(1));
	} while (length(result) > double(1));

    return result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> vecRand2(
	T MinRadius, 
	T MaxRadius)
{
	assert(MinRadius <= MaxRadius);

	detail::tvec2<T> Result(T(0));
	T LenRadius(0);

	do
	{
		Result = compRand2(-MaxRadius, MaxRadius);
		LenRadius = length(Result);
	}
	while(LenRadius > MaxRadius || LenRadius < MinRadius);

    return Result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> vecRand3()
{
	detail::tvec3<T> Result(T(0));
	do
	{
		Result = compRand3(T(-1), T(1));
	}
	while(length(Result) > T(1));

    return Result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> vecRand3(
	T MinRadius, 
	T MaxRadius)
{
	assert(MinRadius <= MaxRadius);

	detail::tvec3<T> Result(0);
	T LenRadius(0);

	do
	{
		Result = compRand3(-MaxRadius, MaxRadius);
		LenRadius = length(Result);
	}
	while(LenRadius > MaxRadius || LenRadius < MinRadius);

    return Result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<float> vecRand4()
{
	detail::tvec4<float> result(float(0));
	do
	{
		result = compRand4(float(-1), float(1));
	} while (length(result) > float(1));

    return result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<double> vecRand4()
{
	detail::tvec4<double> result(double(0));
	do
	{
		result = compRand4(double(-1), double(1));
	} while (length(result) > double(1));

    return result;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> vecRand4(
	T MinRadius, 
	T MaxRadius)
{
	assert(MinRadius <= MaxRadius);

	detail::tvec4<T> Result(T(0));
	T LenRadius(T(0));

	do
	{
		Result = compRand4(-MaxRadius, MaxRadius);
		LenRadius = length(Result);
	}
	while(LenRadius > MaxRadius || LenRadius < MinRadius);

    return Result;
}

template <typename T>
GLM_FUNC_QUALIFIER T gaussRand1(
	T mean, 
	T std_deviation)
{
    T w, x1, x2;

    do
    {
        x1 = compRand1(T(-1), T(1));
        x2 = compRand1(T(-1), T(1));

        w = x1 * x1 + x2 * x2;
    } while(w > T(1));

    return x2 * std_deviation * std_deviation * sqrt((T(-2) * log(w)) / w) + mean;
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> gaussRand2(
	T mean, 
	T std_deviation)
{
    return detail::tvec2<T>(
        gaussRand1(mean, std_deviation),
        gaussRand1(mean, std_deviation));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> gaussRand3(
	T mean, 
	T std_deviation)
{
    return detail::tvec3<T>(
        gaussRand1(mean, std_deviation),
        gaussRand1(mean, std_deviation),
        gaussRand1(mean, std_deviation));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> gaussRand4(
	T mean, 
	T std_deviation)
{
    return detail::tvec4<T>(
        gaussRand1(mean, std_deviation),
        gaussRand1(mean, std_deviation),
        gaussRand1(mean, std_deviation),
        gaussRand1(mean, std_deviation));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> gaussRand2(
	T mean, 
	const detail::tvec2<T>& std_deviation)
{
    return detail::tvec2<T>(
        gaussRand1(mean, std_deviation.x),
        gaussRand1(mean, std_deviation.y));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> gaussRand3(
	T mean, 
	const detail::tvec3<T>& std_deviation)
{
    return detail::tvec3<T>(
        gaussRand1(mean, std_deviation.x),
        gaussRand1(mean, std_deviation.y),
        gaussRand1(mean, std_deviation.z));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> gaussRand4(
	T mean, 
	const detail::tvec4<T>& std_deviation)
{
    return detail::tvec4<T>(
        gaussRand1(mean, std_deviation.x),
        gaussRand1(mean, std_deviation.y),
        gaussRand1(mean, std_deviation.z),
        gaussRand1(mean, std_deviation.w));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> gaussRand2(
	const detail::tvec2<T>& mean, 
	T std_deviation)
{
    return detail::tvec2<T>(
        gaussRand1(mean.x, std_deviation),
        gaussRand1(mean.y, std_deviation));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> gaussRand3(
	const detail::tvec3<T>& mean, 
	T std_deviation)
{
    return detail::tvec3<T>(
        gaussRand1(mean.x, std_deviation),
        gaussRand1(mean.y, std_deviation),
        gaussRand1(mean.z, std_deviation));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> gaussRand4(
	const detail::tvec4<T>& mean, 
	T std_deviation)
{
    return detail::tvec4<T>(
        gaussRand1(mean.x, std_deviation),
        gaussRand1(mean.y, std_deviation),
        gaussRand1(mean.z, std_deviation),
        gaussRand1(mean.w, std_deviation));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec2<T> gaussRand2(
	const detail::tvec2<T>& mean, 
	const detail::tvec2<T>& std_deviation)
{
    return detail::tvec2<T>(
        gaussRand1(mean.x, std_deviation.x),
        gaussRand1(mean.y, std_deviation.y));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec3<T> gaussRand3(
	const detail::tvec3<T>& mean, 
	const detail::tvec3<T>& std_deviation)
{
    return detail::tvec3<T>(
        gaussRand1(mean.x, std_deviation.x),
        gaussRand1(mean.y, std_deviation.y),
        gaussRand1(mean.z, std_deviation.z));
}

template <typename T>
GLM_FUNC_QUALIFIER detail::tvec4<T> gaussRand4(
	const detail::tvec4<T>& mean, 
	const detail::tvec4<T>& std_deviation)
{
    return detail::tvec4<T>(
        gaussRand1(mean.x, std_deviation.x),
        gaussRand1(mean.y, std_deviation.y),
        gaussRand1(mean.z, std_deviation.z),
        gaussRand1(mean.w, std_deviation.w));
}

}//namespace glm
