#include "RenderState.h"
#include "Misc.h"
#include "Log.h"

RenderStateDepth::RenderStateDepth() : RenderStateBase(),
	mTest(true), mBounds(vec2f::Make(0.0f, 1.0f)), mFunc(bonobo::CMP_LESS), mWrite(~0)
{
}

RenderStateBase *RenderStateDepth::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStateDepth *dest = (RenderStateDepth *) dest_;
	if (!mTest && !dest->mTest && !mWrite && !dest->mWrite)
		return nullptr;
	if (memcmp(this, dest, sizeof(RenderStateDepth)) == 0)
		return nullptr;
	return new RenderStateDepth(*dest);
}

void RenderStateDepth::Apply()
{
	if (mTest) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(mFunc);
	} else
		glDisable(GL_DEPTH_TEST);
	glDepthMask(mWrite ? GL_TRUE : GL_FALSE);
}

RenderStateColor::RenderStateColor() : RenderStateBase(),
	mColorWrite(vec4uc::Make(~0)), mBlendEnabled(false), mBlendSrc(bonobo::SB_ONE), mBlendDst(bonobo::DB_ZERO), mBlendEq(bonobo::EQ_FUNC_ADD)
{
}

RenderStateBase *RenderStateColor::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStateColor *dest = (RenderStateColor *) dest_;
	static const vec4uc noColorWrite = vec4uc::Make(u8(0));
	if (mColorWrite == noColorWrite && dest->mColorWrite == noColorWrite)
		return nullptr;
	if (memcmp(this, dest, sizeof(RenderStateColor)) == 0)
		return nullptr;
	return new RenderStateColor(*dest);
}

void RenderStateColor::Apply()
{
	if (mBlendEnabled) {
		glEnable(GL_BLEND);
		glBlendFunc(mBlendSrc, mBlendDst);
		glBlendEquation(mBlendEq);
	} else
		glDisable(GL_BLEND);
	glColorMask(mColorWrite.r, mColorWrite.g, mColorWrite.b, mColorWrite.a);
}

RenderStateStencil::RenderStateStencil() : RenderStateBase(),
	mTest(false), mFunc(bonobo::CMP_ALWAYS), mRef(0), mFuncMask(~0), mWriteMask(0), mOpStencilFail(bonobo::STENCIL_KEEP), mOpStencilPassDepthFail(bonobo::STENCIL_KEEP), mOpStencilPassDepthPass(bonobo::STENCIL_KEEP)
{
}

RenderStateBase *RenderStateStencil::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStateStencil *dest = (RenderStateStencil *) dest_;
	if (!mTest && !dest->mTest)
		return nullptr;
	if (memcmp(this, dest, sizeof(RenderStateStencil)) == 0)
		return nullptr;
	return new RenderStateStencil(*dest);
}

void RenderStateStencil::Apply()
{
	if (mTest) {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(mFunc, mRef, mFuncMask);
		glStencilOp(mOpStencilFail, mOpStencilPassDepthFail, mOpStencilPassDepthPass);
	} else
		glDisable(GL_STENCIL_TEST);
	glStencilMask(mWriteMask);
}

RenderStateScissor::RenderStateScissor() : RenderStateBase(),
	mTest(false), mBegin(vec2i::Make(0, 0)), mSize(vec2i::Make(256, 256))
{
}

RenderStateBase *RenderStateScissor::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStateScissor *dest = (RenderStateScissor *) dest_;
	if (!mTest && !dest->mTest)
		return nullptr;
	if (memcmp(this, dest, sizeof(RenderStateScissor)) == 0)
		return nullptr;
	return new RenderStateScissor(*dest);
}

void RenderStateScissor::Apply()
{
	if (mTest) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(mBegin.x, mBegin.y, mSize.x, mSize.y);
	} else
		glDisable(GL_SCISSOR_TEST);
}

RenderStateCullMode::RenderStateCullMode() : RenderStateBase(),
	mCullMode(bonobo::CULL_NONE)
{
}

RenderStateBase *RenderStateCullMode::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStateCullMode *dest = (RenderStateCullMode *) dest_;
	if (mCullMode == dest->mCullMode)
		return nullptr;
	return new RenderStateCullMode(*dest);
}

void RenderStateCullMode::Apply()
{
	if (mCullMode != bonobo::CULL_NONE) {
		glEnable(GL_CULL_FACE);
		glCullFace(mCullMode);
	} else
		glDisable(GL_CULL_FACE);
}

RenderStatePolygon::RenderStatePolygon() : RenderStateBase(),
	mFactor(0.0f), mUnits(0.0f)
{
}

RenderStateBase *RenderStatePolygon::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStatePolygon *dest = (RenderStatePolygon *) dest_;
	if (memcmp(this, dest, sizeof(RenderStatePolygon)) == 0)
		return nullptr;
	return new RenderStatePolygon(*dest);
}

void RenderStatePolygon::Apply()
{
	if (mFactor != 0.0f || mUnits != 0.0f) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(mFactor, mUnits);
	} else
		glDisable(GL_POLYGON_OFFSET_FILL);
	
}

RenderStateMultisample::RenderStateMultisample() : RenderStateBase(),
	mEnable(false)
{
}

RenderStateBase *RenderStateMultisample::GetTransition(RenderStateBase *dest_)
{
	if (dest_ == nullptr)
		return nullptr;
	Assert(dest_->Type() == Type());
	RenderStateMultisample *dest = (RenderStateMultisample *) dest_;
	if (mEnable == dest->mEnable)
		return nullptr;
	return new RenderStateMultisample(*dest);
}

void RenderStateMultisample::Apply()
{
	if (mEnable)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

RenderState::~RenderState()
{
	for (auto it = mSubStates.begin(); it != mSubStates.end(); ++it)
		delete it->second;
}

RenderState *RenderState::GetTransitionTo(RenderState &to)
{
	RenderState *transition = new RenderState();
	for (auto it = to.mSubStates.begin(); it != to.mSubStates.end(); ++it) {
		auto elem = mSubStates.find(it->first);
		if (elem == mSubStates.end())
			transition->mSubStates[it->first] = it->second;
		else {
			RenderStateBase *t = elem->second->GetTransition(it->second);
			if (t != nullptr)
				transition->mSubStates[it->first] = t;
		}
	}
	return transition;
}

void RenderState::ApplyState()
{
	for (auto it = mSubStates.begin(); it != mSubStates.end(); ++it)
		it->second->Apply();
}
