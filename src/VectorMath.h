#pragma once

/*
 * Vector and Matrix operations (along with various other constants and math
 * stuff...)
 */

/*
TODO:
 * Affine transforms
 */

/*============================================================[ Settings ]====*/

// Enable to sacrifice precision for speed
#define FAST_FLOAT
//#define FAST_DOUBLE

/*============================================================================*/

#include "Types.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <cfloat>

// TODO figure out what fMax was supposed to be
#define fMax FLT_MAX

#ifdef _WIN32
#	pragma warning(disable: 4201) // Non standard, nameless struct/union
//#	pragma warning(disable: 4100) // Unreferenced parameter
#	pragma warning(disable: 4706) // Assignment within conditional expr.
#endif // WIN32

/*===========================================================[ Precision ]====*/

// TODO: Use constexpr instead
template<typename T> FORCE_INLINE bool PreciseMath()	{return true;}

#ifdef FAST_FLOAT
template<> FORCE_INLINE bool PreciseMath<float>()		{return false;}
#endif
#ifdef FAST_DOUBLE
template<> FORCE_INLINE bool PreciseMath<double>()		{return false;}
#endif

/*===========================================================[ Constants ]====*/

#define fPI			3.141592653589793238462643383280f
#define f2PI		6.283185307179586476925286766560f
#define fPIinv		0.318309886183790671537767526745f
#define f2PIinv		0.159154943091895335768883763373f

#define dPI			3.141592653589793238462643383280
#define d2PI		6.283185307179586476925286766560
#define dPIinv		0.318309886183790671537767526745
#define d2PIinv		0.159154943091895335768883763373

/*==============================================================[ Scalar ]====*/

template<typename T> FORCE_INLINE T sabs(const T &a) { // TODO: Xor
	return a < 0 ? -a : a;
}
template<typename T> FORCE_INLINE T smax(const T &a, const T &b) {
	return a > b ? a : b;
}
template<typename T> FORCE_INLINE T smin(const T &a, const T &b) {
	return a < b ? a : b;
}
template<typename T> FORCE_INLINE T smax(const T &a, const T &b, const T &c) {
	return smax<T>(a, smax<T>(b, c));
}
template<typename T> FORCE_INLINE T smid(const T &a, const T &b, const T &c) {
	return a > b ? (a < c ? a : (b > c ? b : c)) : (b < c ? b : (a > c ? a : c));
}
template<typename T> FORCE_INLINE T smin(const T &a, const T &b, const T &c) {
	return smin<T>(a, smin<T>(b, c));
}
template<typename T> FORCE_INLINE T smax(const T &a, const T &b, const T &c, const T &d) {
	return smax<T>(smax<T>(a, b), smax<T>(c, d));
}
template<typename T> FORCE_INLINE T smin(const T &a, const T &b, const T &c, const T &d) {
	return smin<T>(smin<T>(a, b), smin<T>(c, d));
}
template<typename T> FORCE_INLINE i32 smaxi(const T &a, const T &b) {
	return a > b ? 0 : 1;
}
template<typename T> FORCE_INLINE i32 smini(const T &a, const T &b) {
	return a < b ? 0 : 1;
}
template<typename T> FORCE_INLINE i32 smaxi(const T &a, const T &b, const T &c) {
	return a > c ? smaxi<T>(a, b) : (smaxi<T>(b, c) + 1);
}
template<typename T> FORCE_INLINE i32 smidi(const T &a, const T &b, const T &c) {
	return a > b ? (a < c ? 0 : (b > c ? 1 : 2)) : (b < c ? 1 : (a > c ? 0 : 2));
}
template<typename T> FORCE_INLINE i32 smini(const T &a, const T &b, const T &c) {
	return a < c ? smini<T>(a, b) : (smini<T>(b, c) + 1);
}
template<typename T> FORCE_INLINE i32 smaxi(const T &a, const T &b, const T &c, const T &d) {
	return smaxi<T>(smaxi<T>(a, b), smaxi<T>(c, d) + 2);
}
template<typename T> FORCE_INLINE i32 smini(const T &a, const T &b, const T &c, const T &d) {
	return smini<T>(smini<T>(a, b), smini<T>(c, d) + 2);
}
template<typename T> FORCE_INLINE T sclamp(const T &s, const T &a, const T &b) {
	return s > b ? b : (s < a ? a : s);
}
template<typename T> FORCE_INLINE T ssaturate(const T &s) {
	return sclamp(s, 0, 1);
}
template<typename T> FORCE_INLINE void sswap(T &a, T &b) {
	std::swap(a, b);
}
template<typename T> FORCE_INLINE void ssort(T &a, T &b) {
	if (a > b) std::swap(a, b);
}
template<typename T> FORCE_INLINE T fmadd (const T &A, const T &B, const T &C)		{return A * B + C;}
template<typename T> FORCE_INLINE T fmsub (const T &A, const T &B, const T &C)		{return A * B - C;}
template<typename T> FORCE_INLINE T fnmadd(const T &A, const T &B, const T &C)		{return C - (A * B);}
template<typename T> FORCE_INLINE T fnmsub(const T &A, const T &B, const T &C)		{return -(A * B) - C;}
template<typename T> FORCE_INLINE T rcp(const T &A)								{return 1 / A;}
template<typename T> FORCE_INLINE T rsqrt(const T &A)								{return 1 / sqrt(A);}
template<typename T, typename Y> FORCE_INLINE T splat(Y A)							{return (T) A;}

#define toRadf(a)	((a) * 0.01745329251994329576923690768489f)
#define toDegf(a)	((a) * 57.29577951308232087679815481428f)
#define toRadd(a)	((a) * 0.01745329251994329576923690768489)
#define toDegd(a)	((a) * 57.29577951308232087679815481428)

/*=================================================[ Forward declaration ]====*/

template<int N, typename T> struct vec;
template<int N, typename T> struct mat;

/*===============================================[ Function declarations ]====*/

template<typename T, typename Y, int N> vec<N, T> cast(const vec<N, Y> &A);
template<typename T, typename Y, int N> mat<N, T> cast(const mat<N, Y> &A);

template<int N, typename T> bool		operator==			(const vec<N, T> &A, const vec<N, T> &B);
template<int N, typename T> bool		operator!=			(const vec<N, T> &A, const vec<N, T> &B);
template<int N, typename T> bool		operator==			(const mat<N, T> &A, const mat<N, T> &B);
template<int N, typename T> bool		operator!=			(const mat<N, T> &A, const mat<N, T> &B);
template<int N, typename T> bool		isIdentity			(const mat<N, T> &A);

#define ARITHMETIC_OP_DECL(op1, op2) \
	template<int N, typename T> vec<N, T>	operator op1	(const T &A, const vec<N, T> &B); \
	template<int N, typename T> vec<N, T>	operator op1	(const vec<N, T> &A, const vec<N, T> &B); \
	template<int N, typename T> vec<N, T>	operator op1	(const vec<N, T> &A, const T &B); \
	template<int N, typename T> vec<N, T>	&operator op2(vec<N, T> &A, const vec<N, T> &B); \
	template<int N, typename T> vec<N, T>	&operator op2(vec<N, T> &A, const T &B);
ARITHMETIC_OP_DECL(+, +=)
ARITHMETIC_OP_DECL(-, -=)
ARITHMETIC_OP_DECL(*, *=)
ARITHMETIC_OP_DECL(/, /=)
ARITHMETIC_OP_DECL(&, &=)
ARITHMETIC_OP_DECL(|, |=)
ARITHMETIC_OP_DECL(^, ^=)
#undef ARITHMETIC_OP_DECL

template<int N, typename T> vec<N, T>	operator+			(const vec<N, T> &A);
template<int N, typename T> vec<N, T>	operator-			(const vec<N, T> &A);
template<int N, typename T> vec<N, T>	operator~			(const vec<N, T> &A);
template<int N, typename T> mat<N, T>	operator+			(const mat<N, T> &A);
template<int N, typename T> mat<N, T>	operator-			(const mat<N, T> &A);
template<int N, typename T> mat<N, T>	operator~			(const mat<N, T> &A);

template<int N, typename T> vec<N, T>	operator*			(const vec<N, T> &A, const mat<N, T> &B);
template<int N, typename T> vec<N, T>	operator*			(const mat<N, T> &A, const vec<N, T> &B);
template<int N, typename T> mat<N, T>	operator*			(const mat<N, T> &A, const mat<N, T> &B);
template<int N, typename T> vec<N, T>	&operator*=			(vec<N, T> &A, const mat<N, T> &B);
template<int N, typename T> mat<N, T>	&operator*=			(mat<N, T> &A, const mat<N, T> &B);

template<int N, typename T> T			dot					(const vec<N, T> &A, const vec<N, T> &B);
template<int N, typename T> vec<N, T>	clamp				(const vec<N, T> &A, const T &min, const T &max);
template<int N, typename T> vec<N, T>	clamp				(const vec<N, T> &A, const vec<N, T> &min, const vec<N, T> &max);
template<int N, typename T> vec<N, T>	saturate			(const vec<N, T> &A);
template<int N, typename T> vec<N, T>	normalize			(const vec<N, T> &A);
template<int N, typename T> vec<N, T>	floor				(const vec<N, T> &A);
template<int N, typename T> vec<N, T>	ceil				(const vec<N, T> &A);
template<int N, typename T> T			length				(const vec<N, T> &A);
template<int N, typename T> T			lengthSquared		(const vec<N, T> &A);
template<int N, typename T> vec<N, T>	reflect				(const vec<N, T> &A, const vec<N, T> &B);
template<int N, typename T> vec<N, T>	refract				(const vec<N, T> &A, const vec<N, T> &B, const T &rho);
template<int N, typename T> vec<N, T>	project				(const vec<N, T> &A, const vec<N, T> &B);
template<int N, typename T> T			projectedLength		(const vec<N, T> &A, const vec<N, T> &B);

template<int N, typename T> T			minElement			(const vec<N, T> &A);
template<int N, typename T> T			maxElement			(const vec<N, T> &A);
template<int N, typename T> int			minElementIndex		(const vec<N, T> &A);
template<int N, typename T> int			maxElementIndex		(const vec<N, T> &A);

template<int N, typename T> vec<N-1, T> divW				(const vec<N, T> &A);

template<typename T> T					pseudoCross			(const vec<2, T> &A, const vec<2, T> &B);
template<typename T> vec<3, T>			cross				(const vec<3, T> &A, const vec<3, T> &B);
template<typename T> T					crossX				(const vec<3, T> &A, const vec<3, T> &B);
template<typename T> T					crossY				(const vec<3, T> &A, const vec<3, T> &B);
template<typename T> T					crossZ				(const vec<3, T> &A, const vec<3, T> &B);
template<typename T> T					triple				(const vec<3, T> &A, const vec<3, T> &B, const vec<3, T> &C);

template<typename T> vec<2, T>			rotate				(const vec<2, T> &A, const T &rad);
template<typename T> vec<3, T>			rotateX				(const vec<3, T> &A, const T &rad);
template<typename T> vec<3, T>			rotateY				(const vec<3, T> &A, const T &rad);
template<typename T> vec<3, T>			rotateZ				(const vec<3, T> &A, const T &rad);
template<typename T> vec<3, T>			rotate				(const vec<3, T> &A, const T &rad, const vec<3, T> &axis);

template<typename T> vec<3, T>			RGBtoHSV			(const vec<3, T> &A);
template<typename T> vec<3, T>			HSVtoRGB			(const vec<3, T> &A);

template<int N, typename T>	T			innerProd			(const mat<N, T> &A, const mat<N, T> &B, u32 Arow, u32 Bcol);
template<int N, typename T>	vec<N, T>	getRow				(const mat<N, T> &A, u32 row);
template<int N, typename T>	void		setRow				(mat<N, T> &A, u32 row, const vec<N, T> &B);
template<int N, typename T>	vec<N, T>	getCol				(const mat<N, T> &A, u32 col);
template<int N, typename T>	void		setCol				(mat<N, T> &A, u32 col, const vec<N, T> &B);
template<int N, typename T>	vec<N, T>	getDiag				(const mat<N, T> &A);
template<int N, typename T>	vec<N, T>	up					(const mat<N, T> &A);
template<int N, typename T>	vec<N, T>	down				(const mat<N, T> &A);
template<int N, typename T>	vec<N, T>	left				(const mat<N, T> &A);
template<int N, typename T>	vec<N, T>	right				(const mat<N, T> &A);
template<int N, typename T>	vec<N, T>	front				(const mat<N, T> &A);
template<int N, typename T>	vec<N, T>	back				(const mat<N, T> &A);

template<int N, typename T> T			det					(const mat<N, T> &A);
template<int N, typename T> T			trace				(const mat<N, T> &A);
template<int N, typename T> mat<N, T>	inv					(const mat<N, T> &A);
template<int N, typename T> mat<N, T>	transpose			(const mat<N, T> &A);
template<int N, typename T> mat<N, T>	elemMul				(const mat<N, T> &A, const mat<N, T> &B);
template<int N, typename T> mat<N, T>	elemDiv				(const mat<N, T> &A, const mat<N, T> &B);
template<int N, typename T> vec<N, T>	solveSystem			(const mat<N, T> &A, const vec<N, T> &eq);

template<typename T> vec<2, T>			eigenvalues			(const mat<2, T> &A);
template<typename T> mat<2, T>			eigenvectors		(const mat<2, T> &A);
template<typename T> mat<2, T>			eigenvectors		(const mat<2, T> &A, const vec<2, T> &eigVals);

template<typename T> mat<4, T>			affineMul			(const mat<4, T> &A, const mat<4, T> &B);
template<typename T> vec<4, T>			affineMul			(const mat<4, T> &A, const vec<4, T> &B);
template<typename T> vec<3, T>			affineMul0			(const mat<4, T> &A, const vec<3, T> &B);
template<typename T> vec<3, T>			affineMul1			(const mat<4, T> &A, const vec<3, T> &B);

template<int N, typename T> std::ostream &operator<<(std::ostream &os, vec<N, T> A);
template<int N, typename T> std::istream &operator>>(std::istream &is, vec<N, T> &A);
template<int N, typename T> std::ostream &operator<<(std::ostream &os, mat<N, T> A);
template<int N, typename T> std::istream &operator>>(std::istream &is, mat<N, T> &A);

void									sseMul				(mat<4, float> &C, const mat<4, float> &A, const mat<4, float> &B);
void									avxMul				(mat<4, float> &C, const mat<4, float> &A, const mat<4, float> &B);

/*===============================================================[ Setup ]====*/

//base_type() {} \
//explicit base_type(const T &A) { LOOPN_(N) (*this)[idx] = A; } \
//base_type(const base_type<N, T> &A) { LOOPN_(ELEMS) (*this)[idx] = A[idx]; } \

//base_type<N, T> &operator=(const base_type<N, T> &A) {
//	\
//		LOOPN_(ELEMS) (*this)[idx] = A[idx]; \
//		return *this; \
//} \

#define LOOPN		for (i32 idx = 0; idx < N; idx++)
#define LOOPN2		for (i32 idx = 0; idx < N * N; idx++)
#define LOOPNN		for (i32 icol = 0; icol < N; icol++) \
					for (i32 irow = 0; irow < N; irow++)
#define LOOPN_(N)	for (i32 idx = 0; idx < N; idx++)
#define LOOPN2_(N)	for (i32 idx = 0; idx < N * N; idx++)
#define LOOPNN_(N)	for (i32 icol = 0; icol < N; icol++) \
					for (i32 irow = 0; irow < N; irow++)
#undef N

#define BASE_DECL(base_type, N, ELEMS) \
	static base_type<N, T> Make(const T &A) { base_type<N, T> out; LOOPN_(N) out[idx] = A; return out; } \
	static base_type<N, T> Make(const base_type<N, T> &A) { base_type<N, T> out; memcpy(&out[0], &A, sizeof(T) * ELEMS); return out; } \
	T &operator[](int idx) { return ((T *) &V[0])[idx]; } \
	const T operator[](int idx) const { return ((T *) &V[0])[idx]; } \
	operator T*() {return &(*this)[0];} \
	typedef T StorageType; \
	static int Dim() {return N;} \
	static int Elements() {return ELEMS;}

//vec(const T *A) {memcpy(&V[0], A, sizeof(T) * N);}

#define VEC_DECL(N)	BASE_DECL(vec, N, N) \
	static vec<N, T> Make(const T *A) {vec<N, T> out; memcpy(&out.V[0], A, sizeof(T) * N); return out;}
#define MAT_DECL(N)	BASE_DECL(mat, N, (N * N)) \
	union {T V[N * N]; T M[N][N];}; \
	const T cr(int col, int row) const {return M[col][row];} \
	const T rc(int row, int col) const {return M[col][row];} \
	const T xy(int x, int y) const {return M[x][y];} \
	const T yx(int y, int x) const {return M[x][y];} \
	T &cr(int col, int row) {return M[col][row];} \
	T &rc(int row, int col) {return M[col][row];} \
	T &xy(int x, int y) {return M[x][y];} \
	T &yx(int y, int x) {return M[x][y];} \
	const T &operator()(int row, int col) const {return M[col][row];} \
	T &operator()(int row, int col) {return M[col][row];}

template<int N, typename T> struct vec {
	VEC_DECL(N)

	T V[N];
};
template<int N, typename T> struct mat {
	MAT_DECL(N)
};

/*===========================================================[ 2D Vector ]====*/

template<typename T>
struct vec<2, T>
{
	VEC_DECL(2)

	union {
		struct {T x, y;};
		struct {T u, v;};
		T V[2];
	};

	static vec<2, T> Make(const T &nx, const T &ny) {
		vec<2, T> A;
		A.x = nx; A.y = ny;
		return A;
	}

	vec<3, T> xy0() const				{return vec<3, T>::Make(x, y, 0);}
	vec<3, T> xy1() const				{return vec<3, T>::Make(x, y, 1);}
	vec<4, T> xy01() const				{return vec<4, T>::Make(x, y, 0, 1);}
	static vec<2, T> nullVector()		{return vec<2, T>::Make(0, 0);}
};

/*===========================================================[ 3D Vector ]====*/

template<typename T> 
struct vec<3, T>
{
	VEC_DECL(3)

	union {
		struct {T x, y, z;};
		struct {T u, v, w;};
		struct {T r, g, b;};
		T V[3];
	};

	static vec<3, T> Make(const T &nx, const T &ny, const T &nz) {
		vec<3, T> A;
		A.x = nx; A.y = ny; A.z = nz;
		return A;
	}
	static vec<3, T> Make(const vec<2, T> &v, const T &nz) {
		return Make(v.x, v.y, nz);
	}
	static vec<3, T> Make(const T &nxy, const T &nz) {
		return Make(nxy, nxy, nz);
	}

	vec<2, T> xy() const				{return vec<2, T>::Make(x, y);}
	vec<4, T> xyz0() const				{return vec<4, T>::Make(x, y, z, 0);}
	vec<4, T> xyz1() const				{return vec<4, T>::Make(x, y, z, 1);}
	vec<2, T> xz() const				{return vec<2, T>::Make(x, z);}
	static vec<3, T> nullVector()		{return vec<3, T>::Make(0, 0, 0);}
};

/*===========================================================[ 4D Vector ]====*/

template<typename T>
struct vec<4, T>
{
	VEC_DECL(4)

	union {
		struct {T x, y, z, w;};
		struct {T r, g, b, a;};
		T V[4];
	};

	static vec<4, T> Make(const T &nx, const T &ny, const T &nz, const T &nw) {
		vec<4, T> A;
		A.x = nx; A.y = ny; A.z = nz; A.w = nw;
		return A;
	}
	static vec<4, T> Make(const vec<3, T> &v, const T &nw) {
		return Make(v.x, v.y, v.z, nw);
	}
	static vec<4, T> Make(const vec<2, T> &v, const T &nz, const T &nw) {
		return Make(v.x, v.y, nz, nw);
	}
	static vec<4, T> Make(const T &nxyz, const T &nw) {
		return Make(nxyz, nxyz, nxyz, nw);
	}

	vec<4, T> xyz1() const				{return vec<4, T>::Make(x, y, z, 1);}
	vec<3, T> xyz() const				{return vec<3, T>::Make(x, y, z);}
	vec<3, T> xyw() const				{return vec<3, T>::Make(x, y, w);}
	vec<2, T> xy() const				{return vec<2, T>::Make(x, y);}
	vec<2, T> xz() const				{return vec<2, T>::Make(x, z);}
	vec<3, T> xy0() const				{return vec<3, T>::Make(x, y, 0);}
	vec<3, T> xy1() const				{return vec<3, T>::Make(x, y, 1);}
	vec<3, T> xz0() const				{return vec<3, T>::Make(x, z, 0);}
	vec<3, T> xz1() const				{return vec<3, T>::Make(x, z, 1);}
	vec<3, T> yzw() const				{return vec<3, T>::Make(y, z, w);}

	static vec<4, T> nullVector()		{return vec<4, T>::Make(0, 0, 0, 0);}
};

/*==========================================================[ 2x2 Matrix ]====*/

template<typename T>
struct mat<2, T>
{
	MAT_DECL(2)

	static mat<2, T> RowMake(const T *ptr);
	static mat<2, T> RowMake(const vec<2, T> &row0, const vec<2, T> &row1);
	static mat<2, T> RowMake(const T &r0x, const T &r0y,
							 const T &r1x, const T &r1y);
	static mat<2, T> ColMake(const T *ptr);
	static mat<2, T> ColMake(const vec<2, T> &col0, const vec<2, T> &col1);
	static mat<2, T> ColMake(const T &c0x, const T &c0y,
							 const T &c1x, const T &c1y);

	static mat<2, T> Identity() {return mat<2, T>::ColMake(1, 0, 0, 1);}
	static mat<2, T> RotationMatrix(const T &rad);

	mat<3, T> mat3();
	mat<4, T> mat4();
};

/*==========================================================[ 3x3 Matrix ]====*/

template<typename T>
struct mat<3, T>
{
	MAT_DECL(3)

	/* Members */

	static mat<3, T> RowMake(const T *ptr);
	static mat<3, T> RowMake(const vec<3, T> &row0, const vec<3, T> &row1, const vec<3, T> &row2);
	static mat<3, T> RowMake(const T &r0x, const T &r0y, const T &r0z,
							 const T &r1x, const T &r1y, const T &r1z,
							 const T &r2x, const T &r2y, const T &r2z);
	static mat<3, T> ColMake(const T *ptr);
	static mat<3, T> ColMake(const vec<3, T> &col0, const vec<3, T> &col1, const vec<3, T> &col2);
	static mat<3, T> ColMake(const T &c0x, const T &c0y, const T &c0z,
							 const T &c1x, const T &c1y, const T &c1z,
							 const T &c2x, const T &c2y, const T &c2z);

	static mat<3, T> Identity() {return mat<3, T>::ColMake(1, 0, 0, 0, 1, 0, 0, 0, 1);}
	static mat<3, T> RotationMatrix(const T &rad, const vec<3, T> &axis);
	static mat<3, T> RotationXMatrix(const T &rad);
	static mat<3, T> RotationYMatrix(const T &rad);
	static mat<3, T> RotationZMatrix(const T &rad);
	static mat<3, T> ScaleMatrix(T &A);
	static mat<3, T> ScaleMatrix(const vec<3, T> &A);

	mat<4, T> mat4();
};

/*==========================================================[ 4x4 Matrix ]====*/

template<typename T>
struct mat<4, T>
{
	MAT_DECL(4)

	static mat<4, T> RowMake(const T *ptr);
	static mat<4, T> RowMake(const vec<4, T> &row0, const vec<4, T> &row1, const vec<4, T> &row2, const vec<4, T> row3);
	static mat<4, T> RowMake(const T &r0x, const T &r0y, const T &r0z, const T &r0w,
							 const T &r1x, const T &r1y, const T &r1z, const T &r1w,
							 const T &r2x, const T &r2y, const T &r2z, const T &r2w,
							 const T &r3x, const T &r3y, const T &r3z, const T &r3w);
	static mat<4, T> ColMake(const T *ptr);
	static mat<4, T> ColMake(const vec<4, T> &col0, const vec<4, T> &col1, const vec<4, T> &col2, const vec<4, T> col3);
	static mat<4, T> ColMake(const T &c0x, const T &c0y, const T &c0z, const T &c0w,
							 const T &c1x, const T &c1y, const T &c1z, const T &c1w,
							 const T &c2x, const T &c2y, const T &c2z, const T &c2w,
							 const T &c3x, const T &c3y, const T &c3z, const T &c3w);

	static mat<4, T> Identity() {return mat<4, T>::ColMake(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);}
	static mat<4, T> NdcToScreen(int resX, int resY);
	// Note: set nfar = nnear for inf-projection
	static mat<4, T> PerspectiveProjection(T fovy, T aspect, T nnear, T nfar); 
	static mat<4, T> PerspectiveProjectionInverse(T fovy, T aspect, T nnear, T nfar);
	static mat<4, T> PerspectiveProjectionReversed(T fovy, T aspect, T nnear, T nfar);
	static mat<4, T> PerspectiveProjectionReversedInverse(T fovy, T aspect, T nnear, T nfar);
	static mat<4, T> OrthogonalProjection(T left, T right, T bottom, T top, T nnear, T nfar);
	static mat<4, T> OrthogonalProjectionInverse(T left, T right, T bottom, T top, T nnear, T nfar);

	static mat<4, T> RotationMatrix(const T &rad, const vec<3, T> &axis) {
		return mat<3, T>::RotationMatrix(rad, axis).mat4();
	}
	static mat<4, T> RotationXMatrix(const T &rad) {
		return mat<3, T>::RotationXMatrix(rad).mat4();
	}
	static mat<4, T> RotationYMatrix(const T &rad) {
		return mat<3, T>::RotationYMatrix(rad).mat4();
	}
	static mat<4, T> RotationZMatrix(const T &rad) {
		return mat<3, T>::RotationZMatrix(rad).mat4();
	}
	static mat<4, T> TranslationMatrix(const vec<3, T> &A);
	static mat<4, T> ScaleMatrix(T &A) {
		return mat<3, T>::ScaleMatrix(A).mat4();
	}
	static mat<4, T> ScaleMatrix(const vec<3, T> &A) {
		return mat<3, T>::ScaleMatrix(A).mat4();
	}

	mat<3, T> R() const;
};

/*================================================[ Function definitions ]====*/

#include "VectorMath.vecimpl.inl"
#include "VectorMath.matimpl.inl"

/*============================================================[ Typedefs ]====*/

#define VM_TYPEDEF(T, chr) \
	typedef vec<2, T> vec2##chr; \
	typedef vec<3, T> vec3##chr; \
	typedef vec<4, T> vec4##chr; \
	typedef mat<2, T> mat2##chr; \
	typedef mat<3, T> mat3##chr; \
	typedef mat<4, T> mat4##chr; \
	FORCE_INLINE vec<2, T> v2##chr(const T *x)										{return vec<2, T>::Make(x);} \
	FORCE_INLINE vec<3, T> v3##chr(const T *x)										{return vec<3, T>::Make(x);} \
	FORCE_INLINE vec<4, T> v4##chr(const T *x)										{return vec<4, T>::Make(x);} \
	FORCE_INLINE vec<2, T> v2##chr(const T &x)										{return vec<2, T>::Make(x);} \
	FORCE_INLINE vec<3, T> v3##chr(const T &x)										{return vec<3, T>::Make(x);} \
	FORCE_INLINE vec<4, T> v4##chr(const T &x)										{return vec<4, T>::Make(x);} \
	FORCE_INLINE vec<2, T> v2##chr(const vec<2, T> &x)								{return vec<2, T>::Make(x);} \
	FORCE_INLINE vec<3, T> v3##chr(const vec<3, T> &x)								{return vec<3, T>::Make(x);} \
	FORCE_INLINE vec<4, T> v4##chr(const vec<4, T> &x)								{return vec<4, T>::Make(x);} \
	FORCE_INLINE vec<2, T> v2##chr(const T &x, const T &y)							{return vec<2, T>::Make(x, y);} \
	FORCE_INLINE vec<3, T> v3##chr(const T &x, const T &y, const T &z)				{return vec<3, T>::Make(x, y, z);} \
	FORCE_INLINE vec<4, T> v4##chr(const T &x, const T &y, const T &z, const T &w)	{return vec<4, T>::Make(x, y, z, w);} \
	FORCE_INLINE vec<3, T> v3##chr(const vec<2, T> &xy, const T &z)				{return vec<3, T>::Make(xy, z);} \
	FORCE_INLINE vec<3, T> v3##chr(const T &xy, const T &z)						{return vec<3, T>::Make(xy, z);} \
	FORCE_INLINE vec<4, T> v4##chr(const vec<3, T> &xyz, const T &w)				{return vec<4, T>::Make(xyz, w);} \
	FORCE_INLINE vec<4, T> v4##chr(const vec<2, T> &xy, const T &z, const T &w)	{return vec<4, T>::Make(xy, z, w);} \
	FORCE_INLINE vec<4, T> v4##chr(const T &xyz, const T &w)						{return vec<4, T>::Make(xyz, w);}

VM_TYPEDEF(f32, f )
VM_TYPEDEF(f64, d )
VM_TYPEDEF(i8 , c )
VM_TYPEDEF(i16, s )
VM_TYPEDEF(i32, i )
VM_TYPEDEF(i64, l )
VM_TYPEDEF(u8 , uc)
VM_TYPEDEF(u16, us)
VM_TYPEDEF(u32, u )
VM_TYPEDEF(u32, ui)
VM_TYPEDEF(u64, ul)

/*----------------------------------------------------------------------------*/
