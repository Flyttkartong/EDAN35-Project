#include "Log.h"

/*----------------------------------------------------------------------------*/

template<typename T> void SimpleDraw::CopyArray(const T *p, unsigned int n, const vec4f *color)
{
	if (mVtxArray == nullptr) {
		mVtxArrayMem = 1024 * 1024 * 4;
		mVtxArray = (vec4f *) malloc(mVtxArrayMem);
	}
	if (int(n * 2 * sizeof(vec4f)) > mVtxArrayMem) {
		mVtxArrayMem += mVtxArrayMem >> 2;
		mVtxArray = (vec4f *) realloc(mVtxArray, mVtxArrayMem);
	}
	if (color != nullptr) {
		for (unsigned int i = 0; i < n; i++) {
			mVtxArray[i * 2 + 0] = mState.mMatrix * point4(p[i]);
			mVtxArray[i * 2 + 1] = color[i];
		}
	} else {
		for (unsigned int i = 0; i < n; i++) {
			mVtxArray[i * 2 + 0] = mState.mMatrix * point4(p[i]);
			mVtxArray[i * 2 + 1] = mState.mColor;
		}
	}
}

/*----------------------------------------------------------------------------*/

template<typename T> void SimpleDraw::Point(const T &p)
{
	T v = p;
	Points(&v, 1);
}

template<typename T> void SimpleDraw::Points(const T *p, unsigned int n, const vec4f *color)
{
	CopyArray(p, n, color);
	DrawStuff(GL_POINTS, n);
}

template<typename T> void SimpleDraw::Line(const T &p0, const T &p1, const vec4f *color)
{
	T v[2];
	v[0] = p0;
	v[1] = p1;
	CopyArray(v, 2, color);
	DrawStuff(GL_LINES, 2);
}

template<typename T> void SimpleDraw::Lines(const T *p, unsigned int n, const vec4f *color)
{
	Assert(n % 2 == 0);
	CopyArray(p, n, color);
	DrawStuff(GL_LINES, n);
}

template<typename T> void SimpleDraw::LineStrip(const T *p, unsigned int n, const vec4f *color)
{
	Assert(n > 1);
	CopyArray(p, n, color);
	DrawStuff(GL_LINE_STRIP, n);
}

template<typename T> void SimpleDraw::LineLoop(const T *p, unsigned int n, const vec4f *color)
{
	Assert(n > 1);
	CopyArray(p, n, color);
	DrawStuff(GL_LINE_LOOP, n);
}

template<typename T> void SimpleDraw::Triangle(const T &p0, const T &p1, const T &p2)
{
	T v[3] = {p0, p1, p2};
	LineLoop(v, 3);
}

template<typename T> void SimpleDraw::FilledTriangle(const T &p0, const T &p1, const T &p2)
{
	T v[3] = {p0, p1, p2};
	CopyArray(v, 3, nullptr);
	DrawStuff(GL_TRIANGLES, 3);
}

template<typename T> void SimpleDraw::FilledTriangles(const vec3f *p, int n)
{
	CopyArray(p, n * 3, nullptr);
	DrawStuff(GL_TRIANGLES, n * 3);
}

template<typename T> void SimpleDraw::Box(const T &p0, const T &p1)
{
	LogWarning("Not yet implemented");
}
template<> void SimpleDraw::Box<vec2f>(const vec2f &p0, const vec2f &p1);
template<> void SimpleDraw::Box<vec3f>(const vec3f &p0, const vec3f &p1);

template<typename T> void SimpleDraw::FilledBox(const T &p0, const T &p1)
{
	LogWarning("Not yet implemented");
}
template<> void SimpleDraw::FilledBox<vec2f>(const vec2f &p0, const vec2f &p1);

template<typename T> void SimpleDraw::QuadraticBezier(const T &p0, const T &p1, const T &p2, int segments)
{
	if (segments > 256)
		segments = 256;
	T c[256 + 1];

	for (int i = 0; i < segments + 1; i++) {
		float t = float(i) / float(segments);
		float nt = 1.0f - t;
		c[i] = nt * nt * p0 + 2.0f * t * nt * p1 + t * t * p2;
	}
	LineStrip(c, segments + 1);
}

template<typename T> void SimpleDraw::CubicBezier(const T &p0, const T &p1, const T &p2, const T &p3, int segments)
{
	if (segments > 256)
		segments = 256;
	T c[256 + 1];

	for (int i = 0; i < segments + 1; i++) {
		float t = float(i) / float(segments);
		float n = 1.0f - t;
		float tt = t * t;
		float nn = n * n;
		c[i] = nn * n * p0 + 3.0f * nn * t * p1 + 3.0f * n * tt * p2 + tt * t * p3;
	}
	LineStrip(c, segments + 1);
}

template<typename T> void SimpleDraw::Vector(const T &vect, T origin, float arrowSize)
{
	const T p0 = origin;
	const T p1 = origin + vect;
	Ray(p0, p1, arrowSize);
}

template<typename T> void SimpleDraw::Ray(const T &from, const T &to, float arrowSize)
{
	T p0 = from;
	T p1 = to;
	T vect = to - from;
	T v[2] = {p0, p1};
	Lines(v, 2);

	vec3f f0 = divW(mState.mMatrixInverse * v4f(0.0, 0.0f, 1.0f, 1.0f));
	vec3f f1 = divW(mState.mMatrixInverse * v4f(0.0, 0.0f, 0.0f, 1.0f));
	vec3f n = normalize(f1 - f0);
	mat3f R0 = mat3f::RotationMatrix(toRadf(30.0f), n);
	mat3f R1 = mat3f::RotationMatrix(toRadf(-30.0f), n);
	vec3f ap0 = R0 * -vect;
	vec3f ap1 = R1 * -vect;
	T v0[2] = {p1, p1 + arrowSize * ap0};
	Lines(v0, 2);
	T v1[2] = {p1, p1 + arrowSize * ap1};
	Lines(v1, 2);
}

template<typename T> void SimpleDraw::WireSphere(const T &origin, float radius, int segments)
{
	Circle(origin, v3f(1.0f, 0.0f, 0.0f), radius, segments);
	Circle(origin, v3f(0.0f, 1.0f, 0.0f), radius, segments);
	Circle(origin, v3f(0.0f, 0.0f, 1.0f), radius, segments);
}

template<typename T> void SimpleDraw::Frustum(const T &matrix)
{
	mat4f M = cast<f32>(matrix);

	vec3f n[4] = {
		divW(M * v4f(-1.0f, -1.0f, -1.0f, 1.0f)),
		divW(M * v4f( 1.0f, -1.0f, -1.0f, 1.0f)),
		divW(M * v4f( 1.0f,  1.0f, -1.0f, 1.0f)),
		divW(M * v4f(-1.0f,  1.0f, -1.0f, 1.0f))
	};
	vec3f f[4] = {
		divW(M * v4f(-1.0f, -1.0f, 1.0f, 1.0f)),
		divW(M * v4f( 1.0f, -1.0f, 1.0f, 1.0f)),
		divW(M * v4f( 1.0f,  1.0f, 1.0f, 1.0f)),
		divW(M * v4f(-1.0f,  1.0f, 1.0f, 1.0f))
	};
	vec3f p = divW(M * v4f(0.0f, 0.0f, 1.0f, 0.0f));

	Point(p);
	for (int i = 0; i < 4; i++) {
		Line(n[i], f[i]);
		Line(p, n[i]);
	}
	LineLoop(n, 4);
	LineLoop(f, 4);
	vec3f up0 = divW(M * v4f(0.0f, 0.9f, -1.0f, 1.0f));
	vec3f up1 = divW(M * v4f(0.0f, 1.5f, -1.0f, 1.0f));

	Vector(up1 - up0, up0);
}

/*----------------------------------------------------------------------------*/
