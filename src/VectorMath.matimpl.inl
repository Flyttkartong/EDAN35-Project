template<typename T, typename Y, int N>
mat<N, T> cast(const mat<N, Y> &m)
{
	mat<N, T> out;
	LOOPN2 out[idx] = T(m[idx]);
	return out;
}


template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::RowMake(const T *ptr)
{
	mat<2, T> A;
	LOOPNN_(2) A.cr(icol, irow) = ptr[irow * 2 + icol];
	return A;
}
template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::RowMake(const vec<2, T> &row0, const vec<2, T> &row1)
{
	mat<2, T> A;
	setRow(A, 0, row0);
	setRow(A, 1, row1);
	return A;
}
template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::RowMake(const T &r0x, const T &r0y,
								const T &r1x, const T &r1y)
{
	mat<2, T> A;
	A.M[0][0] = r0x; A.M[0][1] = r1x;
	A.M[1][0] = r0y; A.M[1][1] = r1y;
	return A;
}
template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::ColMake(const T *ptr)
{
	mat<2, T> A;
	memcpy(A.M, ptr, sizeof(T) * 4);
	return A;
}
template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::ColMake(const vec<2, T> &col0, const vec<2, T> &col1)
{
	mat<2, T> A;
	setCol(A, 0, col0);
	setCol(A, 1, col1);
	return A;
}
template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::ColMake(const T &c0x, const T &c0y,
								const T &c1x, const T &c1y)
{
	mat<2, T> A;
	A.M[0][0] = c0x; A.M[0][1] = c0y;
	A.M[1][0] = c1x; A.M[1][1] = c1y;
	return A;
}


template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::RowMake(const T *ptr)
{
	mat<3, T> A;
	LOOPNN_(3) A.cr(icol, irow) = ptr[irow * 3 + icol];
	return A;
}
template<typename T>
FORCE_INLINE mat<3, T> RowMake(const vec<3, T> &row0, const vec<3, T> &row1, const vec<3, T> &row2)
{
	mat<3, T> A;
	setRow(A, 0, row0);
	setRow(A, 1, row1);
	setRow(A, 2, row2);
	return A;
}
template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::RowMake(const T &r0x, const T &r0y, const T &r0z,
								const T &r1x, const T &r1y, const T &r1z,
								const T &r2x, const T &r2y, const T &r2z)
{
	mat<3, T> A;
	A.M[0][0] = r0x; A.M[0][1] = r1x; A.M[0][2] = r2x;
	A.M[1][0] = r0y; A.M[1][1] = r1y; A.M[1][2] = r2y;
	A.M[2][0] = r0z; A.M[2][1] = r1z; A.M[2][2] = r2z;
	return A;
}
template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::ColMake(const T *ptr)
{
	mat<3, T> A;
	memcpy(A.M, ptr, sizeof(T) * 9);
	return A;
}
template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::ColMake(const vec<3, T> &col0, const vec<3, T> &col1, const vec<3, T> &col2)
{
	mat<3, T> A;
	setCol(A, 0, col0);
	setCol(A, 1, col1);
	setCol(A, 2, col2);
	return A;
}
template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::ColMake(const T &c0x, const T &c0y, const T &c0z,
								const T &c1x, const T &c1y, const T &c1z,
								const T &c2x, const T &c2y, const T &c2z)
{
	mat<3, T> A;
	A.M[0][0] = c0x; A.M[0][1] = c0y; A.M[0][2] = c0z;
	A.M[1][0] = c1x; A.M[1][1] = c1y; A.M[1][2] = c1z;
	A.M[2][0] = c2x; A.M[2][1] = c2y; A.M[2][2] = c2z;
	return A;
}


template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::RowMake(const T *ptr)
{
	mat<4, T> A;
	LOOPNN_(4) A.cr(icol, irow) = ptr[irow * 4 + icol];
	return A;
}
template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::RowMake(const vec<4, T> &row0, const vec<4, T> &row1, const vec<4, T> &row2, const vec<4, T> row3)
{
	mat<4, T> A;
	setRow(A, 0, row0);
	setRow(A, 1, row1);
	setRow(A, 2, row2);
	setRow(A, 3, row3);
	return A;
}
template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::RowMake(const T &r0x, const T &r0y, const T &r0z, const T &r0w,
								const T &r1x, const T &r1y, const T &r1z, const T &r1w,
								const T &r2x, const T &r2y, const T &r2z, const T &r2w,
								const T &r3x, const T &r3y, const T &r3z, const T &r3w)
{
	mat<4, T> A;
	A.M[0][0] = r0x; A.M[0][1] = r1x; A.M[0][2] = r2x; A.M[0][3] = r3x;
	A.M[1][0] = r0y; A.M[1][1] = r1y; A.M[1][2] = r2y; A.M[1][3] = r3y;
	A.M[2][0] = r0z; A.M[2][1] = r1z; A.M[2][2] = r2z; A.M[2][3] = r3z;
	A.M[3][0] = r0w; A.M[3][1] = r1w; A.M[3][2] = r2w; A.M[3][3] = r3w;
	return A;
}
template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::ColMake(const T *ptr)
{
	mat<4, T> A;
	memcpy(A.M, ptr, sizeof(T) * 16);
	return A;
}
template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::ColMake(const vec<4, T> &col0, const vec<4, T> &col1, const vec<4, T> &col2, const vec<4, T> col3)
{
	mat<4, T> A;
	setCol(A, 0, col0);
	setCol(A, 1, col1);
	setCol(A, 2, col2);
	setCol(A, 3, col3);
	return A;
}
template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::ColMake(const T &c0x, const T &c0y, const T &c0z, const T &c0w,
								const T &c1x, const T &c1y, const T &c1z, const T &c1w,
								const T &c2x, const T &c2y, const T &c2z, const T &c2w,
								const T &c3x, const T &c3y, const T &c3z, const T &c3w)
{
	mat<4, T> A;
	A.M[0][0] = c0x; A.M[0][1] = c0y; A.M[0][2] = c0z; A.M[0][3] = c0w;
	A.M[1][0] = c1x; A.M[1][1] = c1y; A.M[1][2] = c1z; A.M[1][3] = c1w;
	A.M[2][0] = c2x; A.M[2][1] = c2y; A.M[2][2] = c2z; A.M[2][3] = c2w;
	A.M[3][0] = c3x; A.M[3][1] = c3y; A.M[3][2] = c3z; A.M[3][3] = c3w;
	return A;
}




/* Comparison */
template<int N, typename T>
FORCE_INLINE bool operator==(const mat<N, T> &A, const mat<N, T> &B)
{
	LOOPN2 if (A[idx] != B[idx]) return false; // TODO: Not SIMD friendly
	return true;
}

template<int N, typename T>
FORCE_INLINE bool operator!=(const mat<N, T> &A, const mat<N, T> &B)
{
	return !(A == B); // TODO: Not SIMD friendly
}

template<int N, typename T>
FORCE_INLINE bool isIdentity(const mat<N, T> &A)
{
	mat<N, T> I = mat<N, T>::Identity();
	return A == I; // TODO: Not SIMD friendly
}

template<int N, typename T>
FORCE_INLINE T innerProd(const mat<N, T> &A, const mat<N, T> &B, const u32 Arow, const u32 Bcol)
{
	return dot(getRow(A, Arow), getCol(B, Bcol));
}


template<int N, typename T>
FORCE_INLINE vec<N, T> operator*(const vec<N, T> &A, const mat<N, T> &B)
{
	vec<N, T> out;
	LOOPN out[idx] = dot(A, getCol(A, idx));
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> operator*(const mat<N, T> &A, const vec<N, T> &B)
{
	vec<N, T> out;
	LOOPN out[idx] = dot(getRow(A, idx), B);
	return out;
}

template<int N, typename T>
FORCE_INLINE mat<N, T> operator*(const mat<N, T> &A, const mat<N, T> &B)
{
	mat<N, T> out;
	LOOPNN out(irow, icol) = innerProd(A, B, irow, icol);
	return out;
}

template<int N, typename T>
FORCE_INLINE vec<N, T> &operator*=(vec<N, T> &A, const mat<N, T> &B)
{
	return A = A * B;
}

template<int N, typename T>
FORCE_INLINE mat<N, T> &operator*=(mat<N, T> &A, const mat<N, T> &B)
{
	return A = A * B;
}


template<int N, typename T> 
FORCE_INLINE mat<N, T> operator+(const mat<N, T> &A)
{
	return A;
}

template<int N, typename T> 
FORCE_INLINE mat<N, T> operator-(const mat<N, T> &A)
{
	mat<N, T> out;
	LOOPN2 out[idx] = -A[idx];
	return out;
}




/* Misc matrix operations */

template<int N, typename T>
FORCE_INLINE vec<N, T> getRow(const mat<N, T> &A, u32 row)
{
	vec<N, T> out;
	LOOPN out[idx] = A.M[idx][row];
	return out;
}

template<int N, typename T>
FORCE_INLINE void setRow(mat<N, T> &A, u32 row, const vec<N, T> &v)
{
	LOOPN A.M[idx][row] = v[idx];
}

template<int N, typename T>
FORCE_INLINE vec<N, T> getCol(const mat<N, T> &A, u32 col)
{
	return *((vec<N, T> *) &A(0, col));
}

template<int N, typename T>
FORCE_INLINE void setCol(mat<N, T> &A, u32 col, const vec<N, T> &v)
{
	LOOPN A.M[col][idx] = v[idx];
}

template<int N, typename T>
FORCE_INLINE vec<N, T> getDiag(const mat<N, T> &A)
{
	vec<N, T> out;
	LOOPN out[idx] = A.M[idx][idx];
	return out;
}





template<int N, typename T>
vec<N, T> up(const mat<N, T> &A)
{
	return getCol(A, 1);
}

template<int N, typename T>
vec<N, T> down(const mat<N, T> &A)
{
	return -up(A);
}

template<int N, typename T>
vec<N, T> left(const mat<N, T> &A)
{
	return -right(A);
}

template<int N, typename T>
vec<N, T> right(const mat<N, T> &A)
{
	return getCol(A, 0);
}

template<int N, typename T>
vec<N, T> front(const mat<N, T> &A)
{
	return -back(A);
}

template<int N, typename T>
vec<N, T> back(const mat<N, T> &A)
{
	return getCol(A, 2);
}



template<int N, typename T>
std::ostream &operator<<(std::ostream &os, mat<N, T> v)
{
	for (int icol = 0; icol < N; icol++) {
		vec<N, T> c = getCol(v, icol);
		os << c;
	}
	return os;
}

template<int N, typename T>
std::istream &operator>>(std::istream &is, mat<N, T> &v)
{
	for (int icol = 0; icol < N; icol++) {
		vec<N, T> c;
		is >> c;
		setCol(v, icol, c);
	}
	return is;
}


template<int N, typename T>
FORCE_INLINE T trace(const mat<N, T> &A)
{
	T out = 0;
	LOOPN out += A.M[idx][idx];
	return out;
}

template<int N, typename T>
FORCE_INLINE mat<N, T> transpose(const mat<N, T> &A)
{
	mat<N, T> out = A;
	for (i32 irow = 0; irow < N - 1; irow++)
	for (i32 icol = irow + 1; icol < N; icol++) {
		sswap(out.M[irow][icol], out.M[icol][irow]);
	}
	return out;
}

template<int N, typename T>
FORCE_INLINE mat<N, T> elemMul(const mat<N, T> &A, const mat<N, T> &B)
{
	mat<N, T> out;
	LOOPN2 out[idx] = A[idx] * B[idx];
	return out;
}

template<int N, typename T>
FORCE_INLINE mat<N, T> elemDiv(const mat<N, T> &A, const mat<N, T> &B)
{
	mat<N, T> out;
	LOOPN2 out[idx] = A[idx] / B[idx];
	return out;
}





template<typename T>
FORCE_INLINE T det(mat<2, T> &A)
{
	return fmsub(A.M[0][0], A.M[1][1], A.M[0][1] * A.M[1][0]);
}

template<typename T>
FORCE_INLINE T det(mat<3, T> &A)
{
	T a = fmsub(A.M[2][2], A.M[1][1], A.M[2][1] * A.M[1][2]);
	T b = fmsub(A.M[2][2], A.M[0][1], A.M[2][1] * A.M[0][2]);
	T c = fmsub(A.M[1][2], A.M[0][1], A.M[1][1] * A.M[0][2]);
	return fmsub(A.M[0][0], a, fmsub(A.M[1][0], b, A.M[2][0] * c));
//	return A.M[0][0] * (A.M[2][2] * A.M[1][1] - A.M[2][1] * A.M[1][2]) -
//		   A.M[1][0] * (A.M[2][2] * A.M[0][1] - A.M[2][1] * A.M[0][2]) +
//		   A.M[2][0] * (A.M[1][2] * A.M[0][1] - A.M[1][1] * A.M[0][2]); // Fuse
}

template<typename T>
FORCE_INLINE T det(mat<4, T> &A)
{
	#define DET2(n0, n1, n2, n3)		(fmsub((n0), (n3), (n1) * (n2)))

	T s0, s1, s2, s3, s4, s5;
	T c0, c1, c2, c3, c4, c5;

	s0 = DET2(A.M[0][0], A.M[0][1], A.M[1][0], A.M[1][1]);
	s1 = DET2(A.M[0][0], A.M[0][2], A.M[1][0], A.M[1][2]);
	s2 = DET2(A.M[0][0], A.M[0][3], A.M[1][0], A.M[1][3]);
	s3 = DET2(A.M[0][1], A.M[0][2], A.M[1][1], A.M[1][2]);
	s4 = DET2(A.M[0][1], A.M[0][3], A.M[1][1], A.M[1][3]);
	s5 = DET2(A.M[0][2], A.M[0][3], A.M[1][2], A.M[1][3]);

	c5 = DET2(A.M[2][2], A.M[2][3], A.M[3][2], A.M[3][3]);
	c4 = DET2(A.M[2][1], A.M[2][3], A.M[3][1], A.M[3][3]);
	c3 = DET2(A.M[2][1], A.M[2][2], A.M[3][1], A.M[3][2]);
	c2 = DET2(A.M[2][0], A.M[2][3], A.M[3][0], A.M[3][3]);
	c1 = DET2(A.M[2][0], A.M[2][2], A.M[3][0], A.M[3][2]);
	c0 = DET2(A.M[2][0], A.M[2][1], A.M[3][0], A.M[3][1]);

	return fmsub(s0, c5, fmsub(s1, c4, fmadd(s2, c3, fmsub(s3, c2, fmsub(s4, c1, s5 * c0)))));
	//return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;

	#undef DET2
}

template<typename T>
FORCE_INLINE mat<2, T> inv(mat<2, T> &A)
{
	mat<2, T> out;
	T d = det(A);
	T denom = rcp(d);
	out.M[0][0] =  A.M[1][1] * denom;
	out.M[0][1] = -A.M[0][1] * denom;
	out.M[1][0] = -A.M[1][0] * denom;
	out.M[1][1] =  A.M[0][0] * denom;
	return out;
}

template<typename T>
FORCE_INLINE mat<3, T> inv(mat<3, T> &A)
{
	mat<3, T> out;

	T ie = A.M[2][2] * A.M[1][1];
	T ib = A.M[2][2] * A.M[0][1];
	T fb = A.M[1][2] * A.M[0][1];
	T hf = A.M[2][1] * A.M[1][2];
	T hc = A.M[2][1] * A.M[0][2];
	T ec = A.M[1][1] * A.M[0][2];

	T d = A.M[0][0] * (ie - hf) - A.M[1][0] * (ib - hc) + A.M[2][0] * (fb - ec);

	out.M[0][0] = (ie - hf);
	out.M[0][1] = (hc - ib);
	out.M[0][2] = (fb - ec);
	out.M[1][0] = (A.M[1][2] * A.M[2][0] - A.M[1][0] * A.M[2][2]);
	out.M[1][1] = (A.M[0][0] * A.M[2][2] - A.M[0][2] * A.M[2][0]);
	out.M[1][2] = (A.M[1][0] * A.M[0][2] - A.M[0][0] * A.M[1][2]);
	out.M[2][0] = (A.M[1][0] * A.M[2][1] - A.M[1][1] * A.M[2][0]);
	out.M[2][1] = (A.M[0][1] * A.M[2][0] - A.M[0][0] * A.M[2][1]);
	out.M[2][2] = (A.M[0][0] * A.M[1][1] - A.M[0][1] * A.M[1][0]);

	if (PreciseMath<T>()) {
		LOOPN2_(3) out[idx] /= d;
	} else {
		T denom = rcp(d);
		LOOPN2_(3) out[idx] *= denom;
	}

	return out;
}

template<typename T>
FORCE_INLINE mat<4, T> inv(mat<4, T> &A)
{
	mat<4, T> out;

	#define DET2(n0, n1, n2, n3)		(fmsub((n0), (n3), (n1) * (n2)))

	T s0, s1, s2, s3, s4, s5;
	T c0, c1, c2, c3, c4, c5;

	s0 = DET2(A.M[0][0], A.M[0][1], A.M[1][0], A.M[1][1]);
	s1 = DET2(A.M[0][0], A.M[0][2], A.M[1][0], A.M[1][2]);
	s2 = DET2(A.M[0][0], A.M[0][3], A.M[1][0], A.M[1][3]);
	s3 = DET2(A.M[0][1], A.M[0][2], A.M[1][1], A.M[1][2]);
	s4 = DET2(A.M[0][1], A.M[0][3], A.M[1][1], A.M[1][3]);
	s5 = DET2(A.M[0][2], A.M[0][3], A.M[1][2], A.M[1][3]);

	c5 = DET2(A.M[2][2], A.M[2][3], A.M[3][2], A.M[3][3]);
	c4 = DET2(A.M[2][1], A.M[2][3], A.M[3][1], A.M[3][3]);
	c3 = DET2(A.M[2][1], A.M[2][2], A.M[3][1], A.M[3][2]);
	c2 = DET2(A.M[2][0], A.M[2][3], A.M[3][0], A.M[3][3]);
	c1 = DET2(A.M[2][0], A.M[2][2], A.M[3][0], A.M[3][2]);
	c0 = DET2(A.M[2][0], A.M[2][1], A.M[3][0], A.M[3][1]);

	//T d = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
	T d = fmsub(s0, c5, fmsub(s1, c4, fmadd(s2, c3, fmsub(s3, c2, fmsub(s4, c1, s5 * c0)))));

	#undef DET2

	out.M[0][0] = ( A.M[1][1] * c5 - A.M[1][2] * c4 + A.M[1][3] * c3);
	out.M[0][1] = (-A.M[0][1] * c5 + A.M[0][2] * c4 - A.M[0][3] * c3);
	out.M[0][2] = ( A.M[3][1] * s5 - A.M[3][2] * s4 + A.M[3][3] * s3);
	out.M[0][3] = (-A.M[2][1] * s5 + A.M[2][2] * s4 - A.M[2][3] * s3);
	out.M[1][0] = (-A.M[1][0] * c5 + A.M[1][2] * c2 - A.M[1][3] * c1);
	out.M[1][1] = ( A.M[0][0] * c5 - A.M[0][2] * c2 + A.M[0][3] * c1);
	out.M[1][2] = (-A.M[3][0] * s5 + A.M[3][2] * s2 - A.M[3][3] * s1);
	out.M[1][3] = ( A.M[2][0] * s5 - A.M[2][2] * s2 + A.M[2][3] * s1);
	out.M[2][0] = ( A.M[1][0] * c4 - A.M[1][1] * c2 + A.M[1][3] * c0);
	out.M[2][1] = (-A.M[0][0] * c4 + A.M[0][1] * c2 - A.M[0][3] * c0);
	out.M[2][2] = ( A.M[3][0] * s4 - A.M[3][1] * s2 + A.M[3][3] * s0);
	out.M[2][3] = (-A.M[2][0] * s4 + A.M[2][1] * s2 - A.M[2][3] * s0);
	out.M[3][0] = (-A.M[1][0] * c3 + A.M[1][1] * c1 - A.M[1][2] * c0);
	out.M[3][1] = ( A.M[0][0] * c3 - A.M[0][1] * c1 + A.M[0][2] * c0);
	out.M[3][2] = (-A.M[3][0] * s3 + A.M[3][1] * s1 - A.M[3][2] * s0);
	out.M[3][3] = ( A.M[2][0] * s3 - A.M[2][1] * s1 + A.M[2][2] * s0);

	if (PreciseMath<T>()) {
		LOOPN2_(4) out[idx] /= d;
	} else {
		T denom = rcp(d);
		LOOPN2_(4) out[idx] *= denom;
	}

	return out;
}

template<int N, typename T>
vec<N, T> solveSystem(const mat<N, T> &A, const vec<N, T> &eq)
{
	vec<N, T> res;
	mat<3, T> m;
	T d = det(A);
	for (int i = 0; i < N; i++) {
		m = A;
		m.setCol(i, eq);
		res[i] = det(m) / d;
	}
	return res;
}

template<typename T>
FORCE_INLINE vec<2, T> eigenvalues(const mat<2, T> &A)
{
	vec<2, T> ret;
	T t = trace(A);
	T u = sqrt(t * t - 4 * det(A));
	ret.x = t - u;
	ret.y = t + u;
	return ret;
}

template<typename T>
FORCE_INLINE mat<2, T> eigenvectors(const mat<2, T> &A)
{
	vec<2, T> eig = eigenvalues(A);
	return eigenvalues(A, eig);
}

// TODO: Eigen 3 and 4. Power iteration

template<typename T>
FORCE_INLINE mat<2, T> eigenvectors(const mat<2, T> &A, const vec<2, T> &eig)
{
	if (A.M[1][0] != 0)
		return mat<2, T>(A.M[1][0], A.M[1][0], eig.x - A.M[0][0], eig.y - A.M[0][0]); // TODO: Not SIMD friendly
	if (A.M[0][1] != 0)
		return mat<2, T>(eig.x - A.M[1][1], eig.y - A.M[1][1], A.M[0][1], A.M[0][1]);
	return mat<2, T>::Identity();
}


template<typename T>
FORCE_INLINE mat<4, T> affineMul(const mat<4, T> &A, const mat<4, T> &B)
{
	#define APROD0(r, c)	fmadd(A.M[0][r], B.M[c][0], fmadd(A.M[1][r], B.M[c][1], A.M[2][r] * B.M[c][2]))
	#define APROD1(r, c)	fmadd(A.M[0][r], B.M[c][0], fmadd(A.M[1][r], B.M[c][1], fmadd(A.M[2][r], B.M[c][2], A.M[3][r])))

	return mat<4, T>::ColMake(
		APROD0(0, 0), APROD0(0, 1), APROD0(0, 2), APROD1(0, 3),
		APROD0(1, 0), APROD0(1, 1), APROD0(1, 2), APROD1(1, 3),
		APROD0(2, 0), APROD0(2, 1), APROD0(2, 2), APROD1(2, 3),
		0, 0, 0, 1
	);

	#undef APROD0
	#undef APROD1
}

template<typename T>
FORCE_INLINE mat<4, T> affineMul(const mat<4, T> &A, const vec<4, T> &B)
{
	return vec<4, T>(
		fmadd(A.M[0][0], B.x, fmadd(A.M[1][0], B.y, fmadd(A.M[2][0], B.z, A.M[3][0] * B.w))),
		fmadd(A.M[0][1], B.x, fmadd(A.M[1][1], B.y, fmadd(A.M[2][1], B.z, A.M[3][1] * B.w))),
		fmadd(A.M[0][2], B.x, fmadd(A.M[1][2], B.y, fmadd(A.M[2][2], B.z, A.M[3][2] * B.w))),
		B.w
	);
}

template<typename T>
FORCE_INLINE vec<3, T> affineMul0(const mat<4, T> &A, const vec<3, T> &B)
{
	return vec<3, T>(
		fmadd(A.M[0][0], B.x, fmadd(A.M[1][0], B.y, A.M[2][0] * B.z)),
		fmadd(A.M[0][1], B.x, fmadd(A.M[1][1], B.y, A.M[2][1] * B.z)),
		fmadd(A.M[0][2], B.x, fmadd(A.M[1][2], B.y, A.M[2][2] * B.z))
	);
}

template<typename T>
FORCE_INLINE vec<3, T> affineMul1(const mat<4, T> &A, const vec<3, T> &B)
{
	return vec<3, T>(
		fmadd(A.M[0][0], B.x, fmadd(A.M[0][1], B.y, fmadd(A.M[0][2], B.z, A.M[0][3]))),
		fmadd(A.M[1][0], B.x, fmadd(A.M[1][1], B.y, fmadd(A.M[1][2], B.z, A.M[1][3]))),
		fmadd(A.M[2][0], B.x, fmadd(A.M[2][1], B.y, fmadd(A.M[2][2], B.z, A.M[2][3])))
	);
}

template<typename T>
FORCE_INLINE mat<3, T> mat<2, T>::mat3()
{
	return ColMake(rc(0, 0), rc(1, 0), 0,
				   rc(0, 1), rc(1, 1), 0,
				   0, 0, 1);
}

template<typename T>
FORCE_INLINE mat<4, T> mat<2, T>::mat4()
{
	return ColMake(rc(0, 0), rc(1, 0), 0, 0,
				   rc(0, 1), rc(1, 1), 0, 0, 
				   0, 0, 1, 0,
				   0, 0, 0, 1);
}

template<typename T>
FORCE_INLINE mat<4, T> mat<3, T>::mat4()
{
	return ColMake(rc(0, 0), rc(1, 0), rc(2, 0), 0,
				   rc(0, 1), rc(1, 1), rc(2, 1), 0, 
				   rc(0, 2), rc(1, 2), rc(2, 2), 0,
				   0, 0, 0, 1);
}

template<typename T>
FORCE_INLINE mat<2, T> mat<2, T>::RotationMatrix(const T &rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	return mat<2, T>::RowMake(cs, -sn, sn, cs);
}

template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::RotationMatrix(const T &rad, const vec<3, T> &v)
{
	T C = cos(rad);
	T S = sin(rad);
	T nC = 1 - C;

	T xx = v.x * v.x;
	T xy = v.x * v.y;
	T xz = v.x * v.z;
	T yy = v.y * v.y;
	T yz = v.y * v.z;
	T zz = v.z * v.z;

	T nCxy = nC * xy;
	T nCxz = nC * xz;
	T nCyz = nC * yz;

	vec<3, T> vS = v * S;

	return mat<3, T>::ColMake(
		fmadd(xx, nC, C),	nCxy + vS.z,		nCxz - vS.y,
		nCxy - vS.z,		fmadd(yy, nC, C),	nCyz + vS.x,
		nCxz + vS.y,		nCyz - vS.x,		fmadd(zz, nC, C));
}

template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::RotationXMatrix(const T &rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	return mat<3, T>::RowMake(
		1, 0, 0,
		0, cs, -sn,
		0, sn, cs);
}

template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::RotationYMatrix(const T &rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	return mat<3, T>::RowMake(
		cs, 0, sn,
		0, 1, 0,
		-sn, 0, cs);
}

template<typename T>
FORCE_INLINE mat<3, T> mat<3, T>::RotationZMatrix(const T &rad)
{
	T cs = cos(rad);
	T sn = sin(rad);
	return mat<3, T>::RowMake(
		cs, -sn, 0,
		sn, cs, 0,
		0, 0, 1);
}

template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::NdcToScreen(int resX, int resY)
{
	mat<4, T> pixMtx = mat<4, T>::Identity();
	pixMtx.rc(0, 0) = 2 / splat<T>(resX);
	pixMtx.rc(1, 1) = 2 / splat<T>(resY);
	pixMtx.rc(0, 3) = -1;
	pixMtx.rc(1, 3) = -1;
	return pixMtx;
}

// TODO: Test projection matrices

template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::PerspectiveProjection(T fovy, T aspect, T nnear, T nfar)
{
	mat<4, T> projection = mat<4, T>::Identity();

	T f = rcp(tan(fovy / 2));
	projection.rc(0, 0) = f / aspect;
	projection.rc(1, 1) = f;
	projection.rc(3, 2) = -1;
	projection.rc(3, 3) = 0;

	if (nnear == nfar) {
		projection.rc(2, 2) = -1;
		projection.rc(2, 3) = -2 * nnear;
	} else {
		projection.rc(2, 2) = (nfar + nnear) / (nnear - nfar);
		projection.rc(2, 3) = (2 * nfar * nnear) / (nnear - nfar);
	}

	return projection;
}


template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::PerspectiveProjectionInverse(T fovy, T aspect, T nnear, T nfar)
{
	mat<4, T> projectionInv = mat<4, T>::Identity();

	T f = tan(fovy / 2);
	projectionInv.rc(0, 0) = aspect * f;
	projectionInv.rc(1, 1) = f;
	projectionInv.rc(2, 2) = 0;
	projectionInv.rc(2, 3) = -1;

	if (nnear == nfar) {
		projectionInv.rc(3, 3) = rcp(2 * nnear);
		projectionInv.rc(3, 2) = -projectionInv.M[3][3];
	} else {
		projectionInv.rc(3, 3) = (nnear + nfar) / (2 * nnear * nfar);
		projectionInv.rc(3, 2) = (nnear - nfar) / (2 * nnear * nfar);
	}

	return projectionInv;
}

/*

{{F / a, 0, 0, 0}, {0, F, 0, 0}, {0, 0, 1, 2 * n}, {0, 0, -1, 0}}
{{a/F, 0, 0, 0}, {0, F^(-1), 0, 0}, {0, 0, 0, -1}, {0, 0, 1/(2 n), 1/(2 n)}}

{{F / a, 0, 0, 0}, {0, F, 0, 0}, {0, 0, n / (n - f), (n * f) / (n - f)}, {0, 0, -1, 0}}
{{a/F, 0, 0, 0}, {0, F^(-1), 0, 0}, {0, 0, 0, -1}, {0, 0, f^(-1) - n^(-1), f^(-1)}}


*/

template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::PerspectiveProjectionReversed(T fovy, T aspect, T nnear, T nfar)
{
	mat<4, T> projection = mat<4, T>::Identity();

	T f = rcp(tan(fovy / 2));
	projection.rc(0, 0) = f / aspect;
	projection.rc(1, 1) = f;
	projection.rc(3, 2) = -1;
	projection.rc(3, 3) = 0;

	if (nnear == nfar) {
		projection.rc(2, 2) = 1;
		projection.rc(2, 3) = 2 * nnear;
	} else {
		projection.rc(2, 2) = nnear / (nnear - nfar);
		projection.rc(2, 3) = (nfar * nnear) / (nnear - nfar);
	}

	return projection;
}

template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::PerspectiveProjectionReversedInverse(T fovy, T aspect, T nnear, T nfar)
{
	mat<4, T> projectionInv = mat<4, T>::Identity();

	T f = tan(fovy / 2);
	projectionInv.rc(0, 0) = aspect * f;
	projectionInv.rc(1, 1) = f;
	projectionInv.rc(2, 2) = 0;
	projectionInv.rc(2, 3) = -1;

	if (nnear == nfar) {
		projectionInv.rc(3, 3) = 1 / (2 * nnear);
		projectionInv.rc(3, 2) = projectionInv.rc(3, 3);
	} else {
		projectionInv.rc(3, 3) = 1 / nfar;
		projectionInv.rc(3, 2) = 1 / nfar - 1 / nnear;
	}

	return projectionInv;
}


template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::OrthogonalProjection(T left, T right, T bottom, T top, T nnear, T nfar)
{
	mat<4, T> projection = mat<4, T>::Identity();
	projection.rc(0, 0) =   2 / (right - left);
	projection.rc(1, 1) =   2 / (top - bottom);
	projection.rc(2, 2) = - 2 / (nfar - nnear);
	projection.rc(0, 3) = - (right + left) / (right - left);
	projection.rc(1, 3) = - (top + bottom) / (top - bottom);
	projection.rc(2, 3) = - (nfar + nnear) / (nfar - nnear);
	return projection;
}

// TODO: Incorrect?
template<typename T>
FORCE_INLINE mat<4, T> mat<4, T>::OrthogonalProjectionInverse(T left, T right, T bottom, T top, T nnear, T nfar)
{
	mat<4, T> projectionInv = mat<4, T>::Identity();
	projectionInv.r(0, 0) = (right - left) / 2;
	projectionInv.r(1, 1) = (top - bottom) / 2;
	projectionInv.r(2, 2) = (nnear - nfar) / 2;
	projectionInv.r(2, 3) = (nfar + nnear) / 2; // TODO: sign?
	projectionInv.r(3, 3) = 1;
	return projectionInv;
}


template<typename T>
FORCE_INLINE mat<3, T> ScaleMatrix(T &A)
{
	return mat<3, T>::ColMake(	A, 0, 0,
								0, A, 0,
								0, 0, A);
}

template<typename T>
FORCE_INLINE mat<3, T> ScaleMatrix(const vec<3, T> &A)
{
	return mat<3, T>::ColMake(	A.x, 0, 0,
								0, A.y, 0,
								0, 0, A.z);
}

template<typename T>
FORCE_INLINE mat<4, T> TranslationMatrix(const vec<3, T> &A)
{
	return mat<4, T>::ColMake(	1, 0, 0, A.x,
								0, 1, 0, A.y,
								0, 0, 1, A.z,
								0, 0, 0, 1);
}

template<typename T>
FORCE_INLINE mat<3, T> mat<4, T>::R() const
{
	return mat<3, T>::ColMake(	rc(0, 0), rc(1, 0), rc(2, 0),
								rc(0, 1), rc(1, 1), rc(2, 1),
								rc(0, 2), rc(1, 2), rc(2, 2));
}

