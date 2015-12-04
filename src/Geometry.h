#pragma once

#include "VectorMath.h"
#include "Types.h"

template<typename T>
struct Ray {
	Ray(vec<3, T> &origin_, vec<3, T> &direction_, T t0_, T t1_) : 
		origin(origin_), direction(direction_), t0(t0_), t1(t1_) {}
	Ray(vec<3, T> &origin_, vec<3, T> &direction_) :
		origin(origin_), direction(direction_), t0(T(0)), t1(fMax) {} // TODO

	vec<3, T>	origin;
	vec<3, T>	direction;
	T			t0, t1;
};

template<typename T>
FORCE_INLINE bool LineLineIntersection2D(T &pT, T &qT, vec<2, T> p0, vec<2, T> p1, vec<2, T> q0, vec<2, T> q1)
{
	vec<2, T> pd = p1 - p0;
	vec<2, T> qd = q1 - q0;
	T qd_x_pd_inv = 1.0f / pseudoCross(qd, pd);
	vec<2, T> q0_p0 = p0 - q0;
	qT = pseudoCross(q0_p0, pd) * qd_x_pd_inv;
	pT = pseudoCross(q0_p0, qd) * qd_x_pd_inv;
	if (qT < T(0) || qT >= T(1) || pT < T(0) || pT >= T(1))
		return false;
	return true;
}
