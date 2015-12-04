
template<typename T, typename Y, int N>
vec<N, T> cast(const vec<N, Y> &A)
{
	vec<N, T> out;
	LOOPN out[idx] = T(A[idx]);
	return out;
}




/* Comparison */
// TODO: Optimize, memcmp
template<int N, typename T>
FORCE_INLINE bool operator==(const vec<N, T> &A, const vec<N, T> &B)
{
	LOOPN if (A[idx] != B[idx]) return false; // TODO: Not SIMD friendly
	return true;
}

// TODO: Optimize
template<int N, typename T>
FORCE_INLINE bool operator!=(const vec<N, T> &A, const vec<N, T> &B)
{
	return !(A == B); // TODO: Not SIMD friendly
}

#define ARITHMETIC_OP(op1, op2) \
template<int N, typename T> vec<N, T> operator op1(const T &A, const vec<N, T> &B) { \
	vec<N, T> out; \
	LOOPN out[idx] = A op1 B[idx]; \
	return out; \
} \
template<int N, typename T> FORCE_INLINE vec<N, T> operator op1(const vec<N, T> &A, const vec<N, T> &B) { \
	vec<N, T> out; \
	LOOPN out[idx] = A[idx] op1 B[idx]; \
	return out; \
} \
template<int N, typename T> FORCE_INLINE vec<N, T> operator op1(const vec<N, T> &A, const T &B) { \
	vec<N, T> out; \
	LOOPN out[idx] = A[idx] op1 B; \
	return out; \
} \
template<int N, typename T> FORCE_INLINE vec<N, T> &operator op2(vec<N, T> &A, const vec<N, T> &B) { \
	LOOPN A[idx] op2 B[idx]; \
	return A; \
} \
template<int N, typename T> FORCE_INLINE vec<N, T> &operator op2(vec<N, T> &A, const T &B) { \
	LOOPN A[idx] op2 B; \
	return A; \
}
ARITHMETIC_OP(+, +=);
ARITHMETIC_OP(-, -=);
ARITHMETIC_OP(*, *=);
ARITHMETIC_OP(/, /=);
ARITHMETIC_OP(&, &=);
ARITHMETIC_OP(|, |=);
ARITHMETIC_OP(^, ^=);



template<int N, typename T> 
FORCE_INLINE vec<N, T> operator+(const vec<N, T> &A)
{
	return A;
}

template<int N, typename T> 
FORCE_INLINE vec<N, T> operator-(const vec<N, T> &A)
{
	vec<N, T> out;
	LOOPN out[idx] = -A[idx];
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> operator~(const vec<N, T> &A)
{
	vec<N, T> out;
	LOOPN out[idx] = ~A[idx];
	return out;
}

template<int N, typename T>
std::ostream &operator<<(std::ostream &os, vec<N, T> A)
{
	LOOPN os << A[idx] << " ";
	return os;
}

template<int N, typename T>
std::istream &operator>>(std::istream &is, vec<N, T> &A)
{
	LOOPN is >> A[idx];
	return is;
}



template<int N, typename T>
FORCE_INLINE T dot(const vec<N, T> &A, const vec<N, T> &B)
{
	T out = A[0] * B[0];
	for (i32 idx = 1; idx < N; idx++) out = fmadd(A[idx], B[idx], out);
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> clamp(const vec<N, T> &A, const T &min, const T &max)
{
	vec<N, T> out;
	LOOPN out[idx] = sclamp(A[idx], min, max);
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> clamp(const vec<N, T> &A, const vec<N, T> &min, const vec<N, T> &max)
{
	vec<N, T> out;
	LOOPN out[idx] = sclamp(A[idx], min[idx], max[idx]);
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> saturate(const vec<N, T> &A)
{
	vec<N, T> out;
	LOOPN ssaturate(A[idx]);
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> normalize(const vec<N, T> &A)
{
	T len = length(A);
	return A / len;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> floor(const vec<N, T> &A)
{
	vec<N, T> out;
	LOOPN out[idx] = floor(A[idx]);
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> ceil(const vec<N, T> &A)
{
	vec<N, T> out;
	LOOPN out[idx] = ceil(A[idx]);
	return out;
}

template<int N, typename T>
FORCE_INLINE T length(const vec<N, T> &A)
{
	return sqrt(dot(A, A));
}

template<int N, typename T>
FORCE_INLINE T lengthSquared(const vec<N, T> &A)
{
	return dot(A, A);
}

template<int N, typename T>
FORCE_INLINE vec<N, T> reflect(const vec<N, T> &A, const vec<N, T> &B)
{
	return fmsub(B, dot(A, B) * 2, A);
}

template<int N, typename T>
FORCE_INLINE vec<N, T> refract(const vec<N, T> &A, const vec<N, T> &B, const T &rho)
{
	T tr = -dot(A, B);
	T tc = 1 - rho * rho * (1 - tr * tr);
	if (tc < 0) return reflect(A, B); // TODO: Not SIMD friendly
	return A * rho + B * (rho * tr - sqrt(tc));
}

template<int N, typename T>
FORCE_INLINE vec<N, T> project(const vec<N, T> &A, const vec<N, T> &B)
{
	return B * (dot(A, B) / dot(B, B));
}

template<int N, typename T>
FORCE_INLINE T projectedLength(const vec<N, T> &A, const vec<N, T> &B)
{
	return dot(A, B) / length(B);
}


template<typename T>
FORCE_INLINE T minElement(const vec<2, T> &A)
{
	return smin<T>(A.x, A.y);
}

template<typename T>
FORCE_INLINE T maxElement(const vec<2, T> &A)
{
	return smax<T>(A.x, A.y);
}

template<typename T>
FORCE_INLINE T minElement(const vec<3, T> &A)
{
	return smin<T>(A.x, A.y, A.z);
}

template<typename T>
FORCE_INLINE T maxElement(const vec<3, T> &A)
{
	return smax<T>(A.x, A.y, A.z);
}

template<typename T>
FORCE_INLINE T minElement(const vec<4, T> &A)
{
	return smin<T>(A.x, A.y, A.z, A.w);
}

template<typename T>
FORCE_INLINE T maxElement(const vec<4, T> &A)
{
	return smax<T>(A.x, A.y, A.z, A.w);
}

template<typename T>
FORCE_INLINE int minElementIndex(const vec<2, T> &A)
{
	return smini<T>(A.x, A.y);
}

template<typename T>
FORCE_INLINE int maxElementIndex(const vec<2, T> &A)
{
	return smaxi<T>(A.x, A.y);
}

template<typename T>
FORCE_INLINE int minElementIndex(const vec<3, T> &A)
{
	return smini<T>(A.x, A.y, A.z);
}

template<typename T>
FORCE_INLINE int maxElementIndex(const vec<3, T> &A)
{
	return smaxi<T>(A.x, A.y, A.z);
}

template<typename T>
FORCE_INLINE int minElementIndex(const vec<4, T> &A)
{
	return smini<T>(A.x, A.y, A.z, A.w);
}

template<typename T>
FORCE_INLINE int maxElementIndex(const vec<4, T> &A)
{
	return smaxi<T>(A.x, A.y, A.z, A.w);
}

template<int N, typename T> vec<N-1, T> divW(const vec<N, T> &A)
{
	return vec<N-1, T>::Make(&A.x) / A[N-1];
}



template<typename T>
FORCE_INLINE T pseudoCross(const vec<2, T> &A, const vec<2, T> &B)
{
	return fmsub(A.x, B.y, A.y * B.x);
}

template<typename T>
FORCE_INLINE vec<3, T> cross(const vec<3, T> &A, const vec<3, T> &B)
{
	return vec<3, T>::Make(crossX(A, B), crossY(A, B), crossZ(A, B));
}

template<typename T>
FORCE_INLINE T crossX(const vec<3, T> &A, const vec<3, T> &B)
{
	return fmsub(A.y, B.z, A.z * B.y);
}

template<typename T>
FORCE_INLINE T crossY(const vec<3, T> &A, const vec<3, T> &B)
{
	return fmsub(A.z, B.x, A.x * B.z);
}

template<typename T>
FORCE_INLINE T crossZ(const vec<3, T> &A, const vec<3, T> &B)
{
	return fmsub(A.x, B.y, A.y * B.x);
}

template<typename T>
FORCE_INLINE T triple(const vec<3, T> &A, const vec<3, T> &B, const vec<3, T> &C)
{
	return dot(A, cross(B, C));
}

template<typename T>
FORCE_INLINE vec<2, T> rotate(const vec<2, T> &A, const T &rad)
{
	vec<2, T> out;
	T cs = cos(rad);
	T sn = sin(rad);
	out.x = fmsub(cs, A.x, sn * A.y);
	out.y = fmadd(sn, A.x, cs * A.y);
	return out;
}

template<typename T>
FORCE_INLINE vec<3, T> rotateX(const vec<3, T> &A, const T &rad)
{
	vec<3, T> out;
	T cs = cos(rad);
	T sn = sin(rad);
	out.x = A.x;
	out.y = fmsub(cs, A.y, sn * A.z);
	out.z = fmadd(sn, A.y, cs * A.z);
	return out;
}

template<typename T>
FORCE_INLINE vec<3, T> rotateY(const vec<3, T> &A, const T &rad)
{
	vec<3, T> out;
	T cs = cos(rad);
	T sn = sin(rad);
	out.x = fmadd(cs, A.x, sn * A.z);
	out.y = A.y;
	out.z = fmsub(cs, A.z, sn * A.x);
	return out;
}

template<typename T>
FORCE_INLINE vec<3, T> rotateZ(const vec<3, T> &A, const T &rad)
{
	vec<3, T> out;
	T cs = cos(rad);
	T sn = sin(rad);
	out.y = fmsub(cs, A.x, sn * A.y);
	out.z = fmadd(sn, A.x, cs * A.y);
	out.z = A.z;
	return out;
}

template<typename T>
FORCE_INLINE vec<3, T> rotate(const vec<3, T> &A, const T &rad, const vec<3, T> &axis)
{
	mat<3, T> r = RotationMatrix(rad, axis);
	return r * A;
}

template<typename T>
vec<3, T> RGBtoHSV(const vec<3, T> &A)
{
	int imin = minElementIndex(A);
	int imax = maxElementIndex(A);
	T d = A[imax] - A[imin];
	if (A[imax] == 0)
		return vec<3, T>::Make(0, 0, A[imax]); // TODO: Not SIMD friendly
	const int next[] = {1, 2, 0, 1};
	vec<3, T> out = vec<3, T>::Make(60 * (imax * 2 + (A[next[imax]] - A[next[imax + 1]]) / d), d / A[imax], A[imax]);
	if (out.x < 0)
		out.x += 360;
	return out;
}

template<typename T>
vec<3, T> HSVtoRGB(const vec<3, T> &A)
{
    T hh = A.x;
    while (hh >= (T) 360)
		hh -= (T) 360;
    hh /= (T) 60;
    size_t i = size_t(hh);
	T ff = hh - (T) i;//hh - splat<T>(i);
    T p = A.z * (T(1) - A.y);
    T q = A.z * (T(1) - (A.y * ff));
    T t = A.z * (T(1) - (A.y * (T(1) - ff)));

    switch(i) {
		case 0:	return vec<3, T>::Make(A.z, t, p); // TODO: Not SIMD friendly
		case 1:	return vec<3, T>::Make(q, A.z, p);
		case 2:	return vec<3, T>::Make(p, A.z, t);
		case 3:	return vec<3, T>::Make(p, q, A.z);
		case 4:	return vec<3, T>::Make(t, p, A.z);
		default: break;
	}
	return vec<3, T>::Make(A.z, p, q);
}




template<typename T>
FORCE_INLINE vec<4, T> point4(const vec<2, T> &A)
{
	return vec<4, T>::Make(A.x, A.y, 0, 1);
}

template<typename T>
FORCE_INLINE vec<4, T> point4(const vec<3, T> &A)
{
	return vec<4, T>::Make(A.x, A.y, A.z, 1);
}

template<typename T>
FORCE_INLINE vec<4, T> point4(const vec<4, T> &A)
{
	return A;
}
