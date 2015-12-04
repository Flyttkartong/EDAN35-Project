#pragma once

#include "TRSTransform.h"
#include "Geometry.h"
#include "InputHandler.h"

template<typename T>
class FPSCamera
{
public:
	FPSCamera(T fovy, T aspect, T nnear, T nfar);
	~FPSCamera();

public:
	void Update(double dt, InputHandler &ih);
	void SetProjection(T fovy, T aspect, T nnear, T nfar);
	void SetFov(T fovy);
	T GetFov();
	void SetAspect(T a);
	T GetAspect();

	mat<4, T> GetViewToWorldMatrix();
	mat<4, T> GetWorldToViewMatrix();
	mat<4, T> GetClipToWorldMatrix();
	mat<4, T> GetWorldToClipMatrix();
	mat<4, T> GetClipToViewMatrix();
	mat<4, T> GetViewToClipMatrix();

	vec<3, T> GetClipToWorld(vec<3, T> xyw);
	vec<3, T> GetClipToView(vec<3, T> xyw);

	Ray<T> GetWorldRay(vec<2, T> ndc);

public:
	TRSTransform<T> mWorld;
	T mMovementSpeed;
	T mMouseSensitivity;

public:
	T mFov, mAspect, mNear, mFar;
	mat<4, T> mProjection;
	mat<4, T> mProjectionInverse;
	vec<2, T> mRotation;
	vec<2, T> mMousePosition;

public:
	friend std::ostream &operator<<(std::ostream &os, FPSCamera<T> &v) {
		os << v.mFov << " " << v.mAspect << " " << v.mNear << " " << v.mFar << std::endl;
		os << v.mMovementSpeed << " " << v.mMouseSensitivity << std::endl;
		os << v.mRotation << std::endl;
		os << v.mWorld;
		return os;
	}
	friend std::istream &operator>>(std::istream &is, FPSCamera<T> &v) {
		T fov, aspect, nnear, far;
		is >> fov >> aspect >> nnear >> far;
		v.SetProjection(fov, aspect, nnear, far);
		is >> v.mMovementSpeed >> v.mMouseSensitivity;
		is >> v.mRotation;
		is >> v.mWorld;
		return is;
	}
};

#include "FPSCamera.inl"

typedef FPSCamera<float> FPSCameraf;
typedef FPSCamera<double> FPSCamerad;
