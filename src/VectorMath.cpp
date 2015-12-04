#include "VectorMath.h"
#include "StructDesc.h"

//template<> StructDescEntry DescEntry< vec<2, f32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2		, offset_, TYPE_F32); }
//template<> StructDescEntry DescEntry< vec<2, f64> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2		, offset_, TYPE_F64); }
//template<> StructDescEntry DescEntry< vec<2, i32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2		, offset_, TYPE_I32); }
//template<> StructDescEntry DescEntry< vec<2, u32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2		, offset_, TYPE_U32); }
//template<> StructDescEntry DescEntry< vec<2, i8 > >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2		, offset_, TYPE_I8 ); }
//template<> StructDescEntry DescEntry< vec<2, u8 > >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2		, offset_, TYPE_U8 ); }
//template<> StructDescEntry DescEntry< mat<2, f32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2 * 2	, offset_, TYPE_F32); }
//template<> StructDescEntry DescEntry< mat<2, f64> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 2 * 2	, offset_, TYPE_F64); }
//
//template<> StructDescEntry DescEntry< vec<3, f32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3		, offset_, TYPE_F32); }
//template<> StructDescEntry DescEntry< vec<3, f64> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3		, offset_, TYPE_F64); }
//template<> StructDescEntry DescEntry< vec<3, i32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3		, offset_, TYPE_I32); }
//template<> StructDescEntry DescEntry< vec<3, u32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3		, offset_, TYPE_U32); }
//template<> StructDescEntry DescEntry< vec<3, i8 > >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3		, offset_, TYPE_I8 ); }
//template<> StructDescEntry DescEntry< vec<3, u8 > >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3		, offset_, TYPE_U8 ); }
//template<> StructDescEntry DescEntry< mat<3, f32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3 * 3	, offset_, TYPE_F32); }
//template<> StructDescEntry DescEntry< mat<3, f64> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 3 * 3	, offset_, TYPE_F64); }
//
//template<> StructDescEntry DescEntry< vec<4, f32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4		, offset_, TYPE_F32); }
//template<> StructDescEntry DescEntry< vec<4, f64> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4		, offset_, TYPE_F64); }
//template<> StructDescEntry DescEntry< vec<4, i32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4		, offset_, TYPE_I32); }
//template<> StructDescEntry DescEntry< vec<4, u32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4		, offset_, TYPE_U32); }
//template<> StructDescEntry DescEntry< vec<4, i8 > >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4		, offset_, TYPE_I8 ); }
//template<> StructDescEntry DescEntry< vec<4, u8 > >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4		, offset_, TYPE_U8 ); }
//template<> StructDescEntry DescEntry< mat<4, f32> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4 * 4	, offset_, TYPE_F32); }
//template<> StructDescEntry DescEntry< mat<4, f64> >(std::string name_, unsigned int components_, unsigned int offset_)	{ return StructDescEntry(name_, components_ * 4 * 4	, offset_, TYPE_F64); }

#ifdef VMATH

#include <cmath>

#define LOOPN		for (i32 idx = 0; idx < N; idx++)
#define LOOPN2		for (i32 idx = 0; idx < N * N; idx++)
#define LOOPNN		for (i32 irow = 0; irow < N; irow++) \
					for (i32 icol = 0; icol < N; icol++)

/* Functions */
/* Constructors */
template<int N, class T>
vec<N, T>::vec()
{

}

template<int N, class T>
vec<N, T>::vec(T v)
{
	LOOPN V[idx] = v;
}

template<int N, class T>
vec<N, T>::vec(const vec<N, T> &v)
{
	LOOPN V[idx] = v.V[idx];
}

/* Assignment */
template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::operator=(const vec<N, T> &v)
{
	LOOPN V[idx] = v.V[idx];
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::operator=(T v)
{
	LOOPN V[idx] = v;
	return *this;
}


/* Comparison */
// TODO: Optimize
template<int N, class T>
__forceinline bool vec<N, T>::operator==(const vec<N, T> &v) const
{
	bool equals = false;
	LOOPN equals &= (V[idx] == v.V[idx]);
	return equals;
}

// TODO: Optimize
template<int N, class T>
__forceinline bool vec<N, T>::operator!=(const vec<N, T> &v) const
{
	LOOPN if (V[idx] != v.V[idx]) return true;
	return false;
}

/* Addition */
template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator+(const vec<N, T> &v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] + v.V[idx];
	return out;
}

template<int N, class T> 
__forceinline vec<N, T> &vec<N, T>::operator+=(const vec<N, T> &v)
{
	LOOPN V[idx] += v.V[idx];
	return *this;
}

template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator+(T v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] + v;
	return out;
}

template<int N, class T> 
__forceinline vec<N, T> &vec<N, T>::operator+=(T v)
{
	LOOPN V[idx] += v;
	return *this;
}

template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator+() const
{
	return vec<N, T>(*this);
}


/* Subtraction */
template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator-(const vec<N, T> &v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] - v.V[idx];
	return out;
}

template<int N, class T> 
__forceinline vec<N, T> &vec<N, T>::operator-=(const vec<N, T> &v)
{
	LOOPN V[idx] -= v.V[idx];
	return *this;
}

template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator-(T v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] - v;
	return out;
}

template<int N, class T> 
__forceinline vec<N, T> &vec<N, T>::operator-=(T v)
{
	LOOPN V[idx] -= v;
	return *this;
}

template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator-() const
{
	vec<N, T> out;
	LOOPN out.V[idx] = -V[idx];
	return out;
}

/* Element-wise multiplication */
template<int N, class T> 
vec<N, T> vec<N, T>::elemMul(const vec<N, T> &v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] * v;
	return out;
}

template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator&(const vec<N, T> &v) const
{
	return elemMul(*this, v);
}

template<int N, class T> 
__forceinline vec<N, T> &vec<N, T>::operator&=(const vec<N, T> &v)
{
	LOOPN V[idx] *= v.V[idx];
	return *this;
}

/* Multiplication/Dot product */
template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator*(const mat<N, T> &v) const
{
	// TODO
	return vec<N, T>();
	//return vec<N, T>(
	//	v.a * x + v.c * y,
	//	v.b * x + v.d * y
	//);
}

template<int N, class T> 
__forceinline vec<N, T> &vec<N, T>::operator*=(const mat<N, T> &v)
{
	// TODO
	//vec<N, T> nv = vec<N, T>(v * (*this));
	//x = nv.x;
	//y = nv.y;
	return *this;
}

template<int N, class T> 
__forceinline vec<N, T> vec<N, T>::operator*(T v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] * v;
	return out;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::operator*=(T v)
{
	LOOPN V[idx] *= v;
	return *this;
}

template<int N, class T>
__forceinline T vec<N, T>::dot(const vec<N, T> &v) const
{
	T out = vec[0] * v.V[0];
	for (i32 idx = 1; idx < N; idx++) out += V[idx] * v.V[idx];
	return out;
}

template<int N, class T>
__forceinline T vec<N, T>::operator*(const vec<N, T> &v) const
{
	return dot(v);
}

/* Division */
template<int N, class T>
__forceinline vec<N, T> vec<N, T>::operator/(const vec<N, T> &v) const
{
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] / v.V[idx];
	return out;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::operator/=(const vec<N, T> &v)
{
	LOOPN V[idx] /= v.V[idx];
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::operator/(T v) const
{
	T div = T(1) / v;
	vec<N, T> out;
	LOOPN out.V[idx] = V[idx] * div;
	return out;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::operator/=(T v)
{
	T div = T(1) / v;
	vec<N, T> out;
	LOOPN V[idx] *= div;
	return *this;
}


/* Misc useful stuff */
template<class T>
__forceinline T vec<2, T>::minElement() const
{
	return smin2<T>(x, y);
}

template<class T>
__forceinline T vec<2, T>::maxElement() const
{
	return smax2<T>(x, y);
}

template<class T>
__forceinline T vec<3, T>::minElement() const
{
	return smin3<T>(x, y, z);
}

template<class T>
__forceinline T vec<3, T>::maxElement() const
{
	return smax3<T>(x, y, z);
}

template<class T>
__forceinline T vec<4, T>::minElement() const
{
	return smin4<T>(x, y, z, w);
}

template<class T>
__forceinline T vec<4, T>::maxElement() const
{
	return smax4<T>(x, y, z, w);
}


template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::clamp(T mmin, T mmax)
{
	LOOPN V[idx] = sclamp(V[idx], mmin, mmax);
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::getClamped(T mmin, T mmax) const
{
	vec<N, T> out;
	LOOPN = sclamp(V[idx], mmin, mmax);
	return out;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::clamp(const vec<N, T> &min, const vec<N, T> &max)
{
	LOOPN V[idx] = sclamp(V[idx], mmin.V[idx], mmax.V[idx]);
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::getClamped(const vec<N, T> &min, const vec<N, T> &max) const
{
	vec<N, T> out;
	LOOPN = sclamp(V[idx], mmin.V[idx], mmax.V[idx]);
	return out;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::saturate()
{
	LOOPN V[idx] = ssaturate(V[idx]);
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::getSaturated() const
{
	vec<N, T> out;
	LOOPN ssaturate(V[idx]);
	return out;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::normalize()
{
	T t = T(1) / length();
	operator*=(t);
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::getNormalized() const
{
	T t = T(1) / length();
	vec<N, T> out;
	return out * t;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::floor()
{
	// TODO: Type!
	x = floorf(x);
	y = floorf(y);
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> &vec<N, T>::ceil()
{
	// TODO: Type!
	x = ceilf(x);
	y = ceilf(y);
	return *this;
}

template<int N, class T>
__forceinline T vec<N, T>::length() const
{
	// TODO: Specialize
	return sqrtf(dot(*this));
}

template<int N, class T>
__forceinline T vec<N, T>::lengthSquared() const
{
	return dot(*this);
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::getReflection(const vec<N, T> &normal) const
{
	return normal * (dot(normal) * T(2)) - *this;
}

template<int N, class T>
__forceinline vec<N, T> vec<N, T>::getRefraction(const vec<N, T> &normal, T rho) const
{
	T tr = -dot(normal);
	T tc = T(1) - rho * rho (T(1) - tr * tr);
	if (tc < T(0)) return reflection(normal);
	return *this * rho + normal * (rho * tr - sqrt(tc));
}

template<int N, class T>
T vec<N, T>::getProjectedLengthOn(const vec<N, T> &v) const
{
	return dot(v) / v.len();
}

template<int N, class T>
vec<N, T> vec<N, T>::getProjectedOn(const vec<N, T> &v) const
{
	return v * (dot(v) / (v.dot(v)));
}

/* Scalar-vector operations */
template<int N, class T>
vec<N, T> operator+(T lv, vec<N, T> &rv)
{
	return rv + lv;
}

template<int N, class T>
__forceinline vec<N, T> operator-(T lv, vec<N, T> &rv)
{
	return - rv + lv;
}

template<int N, class T>
__forceinline vec<N, T> operator*(T lv, vec<N, T> &rv)
{
	return rv * lv;
}

template<int N, class T>
__forceinline vec<N, T> operator/(T lv, vec<N, T> &rv)
{
	vec<N, T> out;
	LOOPN out[idx] = lv / V[idx];
	return out;
}

template<int N, class T>
vec<N, T> operator+(T lv, const vec<N, T> &rv)
{
	return rv + lv;
}

template<int N, class T>
__forceinline vec<N, T> operator-(T lv, const vec<N, T> &rv)
{
	return - rv + lv;
}

template<int N, class T>
__forceinline vec<N, T> operator*(T lv, const vec<N, T> &rv)
{
	return rv * lv;
}

template<int N, class T>
__forceinline vec<N, T> operator/(T lv, const vec<N, T> &rv)
{
	vec<N, T> out;
	LOOPN out[idx] = lv / V[idx];
	return out;
}



/* Cross product */
template<class T>
__forceinline vec<3, T> vec<3, T>::cross(const vec<3, T> &v) const
{
	return vec<3, T>(crossX(v), crossY(v), crossZ(v));
}

template<class T>
__forceinline T vec<3, T>::crossX(const vec<3, T> &v) const
{
	return y * v.z - z * v.y;
}

template<class T>
__forceinline T vec<3, T>::crossY(const vec<3, T> &v) const
{
	return z * v.x - x * v.z;
}

template<class T>
__forceinline T vec<3, T>::crossZ(const vec<3, T> &v) const
{
	return x * v.y - y * v.x;
}

template<class T>
__forceinline vec<3, T> vec<3, T>::operator^(const vec<3, T> &v) const
{
	return cross(v);
}


/* Misc useful stuff */

template<class T>
__forceinline vec<2, T> vec<3, T>::xy() const
{
	return vec<2, T>(x, y);
}

template<class T>
__forceinline vec<4, T> vec<3, T>::xyz1() const
{
	return vec<4, T>(x, y, z, 1);
}

template<class T>
__forceinline vec<4, T> vec<3, T>::xyz0() const
{
	return vec<4, T>(x, y, z, 0);
}



/* Misc useful stuff */

template<class T>
__forceinline void vec<4, T>::divW()
{
	T div = T(1) / w;
	x *= div;
	y *= div;
	z *= div;
	w = T(1);
}

template<class T>
__forceinline vec<4, T> vec<4, T>::getDivW() const
{
	vec<4, T> res = vec<4, T>(x, y, z, w);
	res.divW();
	return res;
}

template<class T>
__forceinline vec<4, T> vec<4, T>::xyz1() const
{
	return vec<4, T>(x, y, z, T(1));
}

template<class T>
__forceinline vec<3, T> vec<4, T>::xyz() const
{
	return vec<3, T>(x, y, z);
}

template<class T>
__forceinline vec<3, T> vec<4, T>::xyw() const
{
	return vec<3, T>(x, y, w);
}

template<class T>
__forceinline vec<2, T> vec<4, T>::xy() const
{
	return vec<2, T>(x, y);
}

template<class T>
__forceinline vec<2, T> vec<4, T>::xz() const
{
	return vec<2, T>(x, z);
}

template<class T>
__forceinline vec<3, T> vec<4, T>::xy0() const
{
	return vec<3, T>(x, y, Zero<T>());
}

template<class T>
__forceinline vec<3, T> vec<4, T>::xy1() const
{
	return vec<3, T>(x, y, One<T>());
}

template<class T>
__forceinline vec<3, T> vec<4, T>::xz0() const
{
	return vec<3, T>(x, z, Zero<T>());
}

template<class T>
__forceinline vec<3, T> vec<4, T>::xz1() const
{
	return vec<3, T>(x, z, One<T>());
}

template<class T>
__forceinline vec<4, T> vec<4, T>::point4() const
{
	return vec<4, T>(x, y, z, w));
}






/* Functions */

/* Assignment */
template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator=(const mat<N, T> &v)
{
	LOOPN2 V[idx] = v.V[idx];
	return *this;
}


/* Comparison */
template<int N, class T>
__forceinline bool mat<N, T>::operator==(const mat<N, T> &v) const
{
	LOOPN2 if (V[idx] != v.V[idx]) return false;
	return true;
}

template<int N, class T>
__forceinline bool mat<N, T>::operator!=(const mat<N, T> &v) const
{
	return !(*this == v);
}

/* Addition */
template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator+(const mat<N, T> &v) const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = V[idx] + v.V[idx];
	return out;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator+=(const mat<N, T> &v)
{
	LOOPN2 V[idx] += v.V[idx];
	return *this;
}

template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator+(T v) const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = V[idx] + v;
	return out;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator+=(T v)
{
	LOOPN2 V[idx] += v;
	return *this;
}

template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator+() const
{
	return *this;
}


/* Subtraction */
template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator-(const mat<N, T> &v) const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = V[idx] - v.V[idx];
	return out;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator-=(const mat<N, T> &v)
{
	LOOPN2 V[idx] -= v.V[idx];
	return *this;
}

template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator-(T v) const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = V[idx] + v;
	return out;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator-=(T v)
{
	LOOPN2 V[idx] -= v;
	return *this;
}

template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator-() const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = -V[idx];
	return out;
}


/* Inner product */
template<int N, class T>
__forceinline T mat<N, T>::innerProd(const mat<N, T> &v, const u32 row, const u32 col) const
{
	T out = M[row][0] * v.M[0][col];
	for (i32 idx = 1; idx < N; idx++) out += M[row][idx] * v.M[idx][col];
	return out;
}


/* Multiplication */
template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator*(const mat<N, T>	&v) const
{
	mat<N, T> out;
	LOOPNN out.M[irow][icol] = innerProd(v, irow, icol);
	return out;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator*=(const mat<N, T> &v)
{
	mat<N, T> mat = *this * v;
	LOOPN2 V[idx] = mat.V[idx];
	return *this;
}

template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator*(T v) const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = V[idx] + v;
	return out;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator*=(T v)
{
	LOOPN2 V[idx] *= v;
	return *this;
}

template<int N, class T>
__forceinline vec<N, T> mat<N, T>::operator*(const vec<N, T> &v) const
{
	vec<2, T> out;
	LOOPN out.V[idx] = getRow(idx) * v;
	return out;
}


/* Element-wise multiplication */
template<int N, class T>
__forceinline mat<N, T> mat<N, T>::elemMul(const mat<N, T> &v) const
{
	mat<N, T> out;
	LOOPN2 out.V[idx] = V[idx] * v.V[idx];
	return out;
}

/* Division with scalar */
template<int N, class T>
__forceinline mat<N, T> mat<N, T>::operator/(T v) const
{
	T div = T(1) / v;
	return *this * div;
}

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::operator/=(T v)
{
	T div = T(1) / v;
	*this *= div;
	return *this;
}


/* Misc matrix operations */

template<int N, class T>
__forceinline mat<N, T> &mat<N, T>::transpose()
{
	for (i32 irow = 0; irow < N - 1; irow++)
	for (i32 icol = irow + 1; icol < N; icol++) {
		sswap(M[irow][icol], M[icol][irow]);
	}
	return *this;
}

template<int N, class T>
__forceinline T mat<N, T>::trace() const
{
	T out = T(0);
	LOOPN out += M[idx][idx];
	return out;
}

template<int N, class T>
__forceinline vec<N, T> mat<N, T>::getRow(u32 row) const
{
	// TODO: Can reinterpret_cast?
	vec<N, T> out;
	LOOPN out.V[idx] = M[row][idx];
	return out;
}

template<int N, class T>
__forceinline void mat<N, T>::setRow(u32 row, const vec<N, T> &v)
{
	LOOPN M[row][idx] = v.V[idx];
}

template<int N, class T>
__forceinline vec<N, T> mat<N, T>::getCol(u32 col) const
{
	// TODO: Can reinterpret_cast?
	vec<N, T> out;
	LOOPN out.V[idx] = M[idx][col];
	return out;
}

template<int N, class T>
__forceinline void mat<N, T>::setCol(u32 col, const vec<N, T> &vec)
{
	LOOPN M[idx][col] = v.V[idx];
}

/*
template<int N, class T>
__forceinline mat<N, T> operator+(T lv, mat<N, T> &rv)
{
	return rv + lv;
}

template<int N, class T>
__forceinline mat<N, T> operator-(T lv, mat<N, T> &rv)
{
	return - rv + lv;
}

template<int N, class T>
__forceinline mat<N, T> operator*(T lv, mat<N, T> &rv)
{
	return rv * lv;
}
*/

template<int N, class T>
vec<N, T> mat<N, T>::up() const
{
	return getCol(1);
	//return vec3<T>(b, e, h);
}

template<int N, class T>
vec<N, T> mat<N, T>::down() const
{
	return -up();
}

template<int N, class T>
vec<N, T> mat<N, T>::left() const
{
	return -right();
}

template<int N, class T>
vec<N, T> mat<N, T>::right() const
{
	//return vec3<T>(a, d, g);
	return getCol(0);
}









template<class T>
__forceinline bool mat<2, T>::isIdentity() const
{
	return a == 1 && b == 0 && c == 0 && d == 1;
}

template<class T>
__forceinline mat<2, T> &mat<2, T>::identity()
{
	a = 1;
	b = 0;
	c = 0;
	d = 1;
	return *this;
}

template<class T>
__forceinline T mat<2, T>::det() const
{
	return a * d - b * c;
}

template<class T>
__forceinline mat<2, T> &mat<2, T>::inv()
{
	mat<2, T> mat = *this;
	T deter = det();
	if (deter == 0) {
		a = T(1);
		b = T(0);
		c = T(0);
		d = T(1);
		return *this;
	}
	T denom = T(1) / deter;
	a =  mat.d * denom;
	b = -mat.b * denom;
	c = -mat.c * denom;
	d =  mat.a * denom;
	return *this;
}

template<class T>
__forceinline mat<2, T> mat<2, T>::getInverse() const
{
	mat<N, T> mat = *this;
	mat.inv();
	return mat;
}

template<class T>
__forceinline vec<2, T> mat<2, T>::getEigenvalues() const
{
	vec<2, T> ret;
	T t = trace();
	T u = sqrt(t * t - (T(4)) * det());
	ret.x = t - u;
	ret.y = t + u;
	return ret;
}

template<class T>
__forceinline mat<2, T> mat<2, T>::getEigenvectors(const vec<2, T> &eig) const
{
	if (b != 0)
		return mat<2, T>(b, b, eig.x - a, eig.y - a);
	if (c != 0)
		return mat<2, T>(eig.x - d, eig.y - d, c, c);
	return mat<2, T>(1, 0, 0, 1);
}

template<class T>
vec<2, T> mat<2, T>::solveSystem(const vec<2, T> &eq) const
{
	T invd = T(1) / (a * d - b * c);
	T sx = (eq.x * d - b * eq.y) * invd;
	T sy = (a * eq.x - eq.y * c) * invd;
	return vec<2, T>(sx, sy);
}





template<class T>
__forceinline mat<3, T> mat<3, T>::getRotationMatrix(T rad, const vec<3, T> &axis)
{
	mat<3, T> mat;
	vec<3, T> v = axis;
	if (abs(1.0f - v.lengthSquared()) > 0.0001f) /* TODO: Hard coded tolerance */
		v.normalize();

	T cs = cos(rad);
	T sn = sin(rad);

	T xx = v.x * v.x;
	T xy = v.x * v.y;
	T xz = v.x * v.z;
	T yy = v.y * v.y;
	T yz = v.y * v.z;
	T zz = v.z * v.z;

	mat.a = xx + (T(1) - xx) * cs;
	mat.b = xy * (T(1) - cs) - v.z * sn;
	mat.c = xz * (T(1) - cs) + v.y * sn;

	mat.d = xy * (T(1) - cs) + v.z * sn;
	mat.e = yy + (T(1) - yy) * cs;
	mat.f = yz * (T(1) - cs) - v.x * sn;

	mat.g = xz * (T(1) - cs) - v.y * sn;
	mat.h = yz * (T(1) - cs) + v.x * sn;
	mat.i = zz + (T(1) - zz) * cs;
}

template<class T>
__forceinline mat<3, T> mat<3, T>::getRotationXMatrix(T rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	mat<3, T> mat;
	mat.a = 1;		mat.b = 0;		mat.c = 0;
	mat.d = 0;		mat.e = cs;		mat.f = -sn;
	mat.g = 0;		mat.h = sn;		mat.i = cs;
	return mat;
}

template<class T>
__forceinline mat<3, T> mat<3, T>::getRotationYMatrix(T rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	mat<3, T> mat;
	mat.a = cs;		mat.b = 0;		mat.c = sn;
	mat.d = 0;		mat.e = 1;		mat.f = 0;
	mat.g = -sn;	mat.h = 0;		mat.i = cs;
	return mat;
}

template<class T>
__forceinline mat<3, T> mat<3, T>::getRotationZMatrix(T rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	mat<3, T> mat;
	mat.a = cs;		mat.b = -sn;	mat.c = 0;
	mat.d = sn;		mat.e = cs;		mat.f = 0;
	mat.g = 0;		mat.h = 0;		mat.i = 1;
	return mat;
}

template<class T>
__forceinline bool mat<3, T>::isIdentity() const
{
	return a == 1 &&
		b == 0 &&
		c == 0 &&

		d == 0 &&
		e == 1 &&
		f == 0 &&

		g == 0 &&
		h == 0 &&
		i == 1;
}



/* Misc matrix operations */
// TODO: Move to base class
template<class T>
__forceinline mat<3, T> &mat<3, T>::identity()
{
	a = e = i = 1;
	b = c = 0;
	d = f = 0;
	g = h = 0;
	return *this;
}

template<class T>
__forceinline T mat<3, T>::det() const
{
	return a * (i * e - h * f) -
		d * (i * b - h * c) +
		g * (f * b - e * c);
}

template<class T>
__forceinline mat<3, T> &mat<3, T>::inv()
{
	T ie = i * e;
	T ib = i * b;
	T fb = f * b;
	T hf = h * f;
	T hc = h * c;
	T ec = e * c;

	T deter = a * (ie - hf) - d * (ib - hc) + g * (fb - ec);

	if (deter == ((T) 0)) {
		identity();
		return *this;
	}

	mat<3, T> mat = *this;
	T denom = T(1) / deter;

	a = (ie            - hf           ) * denom;
	b = (hc            - ib           ) * denom;
	c = (fb            - ec           ) * denom;

	d = (mat.f * mat.g - mat.d * mat.i) * denom;
	e = (mat.a * mat.i - mat.c * mat.g) * denom;
	f = (mat.d * mat.c - mat.a * mat.f) * denom;

	g = (mat.d * mat.h - mat.e * mat.g) * denom;
	h = (mat.b * mat.g - mat.a * mat.h) * denom;
	i = (mat.a * mat.e - mat.b * mat.d) * denom;
	return *this;
}

template<class T>
__forceinline mat<3, T> mat<3, T>::getInverse() const
{
	mat<3, T> mat = *this;
	mat.inv();
	return mat;
}


template<class T>
mat<3, T> &mat<3, T>::postRotateX(T rad)
{
	T nd, ne, nf;
	T cs, sn;

	cs = cos(rad);
	sn = sin(rad);

	nd = d * cs - g * sn;
	ne = e * cs - h * sn;
	nf = f * cs - i * sn;

	g = d * sn + g * cs;
	h = e * sn + h * cs;
	i = f * sn + i * cs;

	d = nd;
	e = ne;
	f = nf;

	return *this;
}

template<class T>
mat<3, T> &mat<3, T>::postRotateY(T rad)
{
	T na, nb, nc;
	T cs, sn;

	cs = cos(rad);
	sn = sin(rad);

	na = a * cs + g * sn;
	nb = b * cs + h * sn;
	nc = c * cs + i * sn;

	g = g * cs - a * sn;
	h = h * cs - b * sn;
	i = i * cs - c * sn;

	a = na;
	b = nb;
	c = nc;

	return *this;
}

template<class T>
mat<3, T> &mat<3, T>::preRotateY(T rad)
{
	T na, nd, ng;
	T cs, sn;

	cs = cos(rad);
	sn = sin(rad);

	na = a * cs - c * sn;
	nd = d * cs - f * sn;
	ng = g * cs - i * sn;

	c = a * sn + c * cs;
	f = d * sn + f * cs;
	i = g * sn + i * cs;

	a = na;
	d = nd;
	g = ng;

	return *this;
}

template<class T>
mat<3, T> &mat<3, T>::postRotateZ(T rad)
{
	T na, nb, nc;
	T cs, sn;

	cs = cos(rad);
	sn = sin(rad);

	na = a * cs - d * sn;
	nb = b * cs - e * sn;
	nc = c * cs - f * sn;

	d = a * sn + d * cs;
	e = b * sn + e * cs;
	f = c * sn + f * cs;

	a = na;
	b = nb;
	c = nc;

	return *this;
}

template<class T>
mat<3, T> &mat<3, T>::postRotate(T rad, const vec<3, T> axis)
{
	mat<3, T> r = getRotationMatrix(rad, axis);
	return *this;
}

template<class T>
vec<3, T> mat<3, T>::front() const
{
	return -back();
	//return vec3<T>(c, f, i);
}

template<class T>
vec<3, T> mat<3, T>::back() const
{
	return vec<3, T>(c, f, i);
	//return -front(); // TODO: As define
}

template<class T>
vec<3, T> mat<3, T>::solveSystem(const vec<3, T> &eq) const
{
	// TODO: Optimize
	// TODO: Numerical precision
	vec<3, T> res;
	T denom = T(1) / det();
	res.x = mat<3, T>(
		eq.x, b, c,
		eq.y, e, f,
		eq.z, h, i).det() * denom;
	res.y = mat<3, T>(
		a, eq.x, c,
		d, eq.y, f,
		g, eq.z, i).det() * denom;
	res.z = mat<3, T>(
		a, b, eq.x,
		d, e, eq.y,
		g, h, eq.z).det() * denom;
	return res;
}












template<class T>
__forceinline bool mat<4, T>::isIdentity() const
{
	return	a == 1 && b == 0 && c == 0 && d == 0 &&
			e == 0 && f == 1 && g == 0 && h == 0 &&
			i == 0 && j == 0 && k == 1 && l == 0 &&
			m == 0 && n == 0 && o == 0 && p == 1;
}


template<class T>
__forceinline T mat<4, T>::affineiprod0(const mat<4, T> &v, const u32 row, const u32 col) const
{
	return M[row][0] * v.M[0][col] +
		M[row][1] * v.M[1][col] +
		M[row][2] * v.M[2][col];
}

template<class T>
__forceinline T mat<4, T>::affineiprod1(const mat<4, T> &v, const u32 row, const u32 col) const
{
	return M[row][0] * v.M[0][col] +
		M[row][1] * v.M[1][col] +
		M[row][2] * v.M[2][col] +
		M[row][3];
}

template<class T>
__forceinline mat<4, T> mat<4, T>::affineMul(const mat<4, T> &v) const
{
	return mat<4, T>(
			affineiprod0(v, 0, 0),
			affineiprod0(v, 0, 1),
			affineiprod0(v, 0, 2),
			affineiprod1(v, 0, 3),

			affineiprod0(v, 1, 0),
			affineiprod0(v, 1, 1),
			affineiprod0(v, 1, 2),
			affineiprod1(v, 1, 3),

			affineiprod0(v, 2, 0),
			affineiprod0(v, 2, 1),
			affineiprod0(v, 2, 2),
			affineiprod1(v, 2, 3),

			T(0),
			T(0),
			T(0),
			T(1));
}

template<class T>
__forceinline vec<4, T> mat<4, T>::affineMul(const vec<4, T> &v) const
{
	return vec<4, T>(
		a * v.x + b * v.y + c * v.z + d * v.w,
		e * v.x + f * v.y + g * v.z + h * v.w,
		i * v.x + j * v.y + k * v.z + l * v.w,
		v.w
	);
}

template<class T>
__forceinline vec<3, T> mat<4, T>::affineMul(const vec<3, T> &v) const
{
	return vec<3, T>(
		a * v.x + b * v.y + c * v.z + d,
		e * v.x + f * v.y + g * v.z + h,
		i * v.x + j * v.y + k * v.z + l
	);
}

/* Misc matrix operations */
template<class T>
__forceinline mat<4, T> &mat<4, T>::identity()
{
	a = f = k = p = 1;
	b = c = d = 0;
	e = g = h = 0;
	i = j = l = 0;
	m = n = o = 0;
	return *this;
}

template<class T>
__forceinline T mat<4, T>::det2(T n0, T n1, T n2, T n3) const
{
	return n0 * n3 - n1 * n2;
}

template<class T>
__forceinline T mat<4, T>::det() const
{
	T s0, s1, s2, s3, s4, s5;
	T c0, c1, c2, c3, c4, c5;

	s0 = det2(a, b, e, f);
	s1 = det2(a, c, e, g);
	s2 = det2(a, d, e, h);
	s3 = det2(b, c, f, g);
	s4 = det2(b, d, f, h);
	s5 = det2(c, d, g, h);

	c5 = det2(k, l, o, p);
	c4 = det2(j, l, n, p);
	c3 = det2(j, k, n, o);
	c2 = det2(i, l, m, p);
	c1 = det2(i, k, m, o);
	c0 = det2(i, j, m, n);

	return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
}

template<class T>
__forceinline mat<4, T> &mat<4, T>::inv()
{
	// TODO: Use det-code
	T s0, s1, s2, s3, s4, s5;
	T c0, c1, c2, c3, c4, c5;
	T deter, denom;

	s0 = det2(a, b, e, f);
	s1 = det2(a, c, e, g);
	s2 = det2(a, d, e, h);
	s3 = det2(b, c, f, g);
	s4 = det2(b, d, f, h);
	s5 = det2(c, d, g, h);

	c5 = det2(k, l, o, p);
	c4 = det2(j, l, n, p);
	c3 = det2(j, k, n, o);
	c2 = det2(i, l, m, p);
	c1 = det2(i, k, m, o);
	c0 = det2(i, j, m, n);

	deter = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

	if (deter == 0) {
		identity();
		return *this;
	}

	denom = T(1) / deter;

	mat<4, T> mat = *this;

	a = (mat.f * c5 - mat.g * c4 + mat.h * c3) * denom;
	b = (-mat.b * c5 + mat.c * c4 - mat.d * c3) * denom;
	c = (mat.n * s5 - mat.o * s4 + mat.p * s3) * denom;
	d = (-mat.j * s5 + mat.k * s4 - mat.l * s3) * denom;

	e = (-mat.e * c5 + mat.g * c2 - mat.h * c1) * denom;
	f = (mat.a * c5 - mat.c * c2 + mat.d * c1) * denom;
	g = (-mat.m * s5 + mat.o * s2 - mat.p * s1) * denom;
	h = (mat.i * s5 - mat.k * s2 + mat.l * s1) * denom;

	i = (mat.e * c4 - mat.f * c2 + mat.h * c0) * denom;
	j = (-mat.a * c4 + mat.b * c2 - mat.d * c0) * denom;
	k = (mat.m * s4 - mat.n * s2 + mat.p * s0) * denom;
	l = (-mat.i * s4 + mat.j * s2 - mat.l * s0) * denom;

	m = (-mat.e * c3 + mat.f * c1 - mat.g * c0) * denom;
	n = (mat.a * c3 - mat.b * c1 + mat.c * c0) * denom;
	o = (-mat.m * s3 + mat.n * s1 - mat.o * s0) * denom;
	p = (mat.i * s3 - mat.j * s1 + mat.k * s0) * denom;

	return *this;
}

template<class T>
__forceinline mat<4, T> mat<4, T>::getInverse() const
{
	mat<4, T> mat = *this;
	mat.inv();
	return mat;
}

// TODO: Reintroduce
#if 0
template<class T>
__forceinline void mat<4, T>::invertAffine()
{
	/* m, n, o = 0, p = 1 */
	T s0, s1, s2, s3, s4, s5;
	//T c0, c1, c2, c3, c4, c5;
	T deter, denom;

	s0 = det2(a, b, e, f);
	s1 = det2(a, c, e, g);
	s2 = det2(a, d, e, h);
	s3 = det2(b, c, f, g);
	s4 = det2(b, d, f, h);
	s5 = det2(c, d, g, h);

	c5 = k;
	c4 = j;
	c3 = 0;
	c2 = i;
	c1 = 0;
	c0 = 0;

	deter = s0 * k - s1 * j + s3 * i;

	if (deter == 0) {
		identity();
		return;
	}

	denom = T(1) / deter;

	mat<4, T> mat = *this;

	a = ( mat.f * k  - mat.g * j			  ) * denom;
	b = (-mat.b * k  + mat.c * j			  ) * denom;
	c = ( mat.n * s5 - mat.o * s4 + mat.p * s3) * denom;
	d = (-mat.j * s5 + mat.k * s4 - mat.l * s3) * denom;

	e = (-mat.e * k  + mat.g * i			  ) * denom;
	f = ( mat.a * k  - mat.c * i			  ) * denom;
	g = (-mat.m * s5 + mat.o * s2 - mat.p * s1) * denom;
	h = ( mat.i * s5 - mat.k * s2 + mat.l * s1) * denom;

	i = ( mat.e * c4 - mat.f * c2 + mat.h * c0) * denom;
	j = (-mat.a * c4 + mat.b * c2 - mat.d * c0) * denom;
	k = ( mat.m * s4 - mat.n * s2 + mat.p * s0) * denom;
	l = (-mat.i * s4 + mat.j * s2 - mat.l * s0) * denom;

	m = (-mat.e * c3 + mat.f * c1 - mat.g * c0) * denom;
	n = (mat.a * c3 - mat.b * c1 + mat.c * c0) * denom;
	o = (-mat.m * s3 + mat.n * s1 - mat.o * s0) * denom;
	p = (mat.i * s3 - mat.j * s1 + mat.k * s0) * denom;

}
#endif

template<class T>
__forceinline mat<3, T> mat<4, T>::getRotationMatrix() const
{
	return mat<3, T>(a, b, c,
		e, f, g,
		i, j, k);
}

/* Matlab-friendly print function */
// TODO: Overload <<
template<class T>
__forceinline void mat<4, T>::print()
{
	char buf[4096];

	sprintf(buf, "[ %f %f %f %f ;\n %f %f %f %f ;\n %f %f %f %f ;\n %f %f %f %f ]\n",
		a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);

	printf("%s\n", buf);
}


template<class T> T mul(const T &lv, const T &rv)
{
	return lv & rv;
}
template<> float mul(const float &lv, const float &rv)
{
	return lv * rv;
}

#endif // VMATH



