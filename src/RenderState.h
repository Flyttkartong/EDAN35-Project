#pragma once

#include <unordered_map>

#include "VectorMath.h"
#include "GLBdecl.h"
#include "Misc.h"

// vec2f DepthRange;
// vec4f mBlendColor;

//	float LineWidth;
//	float PointSize;

enum RenderSubState {
	RENDER_STATE_DEPTH = 0,
	RENDER_STATE_COLOR_BLEND,
	RENDER_STATE_STENCIL,
	RENDER_STATE_SCISSOR,
	RENDER_STATE_CULLMODE,
	RENDER_STATE_POLYGON,
	RENDER_STATE_MULTISAMPLE,
	N_RENDER_STATES
};

struct RenderSubStateHash {
	std::size_t operator()(RenderSubState const& subState) const {
		return static_cast<std::size_t>(subState);
	}
};

class RenderState;

struct RenderStateBase {
	friend class RenderState;
protected:
	RenderStateBase() {}
public:
	virtual ~RenderStateBase() {}
	virtual RenderSubState Type() = 0;
protected:
	virtual RenderStateBase *GetTransition(RenderStateBase *dest_) = 0;
	virtual void Apply() = 0;
	
};

class RenderStateDepth : public RenderStateBase {
public:
	RenderStateDepth();
	int mTest;
	vec2f mBounds;
	bonobo::Compare mFunc;
	int mWrite;

	RenderSubState Type() {return RENDER_STATE_DEPTH;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderStateColor : public RenderStateBase {
public:
	RenderStateColor();
	vec4uc mColorWrite;
	int mBlendEnabled;
	bonobo::SrcBlend mBlendSrc;
	bonobo::DstBlend mBlendDst;
	bonobo::BlendEquation mBlendEq;
	
	RenderSubState Type() {return RENDER_STATE_COLOR_BLEND;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderStateStencil : public RenderStateBase {
public:
	RenderStateStencil();
	int mTest;
	bonobo::Compare mFunc;
	int mRef;
	unsigned int mFuncMask;
	unsigned int mWriteMask;
	bonobo::StencilOp mOpStencilFail;
	bonobo::StencilOp mOpStencilPassDepthFail;
	bonobo::StencilOp mOpStencilPassDepthPass;
	
	RenderSubState Type() {return RENDER_STATE_STENCIL;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderStateScissor : public RenderStateBase {
public:
	RenderStateScissor();
	int mTest;
	vec2i mBegin;
	vec2i mSize;
	
	RenderSubState Type() {return RENDER_STATE_SCISSOR;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderStateCullMode : public RenderStateBase {
public:
	RenderStateCullMode();
	bonobo::Culling mCullMode;
	
	RenderSubState Type() {return RENDER_STATE_CULLMODE;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderStatePolygon : public RenderStateBase {
public:
	RenderStatePolygon();
	float mFactor;
	float mUnits;
	
	RenderSubState Type() {return RENDER_STATE_POLYGON;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderStateMultisample : public RenderStateBase {
public:
	RenderStateMultisample();
	int mEnable;
	
	RenderSubState Type() {return RENDER_STATE_MULTISAMPLE;}
protected:
	RenderStateBase *GetTransition(RenderStateBase *dest_);
	void Apply();
};

class RenderState {
public:
	RenderState() {}
	~RenderState();

	RenderState *GetTransitionTo(RenderState &to);
	template<typename T> bool SetSubState(T &newState);
	void ApplyState();

private:
	std::unordered_map<RenderSubState, RenderStateBase *, RenderSubStateHash> mSubStates;
};

template<typename T>
bool RenderState::SetSubState(T &newState)
{
	bool existed = false;
	auto elem = mSubStates.find(newState.Type());
	if (elem != mSubStates.end()) {
		existed = true;
		delete elem->second;
	}
	T *n = new T();
	*n = newState;
	mSubStates[newState.Type()] = n;
	return existed;
}
