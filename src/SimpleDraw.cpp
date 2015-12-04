// TODO: Lots of duplicate code
// TODO: VBO Memory

#ifndef STANDALONE_VERSION
#include "config.hpp"
#else
#define SHADERS_PATH(n) SHADERS_DIR n
#define RESOURCES_PATH(n) RESOURCES_DIR n
#endif
#include "SimpleDraw.h"

/*----------------------------------------------------------------------------*/

SimpleDraw gSimpleDraw;

bonobo::ShaderProgram *SimpleDraw::sShaderProgram;
bonobo::ShaderProgram *SimpleDraw::sSpriteShaderProgram;
bonobo::ShaderProgram *SimpleDraw::sTexturedShaderProgram;
bonobo::ShaderProgram *SimpleDraw::sTexturedShaderProgramMS;
bonobo::ShaderProgram *SimpleDraw::sTexturedShaderProgramArray;
bonobo::VAO *SimpleDraw::sVao;
bonobo::VAO *SimpleDraw::sSpriteVao;
bonobo::VAO *SimpleDraw::sTexturedVao;
bonobo::VAO *SimpleDraw::sTexturedVaoMS;
bonobo::VAO *SimpleDraw::sTexturedVaoArray;
bonobo::VBO *SimpleDraw::sVbo;
bonobo::Sampler *SimpleDraw::sSampler;
bool SimpleDraw::sInitiated = false;

SimpleDraw::SimpleDraw()
{
	mVtxArray = nullptr;
	sVbo = nullptr;
	for (int i = 0; i < 64; i++) {
		mStateStack[i].mMatrix = mat4f::Identity();
		mStateStack[i].mMatrixInverse = mat4f::Identity();
		mStateStack[i].mColor = vec4f::Make(1.0f);
	}
	mState.mMatrix = mat4f::Identity();
	mState.mMatrixInverse = mat4f::Identity();
	mState.mColor = vec4f::Make(1.0f);
	mStackPtr = 0;
}

SimpleDraw::~SimpleDraw()
{
	if (mVtxArray != nullptr)
		free(mVtxArray);
	//bonobo::unloadVertexBufferObject(sVbo); // TODO
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::Init()
{
	if (sInitiated)
		return;
	sVbo = bonobo::loadVertexBufferObject(nullptr, 8 * sizeof(f32), 1024 * 1024, bonobo::USAGE_STREAM); // TODO
	{
		std::string shaders[] = { SHADERS_PATH("Simple.vert"), SHADERS_PATH("Simple.frag") };
		sShaderProgram = bonobo::loadShaderProgram(shaders, 2);
		StructDescEntry entries[] = {
			DescEntry<vec4f>("Vertex"),
			DescEntry<vec4f>("Color"),
		};
		StructDesc vtxDesc = StructDesc(entries, 2);
		sVao = bonobo::loadVertexAttributeObject(*sShaderProgram, *sVbo, vtxDesc);
	}

	{
		std::string shadersSpr[] = {SHADERS_PATH("SimpleSprite.vert"), SHADERS_PATH("SimpleSprite.frag")};
		sSpriteShaderProgram = bonobo::loadShaderProgram(shadersSpr, 2);
		std::string shadersTex[] = {SHADERS_PATH("SimpleTextured.vert"), SHADERS_PATH("SimpleTextured.frag")};
		sTexturedShaderProgram = bonobo::loadShaderProgram(shadersTex, 2);
		std::string shadersTexMS[] = {SHADERS_PATH("SimpleTextured.vert"), SHADERS_PATH("SimpleTexturedMS.frag")};
		sTexturedShaderProgramMS = bonobo::loadShaderProgram(shadersTexMS, 2);
		std::string shadersTexArray[] = {SHADERS_PATH("SimpleTextured.vert"), SHADERS_PATH("SimpleTexturedArray.frag")};
		sTexturedShaderProgramArray = bonobo::loadShaderProgram(shadersTexArray, 2);
		StructDescEntry entriesTex[] = {
			DescEntry<vec2f>("Vertex"),
			DescEntry<vec2f>("Texcoord"),
		};
		StructDesc vtxDescTex = StructDesc(entriesTex, 2);

		sTexturedVao = bonobo::loadVertexAttributeObject(*sTexturedShaderProgram, *sVbo, vtxDescTex);
		sTexturedVaoMS = bonobo::loadVertexAttributeObject(*sTexturedShaderProgramMS, *sVbo, vtxDescTex);
		sTexturedVaoArray = bonobo::loadVertexAttributeObject(*sTexturedShaderProgramArray, *sVbo, vtxDescTex);

		StructDescEntry entriesSpr[] = {
			DescEntry<vec3f>("Vertex"),
			DescEntry<vec2f>("Texcoord"),
			DescEntry<vec2f>("Scale"),
			DescEntry<float>("Rotation"),
		};
		StructDesc vtxDescSpr = StructDesc(entriesSpr, 4);
		sSpriteVao = bonobo::loadVertexAttributeObject(*sSpriteShaderProgram, *sVbo, vtxDescSpr);
	}

	sSampler = bonobo::loadSampler();

	sInitiated = true;
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::Destroy()
{
	bonobo::unloadShaderProgram(sShaderProgram);
	//bonobo::unloadVertexAttributeObject(vao);
	bonobo::unloadShaderProgram(sSpriteShaderProgram);
	bonobo::unloadShaderProgram(sTexturedShaderProgram);
	bonobo::unloadShaderProgram(sTexturedShaderProgramMS);
	bonobo::unloadShaderProgram(sTexturedShaderProgramArray);
	//texturedVao = bonobo::loadVertexAttributeObject(*texturedShaderProgram, vtxDescTex);
	//texturedVaoMS = bonobo::loadVertexAttributeObject(*texturedShaderProgramMS, vtxDescTex);
	//texturedVaoArray = bonobo::loadVertexAttributeObject(*texturedShaderProgramArray, vtxDescTex);
	//bonobo::unloadVertexBufferObject(vbo); // TODO
	bonobo::unloadSampler(sSampler);
}

/*----------------------------------------------------------------------------*/

mat4f SimpleDraw::ExchangeMatrix(mat4f newMatrix)
{
	mat4f tmp = mState.mMatrix;
	mState.mMatrix = newMatrix;
	mState.mMatrixInverse = inv(newMatrix);
	return tmp;
}

vec4f SimpleDraw::ExchangeColor(vec4f newColor)
{
	std::swap(mState.mColor, newColor);
	return newColor;
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::PushState()
{
	if (mStackPtr >= 64) { 
		LogError("Cannot push SimpleDraw state. Maximum stack depth reached. (64)");
		return;
	}
	mStateStack[mStackPtr++] = mState;
}

void SimpleDraw::PopState()
{
	if (mStackPtr <= 0) { 
		LogError("Cannot pop SimpleDraw state. Already at level 0.");
		return;
	}
	mState = mStateStack[--mStackPtr];
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::DrawStuff(GLenum mode, unsigned int n)
{
	if (!sInitiated)
		Init();
	if (mVtxArray == nullptr) {
		mVtxArrayMem = 1024 * 1024 * 4;
		mVtxArray = (vec4f *) malloc(mVtxArrayMem);
	}
	bonobo::checkForErrors();
	glBindBuffer(GL_ARRAY_BUFFER, sVbo->mId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f32) * 8 * n, mVtxArray);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(sShaderProgram->mId);
	//uniforms.SetValue("WorldViewProjection", mMatrix);
	//bonobo::setUniforms(*shaderProgram, uniforms);
	glBindVertexArray(sVao->mId);
	bonobo::bindVertexBufferObject(*sVbo, 0);
	glDrawArrays(mode, 0, n);
	glBindVertexArray(0);
	bonobo::checkForErrors();
}

/*----------------------------------------------------------------------------*/

template<> void SimpleDraw::Box<vec2f>(const vec2f &p0, const vec2f &p1)
{
	vec2f pos[4] = {
		vec2f::Make(p0.x, p0.y),
		vec2f::Make(p1.x, p0.y),
		vec2f::Make(p1.x, p1.y),
		vec2f::Make(p0.x, p1.y)};
	LineLoop<vec2f>(pos, 4);
}

// LOWPRIO: Massive optimizations possible...
template<> void SimpleDraw::Box(const vec3f &p0, const vec3f &p1)
{
	vec3f pt[8];
	const float *c[6] = {
		&p0.x, &p0.y, &p0.z,
		&p1.x, &p1.y, &p1.z};
	for (i32 i = 0; i < 8; i++) {
		pt[i].x = *c[(i & 1) * 3];
		pt[i].y = *c[((i >> 1) & 1) * 3 + 1];
		pt[i].z = *c[((i >> 2) & 1) * 3 + 2];
	}

	Line(pt[0], pt[1]);
	Line(pt[1], pt[3]);
	Line(pt[3], pt[2]);
	Line(pt[2], pt[0]);
	Line(pt[4], pt[5]);
	Line(pt[5], pt[7]);
	Line(pt[7], pt[6]);
	Line(pt[6], pt[4]);
	Line(pt[0], pt[4]);
	Line(pt[1], pt[5]);
	Line(pt[2], pt[6]);
	Line(pt[3], pt[7]);
}

/*----------------------------------------------------------------------------*/

template<> void SimpleDraw::FilledBox(const vec2f &p0, const vec2f &p1)
{
	FilledTriangle(v2f(p0.x, p0.y), v2f(p1.x, p0.y), v2f(p0.x, p1.y));
	FilledTriangle(v2f(p0.x, p1.y), v2f(p1.x, p0.y), v2f(p1.x, p1.y));
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::Circle(vec2f p, float radius, int segments)
{
	if (segments > 256)
		segments = 256;
	vec2f c[256];

	for (int i = 0; i < segments; i++) {
		float f = f2PI * float(i) / float(segments);
		float cs = cos(f) * radius;
		float sn = sin(f) * radius;
		c[i] = vec2f::Make(sn, cs) + p;
	}
	LineLoop(c, segments);
}

void SimpleDraw::FilledCircle(vec2f p, float radius, int segments)
{
	if (segments > 256)
		segments = 256;
	vec3f c[256][3];

	for (int i = 0; i < segments; i++) {
		float f = f2PI * float(i) / float(segments);
		float cs = cos(f) * radius;
		float sn = sin(f) * radius;
		c[i][0] = p.xy0();
		c[i][2] = vec3f::Make(sn, cs, 0.0f) + p.xy0();
	}
	for (int i = 0; i < segments; i++)
		c[i][1] = c[(i + 1) % segments][2];
	FilledTriangles<vec3f>((const vec3f *) c, segments);
}

void SimpleDraw::Circle(vec3f p, vec3f axis, float radius, int segments)
{
	if (segments > 256)
		segments = 256;
	vec3f c[256];

	normalize(axis);
	vec3f w = vec3f::Make(abs(axis.x), abs(axis.y), abs(axis.z));
	vec3f v = vec3f::Make(0.0f);
	v[minElementIndex(w)] = 1.0f;
	w = cross(axis, v);
	v = cross(axis, w);
	for (int i = 0; i < segments; i++) {
		float f = f2PI * float(i) / float(segments);
		float cs = cos(f) * radius;
		float sn = sin(f) * radius;
		c[i] = p + cs * v + sn * w;
	}
	LineLoop(c, segments);
}

void SimpleDraw::FilledCircle(vec3f p, vec3f axis, float radius, int segments)
{
	if (segments > 256)
		segments = 256;
	vec3f c[256][3];

	normalize(axis);
	vec3f w = vec3f::Make(abs(axis.x), abs(axis.y), abs(axis.z));
	vec3f v = vec3f::Make(0.0f);
	v[minElementIndex(w)] = 1.0f;
	w = cross(axis, v);
	v = cross(axis, w);

	for (int i = 0; i < segments; i++) {
		float f = f2PI * float(i) / float(segments);
		float cs = cos(f) * radius;
		float sn = sin(f) * radius;
		c[i][0] = p;
		c[i][2] = p + cs * v + sn * w;
	}
	for (int i = 0; i < segments; i++)
		c[i][1] = c[(i + 1) % segments][2];
	FilledTriangles<vec3f>((const vec3f *)c, segments);
}

void SimpleDraw::CircleSector(vec2f p, float radius, float radFrom, float radTo, int segments)
{
	if (segments > 256)
		segments = 256;
	vec2f c[256 + 1];

	for (int i = 0; i < segments + 1; i++) {
		float f = radFrom + (radTo - radFrom) * float(i) / float(segments);
		float cs = cos(f) * radius;
		float sn = sin(f) * radius;
		c[i] = vec2f::Make(sn, cs) + p;
	}
	LineStrip(c, segments + 1);
}

void SimpleDraw::FilledCircleSector(vec2f p, float radius, float radFrom, float radTo, int segments)
{
	if (segments > 256)
		segments = 256;
	vec3f c[256 + 1][3];

	for (int i = 0; i < segments + 1; i++) {
		float f = radFrom + (radTo - radFrom) * float(i) / float(segments);
		float cs = cos(f) * radius;
		float sn = sin(f) * radius;
		c[i][0] = p.xy0();
		c[i][2] = vec3f::Make(sn, cs, 0.0f) + p.xy0();
	}
	for (int i = 0; i < segments; i++)
		c[i][1] = c[i + 1][2];
	FilledTriangles<vec3f>((const vec3f *) c, segments);
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::Texture(const vec2f &p0, const vec2f &p1, const bonobo::Texture &texture, vec4i swizzleOrder, unsigned int sampleLayer, int mipBias, vec2i flipAxis)
{
	//bool ok = Param(swizzleOrder[0] >= 0 && swizzleOrder[0] <= 3) &&
	//	Param(swizzleOrder[1] >= 0 && swizzleOrder[1] <= 3) &&
	//	Param(swizzleOrder[2] >= 0 && swizzleOrder[2] <= 3) &&
	//	Param(swizzleOrder[3] >= 0 && swizzleOrder[3] <= 3);
	//if (!ok)
	//	return;

	vec4f v0 = mState.mMatrix * point4(p0);
	vec4f v1 = mState.mMatrix * point4(p1);

	f32 va[] = {
		v0.x, v0.y, 0.0f, 0.0f,
		v1.x, v0.y, 1.0f, 0.0f,
		v0.x, v1.y, 0.0f, 1.0f,
		v0.x, v1.y, 0.0f, 1.0f,
		v1.x, v0.y, 1.0f, 0.0f,
		v1.x, v1.y, 1.0f, 1.0f,
	};
	glBindBuffer(GL_ARRAY_BUFFER, sVbo->mId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f32) * 4 * 6, va);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	bonobo::ShaderProgram *sp;
	bonobo::VAO *vao;
	if (texture.mTarget == GL_TEXTURE_2D_MULTISAMPLE) {
		sp = sTexturedShaderProgramMS;
		vao = sTexturedVaoMS;
	} else if (texture.mTarget == GL_TEXTURE_2D_ARRAY) {
		sp = sTexturedShaderProgramArray;
		vao = sTexturedVaoArray;
	} else if (texture.mTarget == GL_TEXTURE_2D) {
		sp = sTexturedShaderProgram;
		vao = sTexturedVao;
	} else {
		LogWarning("Not yet implemented");
		return;
	}
	glUseProgram(sp->mId);
	bonobo::setUniform(*sp, "Swizzle", swizzleOrder);
	bonobo::setUniform(*sp, "Layer", (int) sampleLayer);
	bonobo::setUniform(*sp, "MipBias", mipBias);
	bonobo::setUniform(*sp, "FlipAxis", flipAxis);
	glBindVertexArray(vao->mId);
	bonobo::bindTextureSampler(*sp, "Texture", 0, texture, *sSampler);

	//glBindVertexBuffer(0, sVbo->mId, 0, sizeof(f32) * 4);
	glBindBuffer(GL_ARRAY_BUFFER, sVbo->mId);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindSampler(0, 0);
	glUseProgram(0);
	bonobo::checkForErrors();
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::TexturedQuad(const vec3f &p, const vec3f &faceDir, const vec3f &upDir, const bonobo::Texture &texture, vec2f scale, vec4f color)
{
	if (mVtxArray == nullptr) {
		mVtxArrayMem = 1024 * 1024 * 4;
		mVtxArray = (vec4f *) malloc(mVtxArrayMem);
	}
	float *f = (float *) &mVtxArray[0];

	const vec2f order[] = {
		vec2f::Make(0.0f, 0.0f), vec2f::Make(1.0f, 0.0f), vec2f::Make(0.0f, 1.0f),
		vec2f::Make(0.0f, 1.0f), vec2f::Make(1.0f, 0.0f), vec2f::Make(1.0f, 1.0f)
	};

	for (unsigned int i = 0; i < 6; i++) {
		f[i * 8 + 0] = p.x;
		f[i * 8 + 1] = p.y;
		f[i * 8 + 2] = p.z;
		f[i * 8 + 3] = order[i % 6].x;
		f[i * 8 + 4] = order[i % 6].y;
		f[i * 8 + 5] = scale.x;
		f[i * 8 + 6] = scale.y;
		f[i * 8 + 7] = 0.0f;
	}

	vec3f fd = normalize(faceDir);
	vec3f ud = normalize(upDir);
	ud = cross(cross(fd, ud), fd);

	glBindBuffer(GL_ARRAY_BUFFER, sVbo->mId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f32) * 8 * 6, f);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(sSpriteShaderProgram->mId);
	glBindVertexArray(sSpriteVao->mId);
	bonobo::bindTextureSampler(*sSpriteShaderProgram, "Texture", 0, texture, *sSampler);
	bonobo::setUniform(*sSpriteShaderProgram, "WorldToClip", mState.mMatrix);
	bonobo::setUniform(*sSpriteShaderProgram, "ClipToWorld", mState.mMatrixInverse);
	bonobo::setUniform(*sSpriteShaderProgram, "FaceDir", faceDir);
	bonobo::setUniform(*sSpriteShaderProgram, "UpDir", upDir);
	bonobo::setUniform(*sSpriteShaderProgram, "Color", color);

	glBindVertexBuffer(0, sVbo->mId, 0, sizeof(f32) * 8);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindSampler(0, 0);
	glUseProgram(0);
	bonobo::checkForErrors();
}

/*----------------------------------------------------------------------------*/

void SimpleDraw::Sprite(const vec3f &p, const bonobo::Texture &texture, vec2f scale, vec4f color)
{
	Sprites(&p, 1, texture, scale, color);
}

void SimpleDraw::Sprites(const vec3f *p, unsigned int n, const bonobo::Texture &texture, vec2f scale, vec4f color)
{
	if (mVtxArray == nullptr) {
		mVtxArrayMem = 1024 * 1024 * 4;
		mVtxArray = (vec4f *) malloc(mVtxArrayMem);
	}
	if (int(n * 8 * 6 * sizeof(float)) > mVtxArrayMem) {
		mVtxArrayMem = smax<int>(mVtxArrayMem + (mVtxArrayMem >> 2), (int) (n * 8 * 6 * sizeof(float)));
		mVtxArray = (vec4f *) realloc(mVtxArray, mVtxArrayMem);
	}

	float *f = (float *) &mVtxArray[0];

	const vec2f order[] = {
		vec2f::Make(0.0f, 0.0f), vec2f::Make(1.0f, 0.0f), vec2f::Make(0.0f, 1.0f),
		vec2f::Make(0.0f, 1.0f), vec2f::Make(1.0f, 0.0f), vec2f::Make(1.0f, 1.0f)
	};

	for (unsigned int i = 0; i < n * 6; i++) {
		int k = i / 6;
		f[i * 8 + 0] = p[k].x;
		f[i * 8 + 1] = p[k].y;
		f[i * 8 + 2] = p[k].z;
		f[i * 8 + 3] = order[i % 6].x;
		f[i * 8 + 4] = order[i % 6].y;
		f[i * 8 + 5] = scale.x;
		f[i * 8 + 6] = scale.y;
		f[i * 8 + 7] = 0.0f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, sVbo->mId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f32) * 8 * 6 * n, f);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(sSpriteShaderProgram->mId);
	glBindVertexArray(sSpriteVao->mId);
	bonobo::bindTextureSampler(*sSpriteShaderProgram, "Texture", 0, texture, *sSampler);
	bonobo::setUniform(*sSpriteShaderProgram, "WorldToClip", mState.mMatrix);
	bonobo::setUniform(*sSpriteShaderProgram, "ClipToWorld", mState.mMatrixInverse);
	bonobo::setUniform(*sSpriteShaderProgram, "FaceDir", vec3f::Make(0.0f));
	bonobo::setUniform(*sSpriteShaderProgram, "UpDir", vec3f::Make(0.0f));
	bonobo::setUniform(*sSpriteShaderProgram, "Color", color);

	glBindVertexBuffer(0, sVbo->mId, 0, sizeof(f32) * 8);
	glDrawArrays(GL_TRIANGLES, 0, 6 * n);
	glBindVertexArray(0);
	glBindSampler(0, 0);
	glUseProgram(0);
	bonobo::checkForErrors();
}

void SimpleDraw::Sprites(const vec3f *p, unsigned int n, const bonobo::Texture &texture, const vec2f *scale, const float *rotation, vec4f color)
{
	if (mVtxArray == nullptr) {
		mVtxArrayMem = 1024 * 1024 * 4;
		mVtxArray = (vec4f *) malloc(mVtxArrayMem);
	}
	if (int(n * 8 * 6 * sizeof(float)) > mVtxArrayMem) {
		mVtxArrayMem = smax<int>(mVtxArrayMem + (mVtxArrayMem >> 2), (int) (n * 8 * 6 * sizeof(float)));
		mVtxArray = (vec4f *) realloc(mVtxArray, mVtxArrayMem);
	}

	float *f = (float *) &mVtxArray[0];

	const vec2f order[] = {
		vec2f::Make(0.0f, 0.0f), vec2f::Make(1.0f, 0.0f), vec2f::Make(0.0f, 1.0f),
		vec2f::Make(0.0f, 1.0f), vec2f::Make(1.0f, 0.0f), vec2f::Make(1.0f, 1.0f)
	};

	for (unsigned int i = 0; i < n * 6; i++) {
		int k = i / 6;
		f[i * 8 + 0] = p[k].x;
		f[i * 8 + 1] = p[k].y;
		f[i * 8 + 2] = p[k].z;
		f[i * 8 + 3] = order[i % 6].x;
		f[i * 8 + 4] = order[i % 6].y;
	}

	if (scale) {
		for (unsigned int i = 0; i < n * 6; i++) {
			int k = i / 6;
			f[i * 8 + 5] = scale[k].x;
			f[i * 8 + 6] = scale[k].y;
		}
	} else {
		for (unsigned int i = 0; i < n * 6; i++) {
			f[i * 8 + 5] = 1.0f;
			f[i * 8 + 6] = 1.0f;
		}
	}

	if (rotation) {
		for (unsigned int i = 0; i < n * 6; i++) {
			int k = i / 6;
			f[i * 8 + 7] = rotation[k];
		}
	} else {
		for (unsigned int i = 0; i < n * 6; i++)
			f[i * 8 + 7] = 0.0f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, sVbo->mId);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f32) * 8 * 6 * n, f);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(sSpriteShaderProgram->mId);
	glBindVertexArray(sSpriteVao->mId);
	bonobo::bindTextureSampler(*sSpriteShaderProgram, "Texture", 0, texture, *sSampler);
	bonobo::setUniform(*sSpriteShaderProgram, "WorldToClip", mState.mMatrix);
	bonobo::setUniform(*sSpriteShaderProgram, "ClipToWorld", mState.mMatrixInverse);
	bonobo::setUniform(*sSpriteShaderProgram, "FaceDir", vec3f::Make(0.0f));
	bonobo::setUniform(*sSpriteShaderProgram, "UpDir", vec3f::Make(0.0f));
	bonobo::setUniform(*sSpriteShaderProgram, "Color", color);

	glBindVertexBuffer(0, sVbo->mId, 0, sizeof(f32) * 8);
	glDrawArrays(GL_TRIANGLES, 0, 6 * n);
	glBindVertexArray(0);
	glBindSampler(0, 0);
	glUseProgram(0);
	bonobo::checkForErrors();
}

/*----------------------------------------------------------------------------*/

