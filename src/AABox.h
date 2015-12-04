#pragma once

#include "VectorMath.h"
#include "Geometry.h"

template<typename T>
class AABox {
public:
	AABox();
	AABox(const vec<3, T> &pt);
	AABox(const vec<3, T> &pt0, const vec<3, T> &pt1);
	~AABox();

public:
	void Include(const vec<3, T> &point);
	void Include(const AABox<T> &aabox);
	AABox<T> Combine(const AABox<T> &aabox) const;
	vec<3, T> Centrum() const;
	bool Encloses(const vec<3, T> &point) const;
	vec<3, T> GetCornerPoint(size_t i) const;
	vec<3, T> Diagonal() const;

	T Area() const;
	T Volume() const;

	bool ClipLine(vec<3, T> &p0, vec<3, T> &p1) const;
	//bool ClipRay(Ray<T> &ray) const; // TODO
	//int ClipPolygon(vec<3, T> *pts, int n) const; // TODO

	bool Intersect(const Ray<T> &ray) const;
	//bool Intersect(const Ray &ray, T &dist) const; // TODO
	bool Intersect(const Ray<T> &ray, const vec<3, T> &inv_dir, T &dist) const;
	bool Intersect(const Ray<T> &ray, const vec<3, T> &inv_dir, T &minDist, T &maxDist) const;
	bool Intersect(const AABox<T> &aabox) const;

public:
	vec<3, T>		minCorner;
	vec<3, T>		maxCorner;

};

#define AABOX_INC
#include "AABox.inl"
#undef AABOX_INC

typedef AABox<f32>			AABoxf;

