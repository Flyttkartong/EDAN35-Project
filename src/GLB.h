#pragma once

#include "VectorMath.h"
#include "GLBdecl.h"
#include "StructDesc.h"

/*===========================================================================*/

namespace bonobo
{
  void APIENTRY openglErrorCallback( GLenum source, GLenum type, GLuint id
                          , GLenum severity, GLsizei /*length*/
                          , GLchar const* msg, void const* /*data*/
                          );

  void init();
  void destroy();

  bool checkForErrors();

  bonobo::Shader *loadShader(const std::vector<std::string> &code, bonobo::ShaderType type, const char *debugIdentifier = nullptr);
  void unloadShader(bonobo::Shader *handle);

  bonobo::ShaderProgram *loadShaderProgram(const std::vector<bonobo::Shader *> &shaders, const std::vector<std::string> *varyings = nullptr, const char *debugIdentifier = nullptr);
  bonobo::ShaderProgram *loadShaderProgram(const std::vector<std::string> &shaderFilenames, const std::string *prefix = nullptr, const std::vector<std::string> *varyings = nullptr);
  bonobo::ShaderProgram *loadShaderProgram(const std::string shaderFilenames[], unsigned int nFiles, const std::string *prefix = nullptr, const std::vector<std::string> *varyings = nullptr);
  void unloadShaderProgram(bonobo::ShaderProgram *handle);

  bonobo::VAO *loadVertexAttributeObject(const bonobo::ShaderProgram &sp, const bonobo::VBO &vbo, const StructDesc &desc, unsigned int slot = 0);
  bonobo::VAO *loadVertexAttributeObject(const bonobo::ShaderProgram &sp, const bonobo::VBO &vbo, const StructDesc *desc, unsigned int n, unsigned int *slots = nullptr);
  //void bindVertexAttributeObject(bonobo::VAO &vao);
  // TODO Unload

  bonobo::VBO *loadVertexBufferObject(const void *new_array, unsigned int stride, unsigned int n, bonobo::Usage usage = USAGE_STATIC);
  void bindVertexBufferObject(const bonobo::VBO &vbo, unsigned int slot, unsigned int offset = 0);
  void replaceVertexBufferObjectData(bonobo::VBO &handle, void *new_array, unsigned int n);
  // TODO Unload

  //bonobo::State *loadRenderState(RenderStateDesc &desc);
  //
  //void setFullRenderState(bonobo::State &state);
  //void setRenderState(bonobo::State &state);

  bonobo::Texture *loadTexture(const u8 *data, u32 width, u32 height, u32 depth, u32 layers, u32 msaa, u32 mipLevels, bonobo::TextureType type, vec4i componentBits);
  bonobo::Texture *loadTexture1D(const u8 *pixels, u32 width, bonobo::TextureType type, vec4i componentBits, u32 mipLevels = 1000);
  bonobo::Texture *loadTexture1D(u32 width, bonobo::TextureType type, vec4i componentBits);
  bonobo::Texture *loadTexture2D(const u8 *pixels, u32 width, u32 height, bonobo::TextureType type, vec4i componentBits, u32 mipLevels = 1000);
  bonobo::Texture *loadTexture2D(u32 width, u32 height, bonobo::TextureType type, vec4i componentBits, u32 msaa = 1);
  bonobo::Texture *loadTexture2D(std::string filename, u32 mipLevels = 1000);
  bonobo::Texture *loadTexture3D(const u8 *voxels, u32 width, u32 height, u32 depth, bonobo::TextureType type, vec4i componentBits, u32 mipLevels = 1000);
  bonobo::Texture *loadTexture2DArray(const u8 *pixels, u32 width, u32 height, u32 depth, bonobo::TextureType type, vec4i componentBits, u32 mipLevels = 1000);
  // TODO: DepthTexture

  bool uploadTexture(const bonobo::Texture &texture, const u8 *source, int mipLevel = 0);

  void readTexture(void *target, const bonobo::Texture &texture, int mipLevel = 0);
  void readTexture(void *target, const bonobo::Texture &texture, int x, int y, int w, int h, int mipLevel = 0);
  void unloadTexture(bonobo::Texture *handle);

  bonobo::Sampler *loadSampler(bonobo::TextureWrap wrapping = TEXTURE_REPEAT, bonobo::TextureFilter filter = TEXTURE_BEST);
  void unloadSampler(bonobo::Sampler *sampler);

  bonobo::TBO *loadTextureBufferObject(const u8 *data, unsigned int bytes, bonobo::TextureType type, vec4i componentBits, bonobo::Usage usage = USAGE_STATIC);
  void unloadTextureBufferObject(bonobo::TBO *handle);

  bonobo::FBO *loadFrameBufferObject(const std::vector<const bonobo::Texture *> *colorAttachments, const bonobo::Texture *depthAttachment = nullptr,
          const bonobo::Texture *stencilAttachment = nullptr, const bonobo::Texture *depthStencilAttachment = nullptr);
  bonobo::FBO *loadFrameBufferObject(const bonobo::Texture **colorAttachments, unsigned int n, const bonobo::Texture *depthAttachment = nullptr,
          const bonobo::Texture *stencilAttachment = nullptr, const bonobo::Texture *depthStencilAttachment = nullptr);
  void unloadFrameBufferObject(bonobo::FBO *handle);

  bonobo::RBO *loadRenderBufferObject(bonobo::RBO_Target target, unsigned int w, unsigned int h, int msaa = 0, const bonobo::FBO *fbo = nullptr);
  void unloadRenderBufferObject(bonobo::RBO *handle);

  void setRenderTarget(const bonobo::FBO *fbo, const bonobo::RBO *rbo = nullptr);

  bonobo::IBO *loadIndexBufferObject(const void *new_array, unsigned int type_size, unsigned int n, bonobo::Usage usage = USAGE_STATIC);
  void unloadIndexBufferObject(bonobo::IBO *handle);

  void bindTexture(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, const bonobo::Texture &handle);
  void bindTexture(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, GLuint id);
  void bindTextureBufferObject(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, const bonobo::TBO &handle);

  void bindTextureSampler(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, const bonobo::Texture &texhandle, const bonobo::Sampler &smphandle);

  void drawFullscreen(const bonobo::ShaderProgram &sp);

  void clearColor(f32 color[4]);
  void clearDepth(f32 depth);
  void clearStencil(u32 stencil);

  template<typename T> bool setUniform(bonobo::ShaderProgram &sp, const std::string &name, const T &value);
  template<typename T> bool setUniformArray(bonobo::ShaderProgram &sp, const std::string &name, const T *values, int n);
  // TODO: Uniform block

  void debugLogUnsetUniforms(const bonobo::ShaderProgram &sp);

}
