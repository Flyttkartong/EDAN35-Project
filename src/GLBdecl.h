// TODO: enum class

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "StructDesc.h"
#include "Types.h"

// TODO: Get rid of me
#define N_SHADER_TYPES	6
#define DISABLE_SOMETHING 0

// TODO: Make sure this is safe
namespace bonobo
{

  enum Usage {
          USAGE_STATIC = GL_STATIC_DRAW,
          USAGE_DYNAMIC = GL_DYNAMIC_DRAW,
          USAGE_STREAM = GL_STREAM_DRAW
  };

  enum PrimitiveLayout {
          LAYOUT_TRIANGLES = GL_TRIANGLES,
          LAYOUT_TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
          LAYOUT_TRIANGLE_FAN = GL_TRIANGLE_FAN,
          LAYOUT_LINES = GL_LINES,
          LAYOUT_LINE_LOOP = GL_LINE_LOOP,
          LAYOUT_LINE_STRIP = GL_LINE_STRIP,
          LAYOUT_POINTS = GL_POINTS,
          LAYOUT_PATCHES = GL_PATCHES
  };

  enum TextureFilter {
          TEXTURE_NEAREST = GL_NEAREST,
          TEXTURE_BILINEAR = GL_LINEAR,
          TEXTURE_TRILINEAR = GL_LINEAR_MIPMAP_LINEAR,
          TEXTURE_BEST
  };

  enum TextureWrap {
          TEXTURE_CLAMP = GL_CLAMP_TO_EDGE,
          TEXTURE_REPEAT = GL_REPEAT,
          TEXTURE_MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
          TEXTURE_BORDER = GL_CLAMP_TO_BORDER
  };

  enum TextureLayout {
          TEXTURE_1D = GL_TEXTURE_1D,
          TEXTURE_2D = GL_TEXTURE_2D,
          TEXTURE_3D = GL_TEXTURE_3D,
          TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
          TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY,
          TEXTURE_2D_MS = GL_TEXTURE_2D_MULTISAMPLE,
          TEXTURE_2D_MS_ARRAY = GL_TEXTURE_2D_MULTISAMPLE_ARRAY
  };

  /** Comparation strategy for depth and stencil */
  enum Compare {
          CMP_NEVER = GL_NEVER,
          CMP_LESS = GL_LESS,
          CMP_EQUAL = GL_EQUAL,
          CMP_LEQUAL = GL_LEQUAL,
          CMP_GREATER = GL_GREATER,
          CMP_NOTEQUAL = GL_NOTEQUAL,
          CMP_GEQUAL = GL_GEQUAL,
          CMP_ALWAYS = GL_ALWAYS
  };

  /** Source blending methods */
  enum SrcBlend {
          SB_ZERO = GL_ZERO,
          SB_ONE = GL_ZERO,
          SB_SRC_COLOR = GL_SRC_COLOR,
          SB_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
          SB_SRC_ALPHA = GL_SRC_ALPHA,
          SB_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
          SB_DST_ALPHA = GL_DST_ALPHA,
          SB_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
          SB_CONSTANT_COLOR = GL_CONSTANT_COLOR,
          SB_ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
          SB_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
          SB_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
          SB_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE,
          SB_SRC1_COLOR = GL_SRC1_COLOR,
          SB_ONE_MINUS_SRC1_COLOR = GL_ONE_MINUS_SRC1_COLOR,
          SB_SRC1_ALPHA = GL_SRC1_ALPHA,
          SB_ONE_MINUS_SRC1_ALPHA = GL_ONE_MINUS_SRC1_ALPHA
  };

  /** Destination blending methods */
  enum DstBlend {
          DB_ZERO = GL_ZERO,
          DB_ONE = GL_ONE,
          DB_SRC_COLOR = GL_SRC_COLOR,
          DB_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
          DB_DST_COLOR = GL_DST_COLOR,
          DB_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
          DB_SRC_ALPHA = GL_SRC_ALPHA,
          DB_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
          DB_DST_ALPHA = GL_DST_ALPHA,
          DB_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
          DB_CONSTANT_COLOR = GL_CONSTANT_COLOR,
          DB_ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
          DB_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
          DB_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA
  };

  /** Blending equations */
  enum BlendEquation {
          EQ_FUNC_ADD = GL_FUNC_ADD,
          EQ_FUNC_SUBTRACT = GL_FUNC_SUBTRACT,
          EQ_FUNC_REVERSE_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT,
          EQ_FUNC_MIN = GL_MIN,
          EQ_FUNC_MAX = GL_MAX
  };


  /** Culling modes */
  enum Culling {
          CULL_FRONT = GL_FRONT,
          CULL_BACK = GL_BACK,
          CULL_FRONT_AND_BACK = GL_FRONT_AND_BACK,
          CULL_NONE = DISABLE_SOMETHING
  };

  /** Stencil operations */
  enum StencilOp {
          STENCIL_KEEP = GL_KEEP,
          STENCIL_ZERO = GL_ZERO,
          STENCIL_REPLACE = GL_REPLACE,
          STENCIL_INCR = GL_INCR,
          STENCIL_INCR_WRAP = GL_INCR_WRAP,
          STENCIL_DECR = GL_DECR,
          STENCIL_DECR_WRAP = GL_DECR_WRAP,
          STENCIL_INVERT = GL_INVERT,
          //STENCIL_NONE = DISABLE_SOMETHING
  };

  /** Shader types */
  enum ShaderType {
          SHADER_TYPE_VERTEX = GL_VERTEX_SHADER,
          SHADER_TYPE_FRAGMENT = GL_FRAGMENT_SHADER,
          SHADER_TYPE_GEOMETRY = GL_GEOMETRY_SHADER,
          SHADER_TYPE_CONTROL = GL_TESS_CONTROL_SHADER,
          SHADER_TYPE_EVALUATION = GL_TESS_EVALUATION_SHADER,
          SHADER_TYPE_COMPUTE = GL_COMPUTE_SHADER
  };

  enum DataType {
          TYPE_HALF_FLOAT = GL_HALF_FLOAT,
          TYPE_FLOAT = GL_FLOAT,
          TYPE_FIXED = GL_FIXED,
          TYPE_INT_2_10_10_10_REV = GL_INT_2_10_10_10_REV,
          TYPE_UNSIGNED_INT_2_10_10_10_REV = GL_UNSIGNED_INT_2_10_10_10_REV,
          TYPE_UNSIGNED_INT_10F_11F_11F_REV = GL_UNSIGNED_INT_10F_11F_11F_REV,

          TYPE_BYTE = GL_BYTE,
          TYPE_UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
          TYPE_SHORT = GL_SHORT,
          TYPE_UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
          TYPE_INT = GL_INT,
          TYPE_UNSIGNED_INT = GL_UNSIGNED_INT,
  
          TYPE_DOUBLE = GL_DOUBLE
  };

  bonobo::DataType Type(types::DataType dt);

  enum TextureType {
          TEXTURE_UNORM = 0,
          TEXTURE_SNORM,
          TEXTURE_INT,
          TEXTURE_UINT,
          TEXTURE_FLOAT,
          TEXTURE_FLOAT_DEPTH,
          TEXTURE_TYPE_LAST
  };

  enum RBO_Target
  {
          RBO_TARGET_DEPTH_24,
          RBO_TARGET_DEPTH_32F,
          RBO_TARGET_STENCIL,
          RBO_TARGET_DEPTH_STENCIL_24_8,
          RBO_TARGET_DEPTH_STENCIL_32F_8,
  };


  /*----------------------------------------------------------------------------*/

  class Resource
  {
  };

  struct Texture : public bonobo::Resource {
          GLuint				mId;
          GLenum				mFormat;
          GLenum				mInternalFormat;
          GLenum				mPrecision;
          bonobo::TextureLayout	mTarget;
          unsigned int		mWidth;
          unsigned int		mHeight;
          unsigned int		mDepth;
          unsigned int		mMultisamples;
          unsigned int		mLayers;
          unsigned int		mMipLevels;
  };

  struct Sampler : public bonobo::Resource {
          GLuint			mId;
  };

  struct TBO : public bonobo::Resource {
          GLuint			mTboId;
          GLuint			mTextureId;
  };

  struct VAO : public bonobo::Resource {
          GLuint			mId;
  };

  struct VBO : public bonobo::Resource {
          GLuint			mId;
          GLuint			mUsage;
          int				mStride;
          int				mN;
  };

  struct IBO : public bonobo::Resource {
          GLuint			mId;
          GLuint			mUsage;
          GLuint			mTypesize;
          int				mN;
  };

  struct Shader : public bonobo::Resource {
          bonobo::ShaderType	mType;
          GLuint			mId;
  };

  struct Location {
          std::string		mName;
          GLuint			mLoc;
          GLint			mSize;
          GLenum			mType;
          bool			mIsSet;
  };

  struct UniformBlockEntry {
          std::string		mName;
          GLuint			mLoc;
  };

  struct UniformBlock {
          StructDesc		mDesc;
          std::vector<bonobo::UniformBlockEntry> mLocs;
  };

  struct ShaderProgram : public bonobo::Resource {
          GLuint			mId;
          bonobo::Shader		mShaders[N_SHADER_TYPES];
          bool			mHasShader[N_SHADER_TYPES];
          std::unordered_map<std::string, bonobo::Location> mAttribLocs;
          std::unordered_map<std::string, bonobo::Location> mUniformLocs;
          std::unordered_map<std::string, bonobo::UniformBlock> mUniformBlocks;
  };

  struct RBO : public bonobo::Resource {
          GLuint			mId;
  };

  struct FBO : public bonobo::Resource {
          GLuint mId;
          std::vector<const bonobo::Texture *> *mColorAttachments;
          const bonobo::Texture *mDepthAttachment;
          const bonobo::Texture *mStencilAttachment;
          const bonobo::Texture *mDepthStencilAttachment;
  };

}
