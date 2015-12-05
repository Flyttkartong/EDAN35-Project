#include "BuildSettings.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <cassert>
#include <cstring>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include "Log.h"
#include "GLB.h"
#include "lodepng.h"

#ifdef _WIN32
#pragma warning (disable : 4996) // This function or variable may be unsafe
#endif

#define ERR_BUF_LEN 8192

namespace bonobo
{
  static std::string getStringForType(GLenum type);
  static std::string getStringForSource(GLenum source);
  static std::string getStringForSeverity(GLenum severity);
  static Log::Type   getTypeForSeverity(GLenum severity);
  static bool        getShaderProgramUniformLocation(GLuint &dst, bonobo::ShaderProgram &sp, const std::string &name);
  static void        check_bound_shader_program(const bonobo::ShaderProgram &sp, const char *file, const char *function, i32 line);
  static void        debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

  static bool           initiated = false;
  static int            majorVersion = 2;
  static int            minorVersion = 0;
  static int            shaderMajorVersion[] = {2, 2, 3, 4, 4, 4};
  static int            shaderMinorVersion[] = {0, 0, 0, 0, 0, 3};
  static ShaderType shaderType[] = {SHADER_TYPE_VERTEX, SHADER_TYPE_FRAGMENT, SHADER_TYPE_GEOMETRY, SHADER_TYPE_CONTROL, SHADER_TYPE_EVALUATION, SHADER_TYPE_COMPUTE};
  static char *         filenameSuffix[] = {".vert", ".frag", ".geo", ".tcs", ".tes", ".comp"};
  static char *         shaderString[] = { "Vertex", "Fragment", "Geometry", "Control", "Evaluation", "Compute" };

  struct {
          int n_vaos;
          int n_vbos;
          int n_ibos;
          int n_rbos;
          int n_fbos;
          int n_tbos;
          int n_samplers;
          int n_textures;
          int n_cubemaps;
          int n_shaders;
          int n_shaderprogs;
  } Stats;
}


/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

#if DEBUG_LEVEL >= 1
#	define GL_CHECK_ERRORS()				check_errors(__FILE__, __FUNCTION__, __LINE__)
#else
#	define GL_CHECK_ERRORS()
#endif

#if DEBUG_LEVEL >= 2
#	define GL_CHECK_SHADER_BOUND(sp)		check_bound_shader_program(sp, __FILE__, __FUNCTION__, __LINE__)
#else
#	define GL_CHECK_SHADER_BOUND(sp)
#endif

/*----------------------------------------------------------------------------*/
	
bool gl_is_broken = false;
i32 error_line = -1;

/* Ensures that GL hasn't generated any errors */
bool check_errors(const char *file, const char *function, i32 line)
{
	GLenum result;
	result = glGetError();

	if (result) {
		if (error_line != line) {
			gl_is_broken = true;
			error_line = line;
			char *err = "";
			switch (result) {
				case GL_INVALID_ENUM: err = "Invalid enum"; break;
				case GL_INVALID_VALUE: err = "Invalid value"; break;
				case GL_INVALID_OPERATION: err = "Invalid operation"; break;
				case GL_STACK_OVERFLOW: err = "Stack overflow"; break;
				case GL_STACK_UNDERFLOW: err = "Stack undeflow"; break;
				case GL_OUT_OF_MEMORY: err = "Out of memory"; break;
				default: break;
			}
			Log::Report(0, file, function, line, Log::TYPE_WARNING, "GL error caught with error code %x (%s)", result, err);
		}
		return false;
	} else
		error_line = -1;
	return true;
}

/*----------------------------------------------------------------------------*/

static void bonobo::check_bound_shader_program(const bonobo::ShaderProgram &sp, const char *file, const char *function, i32 line)
{
	GLint curr;
	glGetIntegerv(GL_CURRENT_PROGRAM, &curr);
	if (sp.mId != curr)
		Log::Report(0, file, function, line, Log::TYPE_WARNING, "You must bind the shader program using glUseProgram before calling this function");
}


/*----------------------------------------------------------------------------*/
static std::string bonobo::getStringForType(GLenum type)
{
  switch (type)
  {
    case GL_DEBUG_TYPE_ERROR:
         return"Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
         return "Deprecated Behavior";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
         return "Undefined Behavior";
    case GL_DEBUG_TYPE_PORTABILITY:
         return "Portability Issue";
    case GL_DEBUG_TYPE_PERFORMANCE:
         return "Performance Issue";
    case GL_DEBUG_TYPE_MARKER:
         return "Stream Annotation";
    case GL_DEBUG_TYPE_PUSH_GROUP:
         return "Push Group";
    case GL_DEBUG_TYPE_POP_GROUP:
         return "Pop Group";
    case GL_DEBUG_TYPE_OTHER:
         return "Other";
    default:
         assert(false);
         return "";
  }
}

static std::string bonobo::getStringForSource(GLenum source)
{
  switch (source)
  {
    case GL_DEBUG_SOURCE_API:
         return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
         return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
         return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
         return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION:
         return "Application";
    case GL_DEBUG_SOURCE_OTHER:
         return "Other";
    default:
         assert(false);
         return "";
  }
}

static std::string bonobo::getStringForSeverity(GLenum severity)
{
  switch (severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:
         return "High";
    case GL_DEBUG_SEVERITY_MEDIUM:
         return "Medium";
    case GL_DEBUG_SEVERITY_LOW:
         return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
         return "Notification";
    default:
         assert(false);
         return("");
  }
}

static Log::Type bonobo::getTypeForSeverity(GLenum severity)
{
  switch (severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:
         return Log::TYPE_ERROR;
    case GL_DEBUG_SEVERITY_MEDIUM:
         return Log::TYPE_WARNING;
    case GL_DEBUG_SEVERITY_LOW:
         return Log::TYPE_INFO;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
         return Log::TYPE_TRIVIA;
    default:
         assert(false);
         return Log::TYPE_TRIVIA;
  }
}

void APIENTRY bonobo::openglErrorCallback( GLenum source, GLenum type, GLuint id
                             , GLenum severity, GLsizei /*length*/
                             , GLchar const* msg, void const* /*data*/
                             )
{
    Log::Type severityType = bonobo::getTypeForSeverity(severity);
    bool const logOnce = severityType == Log::TYPE_TRIVIA;

    switch (id) {
    case 131218: // NVIDIA "Program/shader state performance warning: Fragment Shader is going to be recompiled because the shader key based on GL state mismatches."
        severityType = Log::Type::TYPE_INFO;
        break;
    default:
        break;
    }

    Log::Report(logOnce ? LOG_MESSAGE_ONCE_FLAG : 0, "", "", 0, severityType, "OpenGL Debug Message [%s, from %s](Id: %u):\n\t%s",
		  bonobo::getStringForType(type).c_str(), bonobo::getStringForSource(source).c_str(), id, msg);
}

void bonobo::debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
#if DEBUG_LEVEL <= 2
	if (severity == GL_DEBUG_SEVERITY_LOW)
		return;
#endif

	// Downgrade severity
    switch(id) {
		case 131218: // NVIDIA "Program/shader state performance warning: Fragment Shader is going to be recompiled because the shader key based on GL state mismatches."
			severity = GL_DEBUG_SEVERITY_LOW;
			break;
		default:
			break;
    }


	Log::Type ls;
	bool logOnce = false;
	switch (severity) {
#if DEBUG_LEVEL <= 2
		case GL_DEBUG_SEVERITY_HIGH: ls = Log::TYPE_INFO; break;
		case GL_DEBUG_SEVERITY_MEDIUM: ls = Log::TYPE_TRIVIA; break;
#elif DEBUG_LEVEL == 3
		case GL_DEBUG_SEVERITY_HIGH: ls = Log::TYPE_ERROR; break;
		case GL_DEBUG_SEVERITY_MEDIUM: ls = Log::TYPE_WARNING; break;
		case GL_DEBUG_SEVERITY_LOW: ls = Log::TYPE_INFO; break;
#endif
		default: ls = Log::TYPE_TRIVIA; logOnce = true; break;
	}
	char *buf;
	switch (type) {
		case GL_DEBUG_TYPE_ERROR: buf = "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: buf = "Deprecated behavior"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: buf = "Undefined behavior"; break;
		case GL_DEBUG_TYPE_PORTABILITY: buf = "Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: buf = "Performance"; break;
		case GL_DEBUG_TYPE_MARKER: buf = "Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: buf = "Push group"; break;
		case GL_DEBUG_TYPE_POP_GROUP: buf = "Pop group"; break;
		default: buf = "Other"; break;
	}
	Log::Report(logOnce ? LOG_MESSAGE_ONCE_FLAG : 0, "", "", 0, ls, "GL debug message (Reason: %s) (Source: %X) (Id: %u):\n\t%s", buf, source, id, message);
}

/*----------------------------------------------------------------------------*/

void bonobo::init()
{
	if (bonobo::initiated)
		return;

	char *glver = (char *) glGetString(GL_VERSION);
	LogInfo("Using GL version: %s\n", glver);
	char *versep = strchr(glver, '.');
	bonobo::majorVersion = (int) versep[-1] - '0';
	bonobo::minorVersion = (int) versep[1] - '0';
	glver = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
	LogInfo("Shading language version is: %s\n", glver);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	GL_CHECK_ERRORS();

	bonobo::Stats.n_vbos = 0;
	bonobo::Stats.n_ibos = 0;
	bonobo::Stats.n_rbos = 0;
	bonobo::Stats.n_fbos = 0;
	bonobo::Stats.n_tbos = 0;
	bonobo::Stats.n_samplers = 0;
	bonobo::Stats.n_textures = 0;
	bonobo::Stats.n_cubemaps = 0;
	bonobo::Stats.n_shaders = 0;
	bonobo::Stats.n_shaderprogs = 0;

	bonobo::initiated = true;
}

/*----------------------------------------------------------------------------*/

bool bonobo::checkForErrors()
{
	return check_errors("??", "??", 0);
}

/*----------------------------------------------------------------------------*/

static bool buildCode(std::vector<std::string> &src_out, const std::string &src_in, unsigned int depth)
{
	bool ok = Param(depth < 256);
	if (!ok)
		return false;

	size_t pos = 0;
	size_t totalLen = src_in.length();
	std::string token;
	if ((pos = src_in.find("#pragma include(\"")) != std::string::npos) {
		size_t epos;
		if ((epos = src_in.find("\")")) == std::string::npos) {
			LogWarning("Couldn't parse include pragma - no closing \" found");
			return false;
		}
		pos += 17;
		std::string includeFilename = src_in.substr(pos, epos - pos);
		std::ifstream in;
		in = std::ifstream(includeFilename, std::ios::in | std::ios::binary);
		if (!in) {
			// Lazily check \shaders as well
			in = std::ifstream("shaders\\" + includeFilename, std::ios::in | std::ios::binary);
			if (!in) {
				LogWarning("Couldn't open include file \"%s\"", includeFilename.c_str());
				return false;
			}
		}
		std::string text;
		in.seekg(0, std::ios::end);
		text.resize((size_t) in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&text[0], text.size());
		in.close();

		std::vector<std::string> next_in;

		pos -= 17;

		// Write out what we have so far
		if (pos > 0)
			src_out.push_back(src_in.substr(0, pos));
		// Insert include file to be processed next
		next_in.push_back(text);
		// Insert the rest of the file to be processed after the included file
		if (totalLen - (epos + 2) > 0)
			next_in.push_back(src_in.substr(epos + 2, totalLen - (epos + 2))); // + 2 to get past "\")"
		// Recursively call
		for (auto it = next_in.begin(); it != next_in.end(); ++it) {
			if (!buildCode(src_out, *it, depth + 1))
				return false;
		}
	} else if (totalLen > 0)
		src_out.push_back(src_in);

	return true;
}

/*----------------------------------------------------------------------------*/

bonobo::Shader *bonobo::loadShader(const std::vector<std::string> &code, bonobo::ShaderType type, const char *identifier)
{
	GLint id = 0, status;
	/* Compile shader */
	id = glCreateShader(type);

	std::vector<std::string> outCode;
	// Find and split #pragma include("file.ext")
	bool buildSuccess = true;
	for (auto it = code.begin(); it != code.end() && buildSuccess; ++it)
		buildSuccess |= buildCode(outCode, *it, 0);

	if (!buildSuccess) {
		LogWarning("Failed to assemble shader code for %s. Check include structure", identifier ? identifier : "??");
		return nullptr;
	}

	const char **strPtrs = new const char*[outCode.size()];
	int i = 0;
	for (auto it = outCode.begin(); it != outCode.end(); ++it, i++)
		strPtrs[i] = it->c_str();
	if (buildSuccess) {
		glShaderSource(id, i, strPtrs, NULL);
		GL_CHECK_ERRORS();
	}

	glCompileShader(id);
	GL_CHECK_ERRORS();
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	GL_CHECK_ERRORS();

	if (status == GL_FALSE) {
		GLsizei length;
		char result[ERR_BUF_LEN];
		glGetShaderInfoLog(id, ERR_BUF_LEN, &length, result);
		glDeleteShader(id);
		if (length)
			Log(result);
        LogWarning("Shader compilation unsuccessful for shader %s", identifier ? identifier : "??");
		Log("=====");
        return nullptr;
	} else {
		GLsizei length;
		char result[ERR_BUF_LEN];
		glGetShaderInfoLog(id, ERR_BUF_LEN, &length, result);
		if (length)
			Log("Shader compiler message for %s: %s", identifier ? identifier : "??", bonobo::shaderString[(i32) type], result);
	}
	bonobo::Stats.n_shaders++;

	bonobo::Shader *handle = new bonobo::Shader();
	handle->mType = type;
	handle->mId = id;

	return handle;
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadShader(bonobo::Shader *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_shaders--;
	Assert(bonobo::Stats.n_shaders >= 0);
	glDeleteShader(handle->mId);
	//delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

bonobo::ShaderProgram *bonobo::loadShaderProgram(const std::vector<bonobo::Shader *> &shaders, const std::vector<std::string> *varyings, const char *identifier)
{
	bonobo::ShaderProgram *handle = new bonobo::ShaderProgram();
	for (unsigned int i = 0; i < N_SHADER_TYPES; i++)
		handle->mHasShader[i] = false;

	// Populate shader program struct with shaders
	for (auto it = shaders.begin(); it != shaders.end(); ++it) {
		bonobo::Shader *s = *it;
		unsigned int idx = N_SHADER_TYPES + 1;
		for (unsigned int i = 0; i < N_SHADER_TYPES; i++) {
			if (s->mType == bonobo::shaderType[i]) {
				idx = i;
				break;
			}
		}
		if (idx >= N_SHADER_TYPES)
			LogWarning("Shader type not found");
		handle->mShaders[idx] = *s;
		handle->mHasShader[idx] = true;
	}

	// Create and link program
	handle->mId = glCreateProgram();
	GL_CHECK_ERRORS();
	for (unsigned int i = 0; i < N_SHADER_TYPES; i++) {
		if (!handle->mHasShader[i])
			continue;
		glAttachShader(handle->mId, handle->mShaders[i].mId);
	}
	GL_CHECK_ERRORS();

	if (varyings && varyings->size() > 0) {
		const GLchar **v = new const GLchar*[varyings->size()];
		int i = 0;
		for (auto it = varyings->begin(); it != varyings->end(); ++it, i++)
			v[i] = it->c_str();
		glTransformFeedbackVaryings(handle->mId, (unsigned int) varyings->size(), v, GL_INTERLEAVED_ATTRIBS); // TODO: Expose attrib mode?
	}

	glLinkProgram(handle->mId);
	GL_CHECK_ERRORS();

	// Check status of linking
	GLint status;
	glGetProgramiv(handle->mId, GL_LINK_STATUS, &status);
	GL_CHECK_ERRORS();

	if (status == GL_FALSE) {
		GLsizei length;
		char result[ERR_BUF_LEN];
		glGetProgramInfoLog(handle->mId, ERR_BUF_LEN, &length, result);
		GL_CHECK_ERRORS();
		glDeleteProgram(handle->mId);
		for (i32 i = 0; i < 5; i++) {
			if (!handle->mHasShader[i])
				continue;
			glDeleteShader(handle->mShaders[i].mId);
		}	
		if (length) {
			Log(result);
			LogWarning("Couldn't link program %s", identifier ? identifier : "??");
			Log("=====");
			delete handle;
			return nullptr;
		}
	} else {
		GLsizei length;
		char result[ERR_BUF_LEN];
        glGetProgramInfoLog(handle->mId, ERR_BUF_LEN, &length, result);
		GL_CHECK_ERRORS();
		if (length)
			Log("Shader linker for %s: %s", identifier ? identifier : "??", result);
	}

	GL_CHECK_ERRORS();

	char buf[1024];
	GLsizei len;
	GLint size;
	GLenum type;

	// Create attribute map
	GLint nAttribs;
	glGetProgramiv(handle->mId, GL_ACTIVE_ATTRIBUTES, &nAttribs);
	GL_CHECK_ERRORS();
	for (int i = 0; i < nAttribs; i++) {
		glGetActiveAttrib(handle->mId, i, 1023, &len, &size, &type, buf);
		GL_CHECK_ERRORS();
		bonobo::Location atr;
		atr.mName = std::string(buf);
		atr.mLoc = glGetAttribLocation(handle->mId, buf);
		GL_CHECK_ERRORS();
		Assert(atr.mLoc != -1);
		atr.mSize = size; // TODO: Currently unused
		atr.mType = type;
		handle->mAttribLocs[atr.mName] = atr;
	}

	// Create uniform map
	GLint nUniforms;
	glGetProgramiv(handle->mId, GL_ACTIVE_UNIFORMS, &nUniforms);
	GL_CHECK_ERRORS();
	for (int i = 0; i < nUniforms; i++) {
		glGetActiveUniform(handle->mId, i, 1023, &len, &size, &type, buf);
		GL_CHECK_ERRORS();
		bonobo::Location un;
		un.mName = std::string(buf);
		un.mLoc = glGetUniformLocation(handle->mId, buf);
		GL_CHECK_ERRORS();
		Assert(un.mLoc != -1);
		un.mSize = size; // TODO: Currently unused
		un.mType = type;
		un.mIsSet = false;
		handle->mUniformLocs[un.mName] = un;
	}

	// TODO: Continue implementing this...
#if 0

	// Create uniform block map
	int maxBlockEntries = 16;
	int *blockEntries = (int *) malloc(maxBlockEntries * sizeof(int));
	int *blockOffset = (int *) malloc(maxBlockEntries * sizeof(int));
	GLint nUniformBlocks;
	glGetProgramiv(handle->mId, GL_ACTIVE_UNIFORM_BLOCKS, &nUniformBlocks);
	GL_CHECK_ERRORS();
	for (int i = 0; i < nUniformBlocks; i++) {
		bonobo::UniformBlock ublock;
		int dataSize, nEntries;
		glGetActiveUniformBlockiv(handle->mId, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
		glGetActiveUniformBlockiv(handle->mId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &nEntries);
		if (nEntries > maxBlockEntries) {
			blockEntries = (int *) realloc(blockEntries, nEntries * sizeof(int));
			blockOffset = (int *) realloc(blockOffset, nEntries * sizeof(int));
			maxBlockEntries = nEntries;
		}

		glGetActiveUniformBlockiv(handle->mId, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, blockEntries);
		glGetActiveUniformsiv(handle->mId, nEntries, (GLuint *) blockEntries, GL_UNIFORM_TYPE, blockOffset);
		for (int k = 0; k < nEntries; k++) {
			glGetActiveUniform(handle->mId, blockEntries[i], 1023, &len, &size, &type, buf);
			ublock.mDesc.SetEntry(// TODO: Convert GL-types
		}

		GL_CHECK_ERRORS();
		bonobo::Location un;
		un.mName = std::string(buf);
		un.mLoc = glGetUniformLocation(handle->mId, buf);
		GL_CHECK_ERRORS();
		Assert(un.mLoc != -1);
		un.mSize = size; // TODO: Currently unused
		un.mType = type;
		un.mIsSet = false;
		handle->mUniformLocs[un.mName] = un;
	}
	free(blockEntries);
	free(blockOffset);
#endif

	GL_CHECK_ERRORS();
	// Success!
	bonobo::Stats.n_shaderprogs++;
	return handle;
}

/*----------------------------------------------------------------------------*/

bonobo::ShaderProgram *bonobo::loadShaderProgram(const std::vector<std::string> &shaderFilenames, const std::string *prefix, const std::vector<std::string> *varyings)
{
	std::vector<bonobo::Shader *> shaders;
	bool shaderFound[N_SHADER_TYPES] = {false};
	for (auto it = shaderFilenames.begin(); it != shaderFilenames.end(); ++it) {
		std::vector<std::string> srcs_in;
		std::ostringstream os;
		if (prefix)
			srcs_in.push_back(*prefix);
		os << "#pragma include(\"" << (*it) << "\")";
		srcs_in.push_back(os.str());

		// Some old code to extract shader type
		const char *ext = strrchr(it->c_str(), '.');
		unsigned int s = 0;
		for (; s < N_SHADER_TYPES; s++) {
			if (strcmp(bonobo::filenameSuffix[s], ext) == 0)
				break;
		}
		if (shaderFound[s]) {
			LogWarning("Shader type already loaded, ignoring");
			continue;
		}

		// Try to load shader
		bonobo::Shader *shader;
		if (!(shader = bonobo::loadShader(srcs_in, shaderType[s], it->c_str()))) {
			GL_CHECK_ERRORS();
			for (auto it = shaders.begin(); it != shaders.end(); ++it)
				bonobo::unloadShader(*it);
			return nullptr;
		}
		shaders.push_back(shader);
		shaderFound[s] = true;
	}

	return bonobo::loadShaderProgram(shaders, varyings);
}

/*----------------------------------------------------------------------------*/

bonobo::ShaderProgram *bonobo::loadShaderProgram(const std::string shaderFilenames[], unsigned int nFiles, const std::string *prefix, const std::vector<std::string> *varyings)
{
	if (nFiles > N_SHADER_TYPES)
		LogError("Too many shaders.");
	std::vector<std::string> fn;
	for (unsigned int i = 0; i < nFiles && !shaderFilenames[i].empty(); i++)
		fn.push_back(shaderFilenames[i]);
	return bonobo::loadShaderProgram(fn, prefix, varyings);
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadShaderProgram(bonobo::ShaderProgram *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_shaderprogs--;
	Assert(bonobo::Stats.n_shaderprogs >= 0);
	glDeleteProgram(handle->mId);
	delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

bonobo::VAO *bonobo::loadVertexAttributeObject(const bonobo::ShaderProgram &sp, const bonobo::VBO &vbo, const StructDesc &desc, unsigned int slot)
{
	return bonobo::loadVertexAttributeObject(sp, vbo, &desc, 1, &slot);
}

/*----------------------------------------------------------------------------*/

bonobo::VAO *bonobo::loadVertexAttributeObject(const bonobo::ShaderProgram &sp, const bonobo::VBO &vbo, const StructDesc *desc, unsigned int n, unsigned int *slots)
{
	//GL_CHECK_SHADER_BOUND(sp); // TODO: Needed?
	bonobo::VAO *vao = new bonobo::VAO;
	glGenVertexArrays(1, &vao->mId);

	glBindVertexArray(vao->mId);

	glBindBuffer(GL_ARRAY_BUFFER, vbo.mId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	unsigned int nBoundAttributes = 0;

	for (unsigned int k = 0; k < n; k++) {
		unsigned int i = 0;
		for (auto it = desc[k].mEntryList.begin(); it != desc[k].mEntryList.end(); ++it, i++) {
			auto locptr = sp.mAttribLocs.find(it->mName);
			if (locptr == sp.mAttribLocs.end())
				continue;
			nBoundAttributes++;

			glEnableVertexAttribArray(locptr->second.mLoc);
			GLboolean normalized = it->mType == types::TYPE_I8 || it->mType == types::TYPE_U8 ||
				it->mType == types::TYPE_I16 || it->mType == types::TYPE_U16 ? GL_TRUE : GL_FALSE; // TODO: Hacky, but should be correct most of the time :)
                        bonobo::DataType type = bonobo::Type(it->mType);
			glVertexAttribPointer(locptr->second.mLoc, it->mComponents, type, normalized, desc[k].mStride, reinterpret_cast<void const*>(it->mOffset));
			GL_CHECK_ERRORS();
			//glVertexAttribFormat(locptr->second.mLoc, it->mComponents, type, normalized, it->mOffset);
			//GL_CHECK_ERRORS();
			//glVertexAttribBinding(locptr->second.mLoc, slots ? slots[k] : k);
			//GL_CHECK_ERRORS();
		}
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	bonobo::Stats.n_vaos++;

	return vao;
}

/*----------------------------------------------------------------------------*/

bonobo::VBO *bonobo::loadVertexBufferObject(const void *new_array, unsigned int stride, unsigned int n, bonobo::Usage usage)
{
	bool ok = Param(stride > 0) &&
		Param(n > 0);
	if (!ok)
		return nullptr; // TODO: Error message

	GLuint vbo;
	glGenBuffers(1, &vbo);
	GL_CHECK_ERRORS();
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GL_CHECK_ERRORS();
	glBufferData(GL_ARRAY_BUFFER, stride * n, new_array, (GLuint) usage);
	GL_CHECK_ERRORS();

	bonobo::VBO *handle = new bonobo::VBO();
	handle->mId = vbo;
	handle->mUsage = (GLuint) usage;
	handle->mStride = stride;
	handle->mN = n;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL_CHECK_ERRORS();
	bonobo::Stats.n_vbos++;
	return handle;
}

/*----------------------------------------------------------------------------*/

void bonobo::bindVertexBufferObject(const bonobo::VBO &vbo, unsigned int /*slot*/, unsigned int /*offset*/)
{
	//glBindVertexBuffer(slot, vbo.mId, offset, vbo.mStride);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.mId);
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

// TODO: Use mapped buffer
void bonobo::replaceVertexBufferObjectData(bonobo::VBO &handle, void *new_array, unsigned int n)
{
	glBindBuffer(GL_ARRAY_BUFFER, handle.mId);
	GL_CHECK_ERRORS();
	glBufferSubData(GL_ARRAY_BUFFER, 0, handle.mStride * n, new_array);
	GL_CHECK_ERRORS();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL_CHECK_ERRORS();
	handle.mN = n;
}

/*----------------------------------------------------------------------------*/

#include "GLTextureFormats.inl"

const TextureFormatPair *getTextureFormat(GLenum &componentPrecision, GLenum &componentFormat, bonobo::TextureType type, vec4i componentBits)
{
	// TODO: Old code follows. Make sure it does what it should
	// TODO: Include closest match
	const TextureFormatPair *formatMap = glTextureFormatMap;
	while (formatMap->type != bonobo::TEXTURE_TYPE_LAST) {
		if (formatMap->type != type) {
			formatMap++;
			continue;
		}
		if (formatMap->componentBits == componentBits)
			break;
		formatMap++;
	}

	if (formatMap->type == bonobo::TEXTURE_TYPE_LAST)
		LogWarning("Couldn't find matching GL-type. Missing from GLTextureFormats.inl?");

	// HACK! Generalize pixel format, and use separate setup-struct
	//GLenum componentFormat = setup.type == bonobo::TEXTURE_FLOAT_DEPTH ? GL_DEPTH_COMPONENT : GL_RGB;
	if (type == bonobo::TEXTURE_FLOAT_DEPTH) {
		componentFormat = GL_DEPTH_COMPONENT;
		//componentFormat = GL_DEPTH_COMPONENT32; // TODO: Test
		//componentFormat = GL_DEPTH_COMPONENT32F_NV;
		componentPrecision = GL_FLOAT;
		return formatMap;
	} else {
		switch (formatMap->components) {
			case 1: componentFormat = GL_RED; break;
			case 2: componentFormat = GL_RG; break;
			default: case 3: componentFormat = GL_RGB; break;
			case 4: componentFormat = GL_RGBA; break;
		}
	}

	if (type == bonobo::TEXTURE_FLOAT) {
		componentPrecision = GL_FLOAT;
	} else {
		i32 roundedPrecision = formatMap->componentBits.r / 8;
		switch (roundedPrecision) {
			default: case 1: componentPrecision = GL_UNSIGNED_BYTE; break;
			case 2: componentPrecision = GL_UNSIGNED_SHORT; break;
			case 4: componentPrecision = GL_UNSIGNED_INT; break;
		}
	}

	return formatMap;
}

/*----------------------------------------------------------------------------*/

bonobo::Texture *bonobo::loadTexture(const u8 *data, u32 width, u32 height, u32 depth, u32 layers, u32 msaa, u32 mipLevels, bonobo::TextureType type, vec4i componentBits)
{
	bonobo::Texture *handle = new bonobo::Texture();
	glGenTextures(1, &handle->mId);
	GL_CHECK_ERRORS();

	handle->mWidth = width;
	handle->mHeight = height;
	handle->mDepth = depth;
	handle->mLayers = layers;
	handle->mMultisamples = msaa;
	handle->mMipLevels = mipLevels;

	Assert(width > 0);
	if (height == 0) {
		Assert(depth == 0);
		Assert(msaa == 0);
		if (layers > 0)
			handle->mTarget = bonobo::TEXTURE_1D_ARRAY;
		else
			handle->mTarget = bonobo::TEXTURE_1D;
	} else if (depth == 0) {
		if (layers > 0) {
			if (msaa > 1) {
				Assert(mipLevels == 0);
				handle->mTarget = bonobo::TEXTURE_2D_MS_ARRAY;
			} else
				handle->mTarget = bonobo::TEXTURE_2D_ARRAY;
		} else {
			if (msaa > 1) {
				Assert(mipLevels == 0);
				handle->mTarget = bonobo::TEXTURE_2D_MS;
			} else
				handle->mTarget = bonobo::TEXTURE_2D;
		}
	} else {
		Assert(layers == 0);
		Assert(msaa == 0);
		handle->mTarget = bonobo::TEXTURE_3D;
	}

	GLenum componentPrecision;
	GLenum componentFormat;
	const TextureFormatPair *formatMap = getTextureFormat(componentPrecision, componentFormat, type, componentBits);

	glBindTexture(handle->mTarget, handle->mId);
	GL_CHECK_ERRORS();

	glTexParameteri(handle->mTarget, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(handle->mTarget, GL_TEXTURE_MAX_LEVEL, mipLevels);
	handle->mFormat = componentFormat;
	handle->mInternalFormat = formatMap->iformat;
	handle->mPrecision = componentPrecision;

	if (!bonobo::uploadTexture(*handle, data)) {
		bonobo::unloadTexture(handle);
		delete handle;
		return nullptr;
	}

	if (mipLevels > 0)
		glGenerateMipmap(handle->mTarget);

	glBindTexture(handle->mTarget, 0);
	GL_CHECK_ERRORS();

	bonobo::Stats.n_textures++;
	return handle;
}

/*----------------------------------------------------------------------------*/

bonobo::Texture *bonobo::loadTexture1D(const u8 *pixels, u32 width, bonobo::TextureType type, vec4i componentBits, u32 mipLevels)
{
	return bonobo::loadTexture(pixels, width, 0, 0, 0, 0, mipLevels, type, componentBits);
}

bonobo::Texture *bonobo::loadTexture1D(u32 width, bonobo::TextureType type, vec4i componentBits)
{
	return bonobo::loadTexture(nullptr, width, 0, 0, 0, 0, 0, type, componentBits);
}

bonobo::Texture *bonobo::loadTexture2D(const u8 *pixels, u32 width, u32 height, bonobo::TextureType type, vec4i componentBits, u32 mipLevels)
{
	return bonobo::loadTexture(pixels, width, height, 0, 0, 0, mipLevels, type, componentBits);
}

bonobo::Texture *bonobo::loadTexture2D(u32 width, u32 height, bonobo::TextureType type, vec4i componentBits, u32 msaa)
{
	return bonobo::loadTexture(nullptr, width, height, 0, 0, msaa, msaa <= 1 ? 1000 : 0, type, componentBits);
}

bonobo::Texture *bonobo::loadTexture2D(std::string filename, u32 mipLevels)
{
	const char *ext = strrchr(filename.c_str(), '.');
	if (strcmp(".png", ext) != 0) {
		LogWarning("Only .png extension is supported for now");
		return nullptr;
	}
	std::vector<unsigned char> image;
	u32 w, h;
	if (lodepng::decode(image, w, h, filename.c_str()) != 0) {
		char buf[1024];
		sprintf(buf, "res/%s", filename.c_str());
		if (lodepng::decode(image, w, h, buf) != 0) {
			LogWarning("Couldn't load or decode image file %s", filename.c_str());
			return nullptr;
		}
	}
	u8 *flipBuffer = new u8[w * h * 4];
	for (u32 y = 0; y < h; y++)
		memcpy(flipBuffer + (h - 1 - y) * w * 4, &image[y * w * 4], w * 4);
	bonobo::Texture *tex = bonobo::loadTexture(flipBuffer, w, h, 0, 0, 0, mipLevels, bonobo::TEXTURE_UNORM, vec4i::Make(8, 8, 8, 8));
	delete flipBuffer;
	return tex;
}

bonobo::Texture *bonobo::loadTexture3D(const u8 *voxels, u32 width, u32 height, u32 depth, bonobo::TextureType type, vec4i componentBits, u32 mipLevels)
{
	return bonobo::loadTexture(voxels, width, height, depth, 0, 0, mipLevels, type, componentBits);
}

bonobo::Texture *bonobo::loadTexture2DArray(const u8 *pixels, u32 width, u32 height, u32 depth, bonobo::TextureType type, vec4i componentBits, u32 mipLevels)
{
	return bonobo::loadTexture(pixels, width, height, 0, depth, 0, mipLevels, type, componentBits);
}

/*----------------------------------------------------------------------------*/

bool bonobo::uploadTexture(const bonobo::Texture &texture, const u8 *source, int mipLevel)
{
	glBindTexture(texture.mTarget, texture.mId);
	GL_CHECK_ERRORS();

	switch (texture.mTarget) {
		case bonobo::TEXTURE_2D_MS_ARRAY:
			glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, texture.mMultisamples, texture.mInternalFormat, texture.mWidth, texture.mHeight, texture.mLayers, GL_TRUE); 
			break;
		case bonobo::TEXTURE_2D_MS:
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, texture.mMultisamples, texture.mInternalFormat, texture.mWidth, texture.mHeight, GL_TRUE);
			break;
		case bonobo::TEXTURE_3D:
			glTexImage3D(GL_TEXTURE_3D, mipLevel, texture.mInternalFormat, texture.mWidth, texture.mHeight, texture.mDepth, 0, texture.mFormat, texture.mPrecision, source);
			break;
		case bonobo::TEXTURE_2D:
			glTexImage2D(GL_TEXTURE_2D, mipLevel, texture.mInternalFormat, texture.mWidth, texture.mHeight, 0, texture.mFormat, texture.mPrecision, source);
			break;
		case bonobo::TEXTURE_1D:
			glTexImage1D(GL_TEXTURE_1D, mipLevel, texture.mInternalFormat, texture.mWidth, 0, texture.mFormat, texture.mPrecision, source);
			break;
		case bonobo::TEXTURE_2D_ARRAY:
			glTexImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, texture.mInternalFormat, texture.mWidth, texture.mHeight, texture.mLayers, 0, texture.mFormat, texture.mPrecision, source);
			break;
		case bonobo::TEXTURE_1D_ARRAY:
			glTexImage2D(GL_TEXTURE_1D_ARRAY, mipLevel, texture.mInternalFormat, texture.mWidth, texture.mHeight, 0, texture.mFormat, texture.mPrecision, source);
			break;
		default:
			LogWarning("Texture format not found");
			return false;
	}
	return true;
}

/*----------------------------------------------------------------------------*/

void bonobo::readTexture(void *target, const bonobo::Texture &texture, int mipLevel)
{
	if (texture.mTarget == bonobo::TEXTURE_2D_MS || texture.mTarget == bonobo::TEXTURE_2D_MS_ARRAY) {
		LogLocOnce(Log::TYPE_WARNING, "Reading multisampled textures is not yet supported"); // LOWPRIO
		return;
	}
	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
	glBindTexture(texture.mTarget, texture.mId);
	glGetTexImage(texture.mTarget, mipLevel, texture.mFormat, texture.mPrecision, target);
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadTexture(bonobo::Texture *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_textures--;
	Assert(bonobo::Stats.n_textures >= 0);
	glDeleteTextures(1, &handle->mId);
	delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

bonobo::Sampler *bonobo::loadSampler(bonobo::TextureWrap wrapping, bonobo::TextureFilter filter)
{
	bonobo::Sampler *handle = new bonobo::Sampler();
	glGenSamplers(1, &handle->mId);
	GL_CHECK_ERRORS();

	if (filter == bonobo::TEXTURE_BEST) {
		glSamplerParameteri(handle->mId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(handle->mId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glSamplerParameteri(handle->mId, GL_TEXTURE_MIN_FILTER, filter);
		glSamplerParameteri(handle->mId, GL_TEXTURE_MAG_FILTER, filter);
	}
	GL_CHECK_ERRORS();
	
	glSamplerParameteri(handle->mId, GL_TEXTURE_WRAP_S, wrapping);
	glSamplerParameteri(handle->mId, GL_TEXTURE_WRAP_T, wrapping);
	glSamplerParameteri(handle->mId, GL_TEXTURE_WRAP_R, wrapping);
	GL_CHECK_ERRORS();

	bonobo::Stats.n_samplers++;
	return handle;
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadSampler(bonobo::Sampler *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_samplers--;
	Assert(bonobo::Stats.n_samplers >= 0);
	glDeleteSamplers(1, &handle->mId);
	delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

bonobo::TBO *bonobo::loadTextureBufferObject(const u8 *data, unsigned int bytes, bonobo::TextureType type, vec4i componentBits, bonobo::Usage usage)
{
	bonobo::TBO *handle = new bonobo::TBO();
	
	GLenum componentPrecision;
	GLenum componentFormat;
	const TextureFormatPair *formatMap = getTextureFormat(componentPrecision, componentFormat, type, componentBits);

	GLuint tbo, tex;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, bytes, data, usage);
	
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	glTexBuffer(GL_TEXTURE_BUFFER, formatMap->iformat, tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	GL_CHECK_ERRORS();

	handle->mTboId = tbo;
	handle->mTextureId = tex;

	bonobo::Stats.n_tbos++;
	return handle;
}

/*----------------------------------------------------------------------------*/

bonobo::FBO *bonobo::loadFrameBufferObject(const std::vector<const bonobo::Texture *> *colorAttachments, const bonobo::Texture *depthAttachment,	
									 const bonobo::Texture *stencilAttachment, const bonobo::Texture *depthStencilAttachment)
{
	// TODO: Needed?
	//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	GL_CHECK_ERRORS();
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GL_CHECK_ERRORS();

	if (colorAttachments) {
		unsigned int i = 0;
		for (auto it = colorAttachments->begin(); it != colorAttachments->end(); ++it, i++) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, (*it)->mTarget, (*it)->mId, 0);
			GL_CHECK_ERRORS();
			GLint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (result != GL_FRAMEBUFFER_COMPLETE) {
				LogWarning("Failed to create frame buffer object");
				// TODO: Destroy?
				return nullptr;
			}
		}
	}
	if (depthAttachment) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, (GLenum) depthAttachment->mTarget, depthAttachment->mId, 0);
		GL_CHECK_ERRORS();
		GLint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE) {
			LogWarning("Failed to create frame buffer object");
			// TODO: Destroy?
			return nullptr;
		}
	}
	if (stencilAttachment) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, (GLenum) stencilAttachment->mTarget, stencilAttachment->mId, 0);
		GL_CHECK_ERRORS();
		GLint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE) {
			LogWarning("Failed to create frame buffer object");
			// TODO: Destroy?
			return nullptr;
		}
	}
	if (depthStencilAttachment) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, (GLenum) depthStencilAttachment->mTarget, depthStencilAttachment->mId, 0);
		GL_CHECK_ERRORS();
		GLint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE) {
			LogWarning("Failed to create frame buffer object");
			// TODO: Destroy?
			return nullptr;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_CHECK_ERRORS();

	bonobo::Stats.n_fbos++;
	bonobo::FBO *handle = new bonobo::FBO();
	handle->mId = fbo;
	handle->mColorAttachments = colorAttachments ? new std::vector<const bonobo::Texture *>(*colorAttachments) : nullptr;
	handle->mDepthAttachment = depthAttachment;
	handle->mStencilAttachment = stencilAttachment;
	handle->mDepthStencilAttachment = depthStencilAttachment;
	return handle;
}

/*----------------------------------------------------------------------------*/

bonobo::FBO *bonobo::loadFrameBufferObject(const bonobo::Texture **colorAttachments, unsigned int n, const bonobo::Texture *depthAttachment,	
									 const bonobo::Texture *stencilAttachment, const bonobo::Texture *depthStencilAttachment)
{
	std::vector<const bonobo::Texture *> ca;
	for (unsigned int i = 0; i < n; i++)
		ca.push_back(colorAttachments[i]);
	return bonobo::loadFrameBufferObject(&ca, depthAttachment, stencilAttachment, depthStencilAttachment);
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadFrameBufferObject(bonobo::FBO *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_fbos--;
	Assert(bonobo::Stats.n_fbos >= 0);
	glDeleteFramebuffers(1, &handle->mId);
	if (handle->mColorAttachments)
		delete handle->mColorAttachments;
	delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

bonobo::RBO *bonobo::loadRenderBufferObject(bonobo::RBO_Target target, unsigned int w, unsigned int h, int msaa, const bonobo::FBO *fbo)
{
	GLuint rbo;

	if (fbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->mId);

	glGenRenderbuffers(1, &rbo);
	GL_CHECK_ERRORS();

	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	GL_CHECK_ERRORS();

	GLint r;
	switch (target) {
		default:
		case RBO_TARGET_DEPTH_32F:
		case RBO_TARGET_DEPTH_24:
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, target == RBO_TARGET_DEPTH_32F ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT24, w, h); // TODO: w and h from fbo
			r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			GL_CHECK_ERRORS();
			if (fbo) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
			r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			break;
		case RBO_TARGET_STENCIL:
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_STENCIL_INDEX, w, h);
			GL_CHECK_ERRORS();
			if (fbo) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
			break;
		case RBO_TARGET_DEPTH_STENCIL_24_8:
		case RBO_TARGET_DEPTH_STENCIL_32F_8:
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, target == RBO_TARGET_DEPTH_STENCIL_32F_8 ? GL_DEPTH32F_STENCIL8 : GL_DEPTH24_STENCIL8, w, h);
			GL_CHECK_ERRORS();
			if (fbo) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
			break;
	}
	GL_CHECK_ERRORS();

	GLint result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (result != GL_FRAMEBUFFER_COMPLETE) {
		LogWarning("Failed to attach render buffer object");
		// TODO: Destroy?
		return nullptr;
	}
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (fbo)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_CHECK_ERRORS();

	bonobo::Stats.n_rbos++;

	bonobo::RBO *handle = new bonobo::RBO();
	handle->mId = rbo;
	return handle;
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadRenderBufferObject(bonobo::RBO *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_rbos--;
	Assert(bonobo::Stats.n_rbos >= 0);
	glDeleteRenderbuffers(1, &handle->mId);
	delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

void bonobo::setRenderTarget(const bonobo::FBO *fbo, const bonobo::RBO *rbo)
{
	GLenum buf[32]; // LOWPRIO: Big enough?
	GLenum result;

	if (rbo) {
		glBindRenderbuffer(GL_RENDERBUFFER, rbo->mId);
		GL_CHECK_ERRORS();
	} else
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->mId);
		GL_CHECK_ERRORS();
		if (fbo->mColorAttachments) {
			unsigned int i = 0;
			for (auto it = fbo->mColorAttachments->begin(); it != fbo->mColorAttachments->end(); ++it, i++)
				buf[i] = GL_COLOR_ATTACHMENT0 + i;
			glDrawBuffers((int) fbo->mColorAttachments->size(), buf);
		}
	} else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (fbo || rbo) {
		GL_CHECK_ERRORS();
		result = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (result != GL_FRAMEBUFFER_COMPLETE)
			LogWarning("Failed to bind FBO/RBO");
	}
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

bonobo::IBO *bonobo::loadIndexBufferObject(const void *new_array, unsigned int type_size, unsigned int n, bonobo::Usage usage)
{
	GLuint ibo;

	glGenBuffers(1, &ibo);
	GL_CHECK_ERRORS();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	GL_CHECK_ERRORS();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * type_size, new_array, (GLuint) usage);
	GL_CHECK_ERRORS();

	bonobo::IBO *handle = new bonobo::IBO();
	handle->mId = ibo;
	handle->mUsage = (GLuint) usage;
	handle->mN = n;
	switch (type_size) {
		case 1: handle->mTypesize = GL_UNSIGNED_BYTE; break;
		case 2: handle->mTypesize = GL_UNSIGNED_SHORT; break;
		case 4: handle->mTypesize = GL_UNSIGNED_INT; break;
		default: LogWarning("Index array type size must be 1, 2 or 4"); break;
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GL_CHECK_ERRORS();

	bonobo::Stats.n_ibos++;
	return handle;
}

/*----------------------------------------------------------------------------*/

void bonobo::unloadIndexBufferObject(bonobo::IBO *handle)
{
	if (!handle) {
		LogLocOnce(Log::TYPE_WARNING, "Null pointer handle");
		return;
	}
	bonobo::Stats.n_ibos--;
	Assert(bonobo::Stats.n_ibos >= 0);
	glDeleteBuffers(1, &handle->mId);
	delete handle;
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

void bonobo::bindTexture(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, const bonobo::Texture &handle)
{
	GL_CHECK_SHADER_BOUND(sp);
	auto match = sp.mUniformLocs.find(samplerName);
	if (match == sp.mUniformLocs.end()) {
		LogMsgOnce(Log::TYPE_INFO, "Shader program doesn't have a sampler \"%s\"", samplerName.c_str());
		return;
	}
	match->second.mIsSet = true;
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(handle.mTarget, handle.mId);
	glUniform1i(match->second.mLoc, slot);
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

// TODO: Remove
void bonobo::bindTexture(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, GLuint id)
{
	GL_CHECK_SHADER_BOUND(sp);
	auto match = sp.mUniformLocs.find(samplerName);
	if (match == sp.mUniformLocs.end()) {
		LogMsgOnce(Log::TYPE_INFO, "Shader program doesn't have a sampler \"%s\"", samplerName.c_str());
		return;
	}
	match->second.mIsSet = true; // TODO: Debug only
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, id);
	glUniform1i(match->second.mLoc, slot);
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

void bonobo::bindTextureBufferObject(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, const bonobo::TBO &handle)
{
	GL_CHECK_SHADER_BOUND(sp);
	auto match = sp.mUniformLocs.find(samplerName);
	if (match == sp.mUniformLocs.end()) {
		LogMsgOnce(Log::TYPE_INFO, "Shader program doesn't have a sampler \"%s\"", samplerName.c_str());
		return;
	}
	match->second.mIsSet = true;
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_BUFFER, handle.mTextureId);
	glUniform1i(match->second.mLoc, slot);
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

void bonobo::bindTextureSampler(bonobo::ShaderProgram &sp, std::string samplerName, unsigned int slot, const bonobo::Texture &texhandle, const bonobo::Sampler &smphandle)
{
	GL_CHECK_SHADER_BOUND(sp);
	auto match = sp.mUniformLocs.find(samplerName);
	if (match == sp.mUniformLocs.end()) {
		LogMsgOnce(Log::TYPE_INFO, "Shader program doesn't have a sampler \"%s\"", samplerName.c_str());
		return;
	}
	match->second.mIsSet = true;
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(texhandle.mTarget, texhandle.mId);
	glUniform1i(match->second.mLoc, slot);
	glBindSampler(slot, smphandle.mId);
	GL_CHECK_ERRORS();
}

/*----------------------------------------------------------------------------*/

void bonobo::debugLogUnsetUniforms(const bonobo::ShaderProgram &sp)
{
	GL_CHECK_SHADER_BOUND(sp);
	GL_CHECK_ERRORS();

	bool allSet = true;
	for (auto it = sp.mUniformLocs.begin(); it != sp.mUniformLocs.end(); ++it) {
		if (!it->second.mIsSet) {
			LogMsgOnce(Log::TYPE_WARNING, "Uniform %s for shader program id %u does not appear to be set", it->second.mName.c_str(), sp.mId);
			allSet = false;
		}
	}
	if (allSet)
		LogMsgOnce(Log::TYPE_INFO, "All uniforms for shader program id %u appear to be set", sp.mId);
}

/*----------------------------------------------------------------------------*/

std::unordered_map<const bonobo::ShaderProgram *, bonobo::VAO *> glb_fullShaderVAOMap;
bonobo::VBO *glb_fullVBO = nullptr;

void bonobo::drawFullscreen(const bonobo::ShaderProgram &sp)
{
	GL_CHECK_SHADER_BOUND(sp);
	if (glb_fullVBO == nullptr) {
		f32 glb_fullVtx[] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};
		glb_fullVBO = bonobo::loadVertexBufferObject(glb_fullVtx, 2 * sizeof(f32), 3, USAGE_STATIC);
	}

	bonobo::VAO *vao = nullptr;
	auto vao_ref = glb_fullShaderVAOMap.find(&sp);
	if (vao_ref == glb_fullShaderVAOMap.end()) {
		StructDescEntry fullEntries[] = {
			DescEntry<vec2f>("Vertex"),
		};
		StructDesc fullSd = StructDesc(fullEntries, 1);
		vao = bonobo::loadVertexAttributeObject(sp, *glb_fullVBO, fullSd);
		glb_fullShaderVAOMap[&sp] = vao;
	} else
		vao = vao_ref->second;

	glBindVertexArray(vao->mId);
	bonobo::bindVertexBufferObject(*glb_fullVBO, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	bonobo::checkForErrors();
	glBindVertexArray(0);
}

static bool bonobo::getShaderProgramUniformLocation(GLuint &dst, bonobo::ShaderProgram &sp, const std::string &name)
{
    GL_CHECK_SHADER_BOUND(sp);
    // TODO: Add type and size checks
	auto match = sp.mUniformLocs.find(name);
	if (match == sp.mUniformLocs.end())
		return false;
	match->second.mIsSet = true;
	dst = match->second.mLoc;
	return true;
}

#define UNIFORM_UPLOAD_SETUP()			GL_CHECK_SHADER_BOUND(sp); \
											GLuint loc; \
											if (!bonobo::getShaderProgramUniformLocation(loc, sp, name)) { \
												LogMsgOnce(Log::TYPE_INFO, "Uniform %s not found in shader", name.c_str()); \
												return false; \
											}
#define UNIFORM_UPLOAD_1(t, f)			template<> bool bonobo::setUniform<t>(bonobo::ShaderProgram &sp, const std::string &name, const t &v) { \
												UNIFORM_UPLOAD_SETUP() \
												f(loc, v); GL_CHECK_ERRORS(); return true; }
#define UNIFORM_UPLOAD_2(t, f)			template<> bool bonobo::setUniform<t>(bonobo::ShaderProgram &sp, const std::string &name, const t &v) { \
												UNIFORM_UPLOAD_SETUP() \
												f(loc, v[0], v[1]); GL_CHECK_ERRORS(); return true; }
#define UNIFORM_UPLOAD_3(t, f)			template<> bool bonobo::setUniform<t>(bonobo::ShaderProgram &sp, const std::string &name, const t &v) { \
												UNIFORM_UPLOAD_SETUP() \
												f(loc, v[0], v[1], v[2]); GL_CHECK_ERRORS(); return true; }
#define UNIFORM_UPLOAD_4(t, f)			template<> bool bonobo::setUniform<t>(bonobo::ShaderProgram &sp, const std::string &name, const t &v) { \
												UNIFORM_UPLOAD_SETUP() \
												f(loc, v[0], v[1], v[2], v[3]); GL_CHECK_ERRORS(); return true; }
#define UNIFORM_UPLOAD_A(t, gt, f)		template<> bool bonobo::setUniformArray<t>(bonobo::ShaderProgram &sp, const std::string &name, const t *v, int n) { \
												UNIFORM_UPLOAD_SETUP() \
												f(loc, (GLsizei) n, (gt *) v); GL_CHECK_ERRORS(); return true; }
#define UNIFORM_UPLOAD_MTX(t, f)		template<> bool bonobo::setUniform<t>(bonobo::ShaderProgram &sp, const std::string &name, const t &v) { \
												UNIFORM_UPLOAD_SETUP() \
												f(loc, (GLsizei) 1, GL_FALSE, (GLfloat *) &(v.M[0][0])); GL_CHECK_ERRORS(); return true; }

UNIFORM_UPLOAD_1(unsigned int, glUniform1ui)
UNIFORM_UPLOAD_2(vec2u, glUniform2i)
UNIFORM_UPLOAD_3(vec3u, glUniform3i)
UNIFORM_UPLOAD_4(vec4u, glUniform4i)
UNIFORM_UPLOAD_1(int, glUniform1i)
UNIFORM_UPLOAD_2(vec2i, glUniform2i)
UNIFORM_UPLOAD_3(vec3i, glUniform3i)
UNIFORM_UPLOAD_4(vec4i, glUniform4i)
UNIFORM_UPLOAD_1(float, glUniform1f)
UNIFORM_UPLOAD_2(vec2f, glUniform2f)
UNIFORM_UPLOAD_3(vec3f, glUniform3f)
UNIFORM_UPLOAD_4(vec4f, glUniform4f)
UNIFORM_UPLOAD_A(unsigned int, GLuint, glUniform1uiv)
UNIFORM_UPLOAD_A(vec2u, GLuint, glUniform2uiv)
UNIFORM_UPLOAD_A(vec3u, GLuint, glUniform3uiv)
UNIFORM_UPLOAD_A(vec4u, GLuint, glUniform4uiv)
UNIFORM_UPLOAD_A(int, GLint, glUniform1iv)
UNIFORM_UPLOAD_A(vec2i, GLint, glUniform2iv)
UNIFORM_UPLOAD_A(vec3i, GLint, glUniform3iv)
UNIFORM_UPLOAD_A(vec4i, GLint, glUniform4iv)
UNIFORM_UPLOAD_A(float, GLfloat, glUniform1fv)
UNIFORM_UPLOAD_A(vec2f, GLfloat, glUniform2fv)
UNIFORM_UPLOAD_A(vec3f, GLfloat, glUniform3fv)
UNIFORM_UPLOAD_A(vec4f, GLfloat, glUniform4fv)
UNIFORM_UPLOAD_MTX(mat2f, glUniformMatrix2fv)
UNIFORM_UPLOAD_MTX(mat3f, glUniformMatrix3fv)
UNIFORM_UPLOAD_MTX(mat4f, glUniformMatrix4fv)

