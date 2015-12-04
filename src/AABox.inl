#include "AABox.h"
#include <cmath>
#include <cfloat>

/*----------------------------------------------------------------------------*/

template<typename T>
AABox<T>::AABox()
{
	minCorner = vec<3, T>::Make(FLT_MAX, FLT_MAX, FLT_MAX);
	maxCorner = vec<3, T>::Make(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}

/*----------------------------------------------------------------------------*/

template<typename T>
AABox<T>::AABox(const vec<3, T> &pt)
{
	minCorner = maxCorner = pt;
}

/*----------------------------------------------------------------------------*/

template<typename T>
AABox<T>::AABox(const vec<3, T> &pt0, const vec<3, T> &pt1)
{
	for (int i = 0; i < 3; i++) {
		if (pt0[i] < pt1[i]) {
			minCorner[i] = pt0[i];
			maxCorner[i] = pt1[i];
		} else {
			minCorner[i] = pt1[i];
			maxCorner[i] = pt0[i];
		}
	}
}

/*----------------------------------------------------------------------------*/

template<typename T>
AABox<T>::~AABox()
{

}

/*----------------------------------------------------------------------------*/

template<typename T>
void AABox<T>::Include(const vec<3, T> &point)
{
	// TODO: Vec-instruction: perElemMin/Max?
	maxCorner.x = std::max(maxCorner.x, point.x);
	maxCorner.y = std::max(maxCorner.y, point.y);
	maxCorner.z = std::max(maxCorner.z, point.z);
	minCorner.x = std::min(minCorner.x, point.x);
	minCorner.y = std::min(minCorner.y, point.y);
	minCorner.z = std::min(minCorner.z, point.z);
}

/*----------------------------------------------------------------------------*/

template<typename T>
void AABox<T>::Include(const AABox<T> &aabox)
{
	for (size_t i = 0; i < 3; i++) {
		minCorner.V[i] = std::min(aabox.minCorner.V[i], minCorner.V[i]);
		maxCorner.V[i] = std::max(aabox.maxCorner.V[i], maxCorner.V[i]);
	}
}

/*----------------------------------------------------------------------------*/

template<typename T>
AABox<T> AABox<T>::Combine(const AABox<T> &aabox) const
{
	AABox<T> box;
	for (size_t i = 0; i < 3; i++) {
		box.minCorner.V[i] = std::min(aabox.minCorner.V[i], minCorner.V[i]);
		box.maxCorner.V[i] = std::max(aabox.maxCorner.V[i], maxCorner.V[i]);
	}
	return box;
}

/*----------------------------------------------------------------------------*/

template<typename T>
vec<3, T> AABox<T>::Centrum() const
{
	return (maxCorner + minCorner) / T(2);
}

/*----------------------------------------------------------------------------*/

template<typename T>
bool AABox<T>::Encloses(const vec<3, T> &point) const
{
	return point.x <= maxCorner.x && point.x >= minCorner.x &&
		   point.y <= maxCorner.y && point.y >= minCorner.y &&
		   point.z <= maxCorner.z && point.z >= minCorner.z;
}

/*----------------------------------------------------------------------------*/

template<typename T>
vec<3, T> AABox<T>::GetCornerPoint(size_t i) const
{
	const T *c[6] = {
		&minCorner.x, &minCorner.y, &minCorner.z,
		&maxCorner.x, &maxCorner.y, &maxCorner.z};
	vec<3, T> pt;
	pt.x = *c[(i & 1) * 3];
	pt.y = *c[((i >> 1) & 1) * 3 + 1];
	pt.z = *c[((i >> 2) & 1) * 3 + 2];
	return pt;
}

/*----------------------------------------------------------------------------*/

template<typename T>
vec<3, T> AABox<T>::Diagonal() const
{
	return maxCorner - minCorner;
}

/*----------------------------------------------------------------------------*/

template<typename T>
T AABox<T>::Area() const
{
	vec<3, T> diff = maxCorner - minCorner;
	return static_cast<T>(2) * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
}

/*----------------------------------------------------------------------------*/

template<typename T>
T AABox<T>::Volume() const
{
	vec<3, T> diff = maxCorner - minCorner;
	return diff.lengthSquared();
}

/*----------------------------------------------------------------------------*/

// TODO: Not SSE friendly
template<typename T>
bool AABox<T>::ClipLine(vec<3, T> &p0, vec<3, T> &p1) const
{
	f32 t0 = 0.0f;
	f32 t1 = 1.0f;
	for (i32 i = 0; i < 3; i++) {
		f32 invDir = 1.0f / (p1.V[i] - p0.V[i]);
		f32 tNear = (minCorner[i] - p0.V[i]) * invDir;
		f32 tFar  = (maxCorner[i] - p0.V[i]) * invDir;
		if (tNear > tFar)
			std::swap(tNear, tFar);
		if (tNear > t0) t0 = tNear;
		if (tFar < t1) t1 = tFar;
		if (t0 > t1) return false;
	}
	vec3f dir = p1 - p0;
	p1 = p0 + t1 * dir;
	p0 = p0 + t0 * dir;
	return true;
}

/*----------------------------------------------------------------------------*/

// TODO: Not SSE friendly
template<typename T>
bool AABox<T>::Intersect(const Ray<T> &ray) const
{
	f32 t0 = ray.t0;
	f32 t1 = ray.t1;
	for (i32 i = 0; i < 3; i++) {
		f32 invDir = 1.0f / ray.direction[i];
		f32 tNear = (minCorner[i] - ray.origin[i]) * invDir;
		f32 tFar  = (maxCorner[i] - ray.origin[i]) * invDir;
		if (tNear > tFar) {
			f32 tmp = tNear;
			tNear = tFar;
			tFar = tmp;
		}
		if (tNear > t0) t0 = tNear;
		if (tFar < t1) t1 = tFar;
		if (t0 > t1) return false;
	}
	//dist = t0;
	return true;
}

/*----------------------------------------------------------------------------*/

// TODO: Not SSE friendly
template<typename T>
bool AABox<T>::Intersect(const Ray<T> &ray, const vec<3, T> &inv_dir, T &dist) const
{
	f32 tmin = 0.0f;
	f32 tmax = fMax; // TODO: T
	for (size_t i = 0; i < 3; i++) {
		f32 t1 = (minCorner.V[i] - ray.origin.V[i]) * inv_dir.V[i];
		f32 t2 = (maxCorner.V[i] - ray.origin.V[i]) * inv_dir.V[i];
		if (t1 > t2) {
			f32 tmp = t1;
			t1 = t2;
			t2 = tmp;
		}
		tmin = std::max(tmin, t1);
		tmax = std::min(tmax, t2);
		if (tmin > tmax) {
			dist = fMax;
			return false;
		}
	}

	dist = tmin;
	return true;
}

/*----------------------------------------------------------------------------*/

// TODO: Not SSE friendly
template<typename T>
bool AABox<T>::Intersect(const Ray<T> &ray, const vec<3, T> &inv_dir, T &minDist, T &maxDist) const
{
	for (size_t i = 0; i < 3; i++) {
		f32 t1 = (minCorner.V[i] - ray.origin.V[i]) * inv_dir.V[i];
		f32 t2 = (maxCorner.V[i] - ray.origin.V[i]) * inv_dir.V[i];
		if (t1 > t2) {
			f32 tmp = t1;
			t1 = t2;
			t2 = tmp;
		}
		minDist = std::max(minDist, t1);
		maxDist = std::min(maxDist, t2);
		if (minDist > maxDist)
			return false;
	}
	return true;
}

/*----------------------------------------------------------------------------*/

// TODO: SSE
template<typename T>
bool AABox<T>::Intersect(const AABox<T> &aabox) const
{
	return !(aabox->maxCorner.x < minCorner.x || aabox->minCorner.x > maxCorner.x ||
		aabox->maxCorner.y < minCorner.y || aabox->minCorner.y > maxCorner.y ||
		aabox->maxCorner.z < minCorner.z || aabox->minCorner.z > maxCorner.z);
}

/*----------------------------------------------------------------------------*/
