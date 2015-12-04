#include <cmath>
#include "TRSTransform.h"

/*----------------------------------------------------------------------------*/

template<typename T>
TRSTransform<T>::TRSTransform()
{
	ResetTransform();
}

/*----------------------------------------------------------------------------*/

template<typename T>
TRSTransform<T>::~TRSTransform()
{
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::ResetTransform()
{
	mT = vec<3, T>::Make(0, 0, 0);
	mS = vec<3, T>::Make(1, 1, 1);
	mR = mat<3, T>::Identity();
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::Translate(vec<3, T> v)
{
	mT += v;
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::Scale(vec<3, T> v)
{
	mS = mS.elemMul(v);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::Scale(T uniform)
{
	mS *= uniform;
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::Rotate(T angle, vec<3, T> v)
{
	mR *= mat<3, T>::RotationMatrix(angle, v);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::RotateX(T angle)
{
	T C = cos(angle);
	T S = sin(angle);
	mR = mat<3, T>::RowMake(
		mR.rc(0, 0), mR.rc(0, 1), mR.rc(0, 2),
		C * mR.rc(1, 0) - mR.rc(2, 0) * S, C * mR.rc(1, 1) - mR.rc(2, 1) * S, C * mR.rc(1, 2) - mR.rc(2, 2) * S,
		C * mR.rc(2, 0) + mR.rc(1, 0) * S, C * mR.rc(2, 1) + mR.rc(1, 1) * S, C * mR.rc(2, 2) + mR.rc(1, 2) * S);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::RotateY(T angle)
{
	T C = cos(angle);
	T S = sin(angle);
	mR = mat<3, T>::RowMake(
		C * mR.rc(0, 0) + mR.rc(2, 0) * S, C * mR.rc(0, 1) + mR.rc(2, 1) * S, C * mR.rc(0, 2) + mR.rc(2, 2) * S,
		mR.rc(1, 0), mR.rc(1, 1), mR.rc(1, 2),
		C * mR.rc(2, 0) - mR.rc(0, 0) * S, C * mR.rc(2, 1) - mR.rc(0, 1) * S, C * mR.rc(2, 2) - mR.rc(0, 2) * S);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::RotateZ(T angle)
{
	T C = cos(angle);
	T S = sin(angle);
	mR = mat<3, T>::RowMake(
		C * mR.rc(0, 0) - mR.rc(1, 0) * S, C * mR.rc(0, 1) - mR.rc(1, 1) * S, C * mR.rc(0, 2) - mR.rc(1, 2) * S,
		C * mR.rc(1, 0) + mR.rc(0, 0) * S, C * mR.rc(1, 1) + mR.rc(0, 1) * S, C * mR.rc(1, 2) + mR.rc(0, 2) * S,
		mR.rc(2, 0), mR.rc(2, 1), mR.rc(2, 2));
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::PreRotate(T angle, vec<3, T> v)
{
	mR = mat<3, T>::RotationMatrix(angle, v) * mR;
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::PreRotateX(T angle)
{
	T C = cos(angle);
	T S = sin(angle);
	mR = mat<3, T>::RowMake(
		mR.rc(0, 0), C * mR.rc(0, 1) + mR.rc(0, 2) * S, C * mR.rc(0, 2) - mR.rc(0, 1) * S,
		mR.rc(1, 0), C * mR.rc(1, 1) + mR.rc(1, 2) * S, C * mR.rc(1, 2) - mR.rc(1, 1) * S,
		mR.rc(2, 0), C * mR.rc(2, 1) + mR.rc(2, 2) * S, C * mR.rc(2, 2) - mR.rc(2, 1) * S);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::PreRotateY(T angle)
{
	T C = cos(angle);
	T S = sin(angle);
	mR = mat<3, T>::RowMake(
		C * mR.rc(0, 0) - mR.rc(0, 2) * S, mR.rc(0, 1), C * mR.rc(0, 2) + mR.rc(0, 0) * S,
		C * mR.rc(1, 0) - mR.rc(1, 2) * S, mR.rc(1, 1), C * mR.rc(1, 2) + mR.rc(1, 0) * S,
		C * mR.rc(2, 0) - mR.rc(2, 2) * S, mR.rc(2, 1), C * mR.rc(2, 2) + mR.rc(2, 0) * S);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::PreRotateZ(T angle)
{
	T C = cos(angle);
	T S = sin(angle);
	mR = mat<3, T>::RowMake(
		C * mR.rc(0, 0) + mR.rc(0, 1) * S, C * mR.rc(0, 1) - mR.rc(0, 0) * S, mR.rc(0, 2),
		C * mR.rc(1, 0) + mR.rc(1, 1) * S, C * mR.rc(1, 1) - mR.rc(1, 0) * S, mR.rc(1, 2),
		C * mR.rc(2, 0) + mR.rc(2, 1) * S, C * mR.rc(2, 1) - mR.rc(2, 0) * S, mR.rc(2, 2));
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetTranslate(vec<3, T> v)
{
	mT = v;
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetScale(vec<3, T> v)
{
	mS = v;
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetScale(T uniform)
{
	mS = vec<3, T>(uniform);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetRotate(T angle, vec<3, T> v)
{
    mR = mat<3, T>::RowMake(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	Rotate(angle, v);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetRotateX(T angle)
{
	mR = mat<3, T>::RotationXMatrix(angle);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetRotateY(T angle)
{
	mR = mat<3, T>::RotationYMatrix(angle);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::SetRotateZ(T angle)
{
	mR = mat<3, T>::RotationZMatrix(angle);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::LookTowards(vec<3, T> front_vec, vec<3, T> up_vec)
{
	front_vec = normalize(front_vec);
	up_vec = normalize(up_vec);

	if (abs(dot(up_vec, front_vec)) > 0.99999f)
		return;

	vec<3, T> prev_up = up_vec;

	vec<3, T> right = cross(front_vec, prev_up);
	vec<3, T> up = cross(right, front_vec);

	right = normalize(right);
	up = normalize(up);

	setCol(mR, 0, right);
	setCol(mR, 1, up);
	setCol(mR, 2, -front_vec);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::LookTowards(vec<3, T> front_vec)
{
	LookTowards(front_vec, vec<3, T>::Make(0, 1, 0));
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::LookAt(vec<3, T> point, vec<3, T> up_vec)
{
	LookTowards(point - mT, up_vec);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void TRSTransform<T>::LookAt(vec<3, T> point)
{
	LookTowards(point - mT);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetTranslationMatrix() const
{
	return mat<4, T>::RowMake(1, 0, 0, mT.x,
					 0, 1, 0, mT.y,
					 0, 0, 1, mT.z,
					 0, 0, 0, 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetRotationMatrix() const
{
	return mat<4, T>::RowMake(	mR.rc(0, 0), mR.rc(0, 1), mR.rc(0, 2), 0,
								mR.rc(1, 0), mR.rc(1, 1), mR.rc(1, 2), 0,
								mR.rc(2, 0), mR.rc(2, 1), mR.rc(2, 2), 0,
								0  , 0  , 0  , 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetScaleMatrix() const
{
	return mat<4, T>::RowMake(mS.x, 0  , 0  , 0,
					 0  , mS.y, 0  , 0,
					 0  , 0  , mS.z, 0,
					 0  , 0  , 0  , 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetTranslationMatrixInverse() const
{
	return mat<4, T>::RowMake(1, 0, 0, -mT.x,
					 0, 1, 0, -mT.y,
					 0, 0, 1, -mT.z,
					 0, 0, 0, 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetRotationMatrixInverse() const
{
	return mat<4, T>::RowMake(	mR.rc(0, 0), mR.rc(1, 0), mR.rc(2, 0), 0,
								mR.rc(0, 1), mR.rc(1, 1), mR.rc(2, 1), 0,
								mR.rc(0, 2), mR.rc(1, 2), mR.rc(2, 2), 0,
								0  , 0  , 0  , 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetScaleMatrixInverse() const
{
	return mat<4, T>::RowMake(T(1)/mS.x, 0  , 0        , 0,
				 0       , T(1)/mS.y, 0       , 0,
				 0       , 0       , T(1)/mS.z, 0,
				 0       , 0       , 0        , 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetTranslationRotationMatrix() const
{
	return mat<4, T>::RowMake(	mR.rc(0, 0), mR.rc(0, 1), mR.rc(0, 2), mT.x,
								mR.rc(1, 0), mR.rc(1, 1), mR.rc(1, 2), mT.y,
								mR.rc(2, 0), mR.rc(2, 1), mR.rc(2, 2), mT.z,
								0	, 0	 , 0  , 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetMatrix() const
{
	return mat<4, T>::RowMake(	mR.rc(0, 0)*mS.x, mR.rc(0, 1)*mS.y, mR.rc(0, 2)*mS.z, mT.x,
								mR.rc(1, 0)*mS.x, mR.rc(1, 1)*mS.y, mR.rc(1, 2)*mS.z, mT.y,
								mR.rc(2, 0)*mS.x, mR.rc(2, 1)*mS.y, mR.rc(2, 2)*mS.z, mT.z,
								0		, 0		 , 0	  , 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<4, T> TRSTransform<T>::GetMatrixInverse() const
{
	vec<3, T> X = vec<3, T>::Make(T(1) / mS.x, T(1) / mS.y, T(1) / mS.z);

	T a = mR.rc(0, 0) * X.x;
	T b = mR.rc(0, 1) * X.y;
	T c = mR.rc(0, 2) * X.z;
	T d = mR.rc(1, 0) * X.x;
	T e = mR.rc(1, 1) * X.y;
	T f = mR.rc(1, 2) * X.z;
	T g = mR.rc(2, 0) * X.x;
	T h = mR.rc(2, 1) * X.y;
	T i = mR.rc(2, 2) * X.z;

	return mat<4, T>::RowMake(a, d, g, -(mT.x * a + mT.y * d + mT.z * g),
				 b, e, h, -(mT.x * b + mT.y * e + mT.z * h),
				 c, f, i, -(mT.x * c + mT.y * f + mT.z * i),
				 0			, 0			, 0			, 1);
}

/*----------------------------------------------------------------------------*/

template<typename T>
mat<3, T> TRSTransform<T>::GetRotation() const
{
	return mR;
}

/*----------------------------------------------------------------------------*/

template<typename T>
vec<3, T> TRSTransform<T>::GetTranslation() const
{
	return mT;
}

/*----------------------------------------------------------------------------*/

template<typename T>
vec<3, T> TRSTransform<T>::GetScale() const
{
	return mS;
}

/*----------------------------------------------------------------------------*/

template<typename T>
vec<3, T> TRSTransform<T>::GetUp() const
{
	return vec<3, T>::Make(mR.rc(0, 1)*mS.y, mR.rc(1, 1)*mS.y, mR.rc(2, 1)*mS.y);
}

template<typename T>
vec<3, T> TRSTransform<T>::GetDown() const
{
	return -GetUp();
}

template<typename T>
vec<3, T> TRSTransform<T>::GetLeft() const
{
	return -GetRight();
}

template<typename T>
vec<3, T> TRSTransform<T>::GetRight() const
{
	return vec<3, T>::Make(mR.rc(0, 0)*mS.x, mR.rc(1, 0)*mS.x, mR.rc(2, 0)*mS.x);
}

template<typename T>
vec<3, T> TRSTransform<T>::GetFront() const
{
	return -GetBack();
}

template<typename T>
vec<3, T> TRSTransform<T>::GetBack() const
{
	return vec<3, T>::Make(mR.rc(0, 2)*mS.z, mR.rc(1, 2)*mS.z, mR.rc(2, 2)*mS.z);
}

/*----------------------------------------------------------------------------*/
