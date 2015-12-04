template<typename T>
FPSCamera<T>::FPSCamera(T fovy, T aspect, T nnear, T nfar) : mRotation(vec<2, T>::Make(0.0f)), mMousePosition(vec<2, T>::Make(0.0f)), mMovementSpeed(1), mMouseSensitivity(1)
{
	SetProjection(fovy, aspect, nnear, nfar);
}

template<typename T>
FPSCamera<T>::~FPSCamera()
{
}

template<typename T>
void FPSCamera<T>::SetProjection(T fovy, T aspect, T nnear, T nfar)
{
	mFov = fovy;
	mAspect = aspect;
	mNear = nnear;
	mFar = nfar;
	mProjection = mat<4, T>::PerspectiveProjection(fovy, aspect, nnear, nfar);
	mProjectionInverse = mat<4, T>::PerspectiveProjectionInverse(fovy, aspect, nnear, nfar);
}

template<typename T>
void FPSCamera<T>::SetFov(T fovy)
{
	SetProjection(fovy, mAspect, mNear, mFar);
}

template<typename T>
T FPSCamera<T>::GetFov()
{
	return mFov;
}

template<typename T>
void FPSCamera<T>::SetAspect(T a)
{
	SetProjection(mFov, a, mNear, mFar);
}

template<typename T>
T FPSCamera<T>::GetAspect()
{
	return mAspect;
}


template<typename T>
void FPSCamera<T>::Update(double dt, InputHandler &ih)
{
	vec<2, T> newMousePosition = cast<T>(ih.GetMousePosition());
	vec<2, T> mouse_diff = newMousePosition - mMousePosition;
	mouse_diff.y = -mouse_diff.y;
	mMousePosition = newMousePosition;
	mouse_diff *= mMouseSensitivity;

	if (ih.GetMouseState(GLFW_MOUSE_BUTTON_LEFT) & PRESSED) {
		mRotation.x -= mouse_diff.x;
		mRotation.y += mouse_diff.y;
		mWorld.SetRotateX(mRotation.y);
		mWorld.RotateY(mRotation.x);
	}

	T movementModifier = ((ih.GetKeycodeState(GLFW_MOD_SHIFT) & PRESSED)) ? 0.25f : ((ih.GetKeycodeState(GLFW_MOD_CONTROL) & PRESSED)) ? 4.0f : 1.0f;
	T movement = movementModifier * T(dt) * mMovementSpeed;

	T move = 0.0f, strafe = 0.0f, levitate = 0.0f;
	if ((ih.GetKeycodeState(GLFW_KEY_W) & PRESSED)) move += movement;
	if ((ih.GetKeycodeState(GLFW_KEY_S) & PRESSED)) move -= movement;
	if ((ih.GetKeycodeState(GLFW_KEY_A) & PRESSED)) strafe -= movement;
	if ((ih.GetKeycodeState(GLFW_KEY_D) & PRESSED)) strafe += movement; 
	if ((ih.GetKeycodeState(GLFW_KEY_Q) & PRESSED)) levitate -= movement;
	if ((ih.GetKeycodeState(GLFW_KEY_E) & PRESSED)) levitate += movement;

	mWorld.Translate(mWorld.GetFront() * move);
	mWorld.Translate(mWorld.GetRight() * strafe);
	mWorld.Translate(mWorld.GetUp() * levitate);
}

template<typename T>
mat<4, T> FPSCamera<T>::GetViewToWorldMatrix()
{
	return mWorld.GetMatrix();
}

template<typename T>
mat<4, T> FPSCamera<T>::GetWorldToViewMatrix()
{
	return mWorld.GetMatrixInverse();
}

template<typename T>
mat<4, T> FPSCamera<T>::GetClipToWorldMatrix()
{
	return GetViewToWorldMatrix() * mProjectionInverse;
}

template<typename T>
mat<4, T> FPSCamera<T>::GetWorldToClipMatrix()
{
	return mProjection * GetWorldToViewMatrix();
}

template<typename T>
mat<4, T> FPSCamera<T>::GetClipToViewMatrix()
{
	return mProjectionInverse;
}

template<typename T>
mat<4, T> FPSCamera<T>::GetViewToClipMatrix()
{
	return mProjection;
}

template<typename T>
vec<3, T> FPSCamera<T>::GetClipToWorld(vec<3, T> xyw)
{
	vec<4, T> vv = GetClipToView(xyw).xyz1();
	vec<3, T> wv = mWorld.GetMatrix().affineMul(vv);
	return wv;
}

template<typename T>
vec<3, T> FPSCamera<T>::GetClipToView(vec<3, T> xyw)
{
	vec<3, T> vv;
	vv.x = mProjectionInverse.M[0][0] * xyw.x;
	vv.y = mProjectionInverse.M[1][1] * xyw.y;
	vv.z = -xyw.w;
	return vv;
}

template<typename T>
Ray<T> FPSCamera<T>::GetWorldRay(vec<2, T> ndc)
{
	mat<4, T> mtx = GetClipToWorldMatrix();
	vec<3, T> origin = divW(mtx * vec<4, T>(ndc, -1.0f, 1.0f)).xyz();
	vec<3, T> far = divW(mtx * vec<4, T>(ndc, 1.0f, 1.0f)).xyz();
	return Ray<T>(origin, far - origin);
}






