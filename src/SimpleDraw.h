/*
	WARNING!
		This collection drawing functions are Quick, Dirty and SLOW!
		They are only intended for debugging purposes!
*/


#pragma once

#include "VectorMath.h"
#include "StructDesc.h"
#include "GLB.h"

class SimpleDraw {
public:
	static void Init();
	static void Destroy();

public:
	SimpleDraw();
	~SimpleDraw();

	mat4f ExchangeMatrix(mat4f newMatrix);
	vec4f ExchangeColor(vec4f newColor);
	void PushState();
	void PopState();

	template<typename T> void	Point			(const T &p);
	template<typename T> void	Points			(const T *p, unsigned int n, const vec4f *color = nullptr);
	template<typename T> void	Line			(const T &p0, const T &p1, const vec4f *color = nullptr);
	template<typename T> void	Lines			(const T *p, unsigned int n, const vec4f *color = nullptr);
	template<typename T> void	LineStrip		(const T *p, unsigned int n, const vec4f *color = nullptr);
	template<typename T> void	LineLoop		(const T *p, unsigned int n, const vec4f *color = nullptr);
	template<typename T> void	Triangle		(const T &p0, const T &p1, const T &p2);
	template<typename T> void	FilledTriangle		(const T &p0, const T &p1, const T &p2);
	template<typename T> void	FilledTriangles		(const vec3f *p, int n);
	template<typename T> void	Box			(const T &p0, const T &p1);
	template<typename T> void	FilledBox		(const T &p0, const T &p1);
	void				Texture			(const vec2f &p0, const vec2f &p1, const bonobo::Texture &texture, vec4i swizzleOrder = vec4i::Make(0, 1, 2, 3), unsigned int sampleLayer = 0, int mipBias = 0, vec2i flipAxis = vec2i::Make(1, 1));
	void				TexturedQuad		(const vec3f &p, const vec3f &faceDir, const vec3f &upDir, const bonobo::Texture &texture, vec2f scale = vec2f::Make(1.0f), vec4f color = v4f(1.0f));
	void				Sprite			(const vec3f &p, const bonobo::Texture &texture, vec2f scale = vec2f::Make(1.0f), vec4f color = vec4f::Make(1.0f));
	void				Sprites			(const vec3f *p, unsigned int n, const bonobo::Texture &texture, vec2f scale = vec2f::Make(1.0f), vec4f color = vec4f::Make(1.0f));
	void				Sprites			(const vec3f *p, unsigned int n, const bonobo::Texture &texture, const vec2f *scale, const float *rotation = nullptr, vec4f color = vec4f::Make(1.0f));
	template<typename T> void	QuadraticBezier		(const T &p0, const T &p1, const T &p2, int segments = 16);
	template<typename T> void	CubicBezier		(const T &p0, const T &p1, const T &p2, const T &p3, int segments = 16);
	void				Circle			(vec2f p, float radius, int segments = 16);
	void				FilledCircle		(vec2f p, float radius, int segments = 16);
	void				Circle			(vec3f p, vec3f axis, float radius, int segments = 16);
	void				FilledCircle		(vec3f p, vec3f axis, float radius, int segments = 16);
	void				CircleSector		(vec2f p, float radius, float radFrom, float radTo, int segments = 16);
	void				FilledCircleSector	(vec2f p, float radius, float radFrom, float radTo, int segments = 16);
	template<typename T> void	Vector			(const T &vect, T origin = T(0.0f), float arrowSize = 0.15f);
	template<typename T> void	Ray			(const T &from, const T &to, float arrowSize = 0.15f);
	template<typename T> void	WireSphere		(const T &origin, float radius, int segments = 16);
	template<typename T> void	Frustum			(const T &matrix);

protected:
	template<typename T> void CopyArray(const T *p, unsigned int n, const vec4f *color);
	void DrawStuff(GLenum mode, unsigned int n);

protected:
	static bonobo::ShaderProgram *sShaderProgram;
	static bonobo::ShaderProgram *sSpriteShaderProgram;
	static bonobo::ShaderProgram *sTexturedShaderProgram;
	static bonobo::ShaderProgram *sTexturedShaderProgramMS;
	static bonobo::ShaderProgram *sTexturedShaderProgramArray;
	static bonobo::VAO *sVao;
	static bonobo::VAO *sSpriteVao;
	static bonobo::VAO *sTexturedVao;
	static bonobo::VAO *sTexturedVaoMS;
	static bonobo::VAO *sTexturedVaoArray;
	static bonobo::VBO *sVbo;
	static bonobo::Sampler *sSampler;
	static bool sInitiated;

	//float *mVtxArray[1024 * 1024]; // TODO
	vec4f *mVtxArray;
	int mVtxArrayMem;

	struct State {
		mat4f mMatrix;
		mat4f mMatrixInverse;
		vec4f mColor;
	};
	State mState;
	State mStateStack[64];
	int mStackPtr;
};

#include "SimpleDraw.inl"

extern SimpleDraw gSimpleDraw;

