#pragma once

#include "VectorMath.h"

/**
 * A TRS-transform M is composed of rotation, translation and scale. We define
 * M as:
 *
 *    M = T * R * S
 *
 * Transformations can be used hierachically. This means that every transformable child
 * node will inherit the transformations of its parent. Let's say for instance,
 * that there are two nodes A and B. A is the parent of B and both are
 * transformable. The local transformation of A is denoted Ma and for B its Ma.
 * The matrix that brings nodes A and B to world space is denoted Wa and Wa.
 * The resulting transformations for the nodes will then be:
 *
 *    Ma = Ta * Ra * Sa
 *    Mb = Tb * Rb * Sb
 *
 *    Wa = Ma = Ta * Ra * Sa
 *    Wb = Ma * Mb = (Ta * Ra * Sa) * (Ta * Rb * Sb)
 *
 * The MB matrix will then, in turn, be recursively be applied to any children
 * of node B to construct new model->world matrices, in the same manner as in
 * the example above.
 *
 */
template<typename T>
class TRSTransform {

public:
	TRSTransform();
	~TRSTransform();

public:
	/* Reset the transformation to the identity matrix */
	void ResetTransform();

		/* Relative transformations */
		
	void Translate(vec<3, T> v);
	void Scale(vec<3, T> v);
	void Scale(T uniform);

	/* Rotate around vector (x, y, z) */
	void Rotate(T angle, vec<3, T> v);
	void RotateX(T angle);
	void RotateY(T angle);
	void RotateZ(T angle);
	void PreRotate(T angle, vec<3, T> v);
	void PreRotateX(T angle);
	void PreRotateY(T angle);
	void PreRotateZ(T angle);

		/* Absolute transformations */

	void SetTranslate(vec<3, T> v);
	void SetScale(vec<3, T> v);
	void SetScale(T uniform);

	/* Rotate around vector (x, y, z) */
	void SetRotate(T angle, vec<3, T> v);
	void SetRotateX(T angle);
	void SetRotateY(T angle);
	void SetRotateZ(T angle);

	
	void LookTowards(vec<3, T> front_vec, vec<3, T> up_vec);
	void LookTowards(vec<3, T> front_vec);
	void LookAt(vec<3, T> point, vec<3, T> up_vec);
	void LookAt(vec<3, T> point);

		/* Useful getters */

	mat<4, T> GetMatrix() const;
	mat<4, T> GetMatrixInverse() const;

	mat<3, T> GetRotation() const;
	vec<3, T> GetTranslation() const;
	vec<3, T> GetScale() const;

	mat<4, T> GetTranslationMatrix() const;
	mat<4, T> GetRotationMatrix() const;
	mat<4, T> GetScaleMatrix() const;

	mat<4, T> GetTranslationMatrixInverse() const;
	mat<4, T> GetRotationMatrixInverse() const;
	mat<4, T> GetScaleMatrixInverse() const;

	mat<4, T> GetTranslationRotationMatrix() const;

	vec<3, T> GetUp() const;
	vec<3, T> GetDown() const;
	vec<3, T> GetLeft() const;
	vec<3, T> GetRight() const;
	vec<3, T> GetFront() const;
	vec<3, T> GetBack() const;

protected:
	mat<3, T>	mR;
	vec<3, T>	mT;
	vec<3, T>	mS;

public:
	friend std::ostream &operator<<(std::ostream &os, TRSTransform<T> &v)
	{
		os << v.mT << std::endl;
		os << v.mR << std::endl;
		os << v.mS << std::endl;
		return os;
	}
	friend std::istream &operator>>(std::istream &is, TRSTransform<T> &v)
	{
		is >> v.mT;
		is >> v.mR;
		is >> v.mS;
		return is;
	}
};

#include "TRSTransform.inl"
