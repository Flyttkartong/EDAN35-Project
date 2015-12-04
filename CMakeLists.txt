# As we use the VERSION option---which was introduced in CMAKE 3.0---of the
# project() command, set the minimum required version to 3.0.
CMAKE_MINIMUM_REQUIRED (VERSION 3.0)


# Set the project name, versions and languages used.
PROJECT (
  EDAN_35_Lab2
  VERSION 1.0.0.0
  LANGUAGES CXX C
)


# To be able to dowload and compile the dependencies
INCLUDE (ExternalProject)


# Set C++ flags
IF (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
  # clang and GCC do not default to C++11 by default, but MSVC seems to (and I
  # couldn't find the proper flag to enable C++11 on MSVC anyway).
  ADD_COMPILE_OPTIONS (--std=c++11)
ENDIF ()
#IF (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
#  ADD_COMPILE_OPTIONS (-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-documentation -fno-limit-debug-info -Wno-gnu-zero-variadic-macro-arguments -Wno-attributes)
#ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#  ADD_COMPILE_OPTIONS (-Wall -Wextra -Wno-attributes)
#ENDIF ()


# Set build type
SET (CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
IF (NOT CMAKE_BUILD_TYPE)
  SET (
    CMAKE_BUILD_TYPE
    "Release"
    CACHE STRING
    "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}."
    FORCE
  )
ENDIF (NOT CMAKE_BUILD_TYPE)
SET_PROPERTY (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})


# OpenGL is used for the rendering
FIND_PACKAGE (OpenGL REQUIRED)


IF (NOT (DEFINED USE_SYSTEM_GLFW))
  SET (USE_SYSTEM_GLFW TRUE)
ENDIF ()
#IF (NOT (DEFINED USE_SYSTEM_GLM))
#  SET (USE_SYSTEM_GLM TRUE)
#ENDIF ()


# GLFW is used for inputs and windows handling
FIND_PACKAGE (glfw NAMES glfw glfw3)
IF (NOT glfw_FOUND)
  MESSAGE (STATUS "Ignore the previous message about glfw, we will use our own.")
  EXTERNALPROJECT_ADD (
    glfw
    PREFIX "glfw"
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG 3.1.2
    BINARY_DIR "glfw/build"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_SHARED_LIBS=ON
  )
  SET (USE_SYSTEM_GLFW FALSE)
  SET (glfw_FOUND TRUE CACHE BOOL "Whether glfw was found")
  SET (GLFW3_INCLUDE_DIR "${CMAKE_BINARY_DIR}/glfw/include" CACHE STRING "glfw include dir")
  IF (CMAKE_HOST_WIN32)
    SET (GLFW3_LIBRARY "${CMAKE_BINARY_DIR}/glfw/lib/glfw3.dll" CACHE STRING "glfw lib path")
    SET (GLFW3_static_LIBRARY "${CMAKE_BINARY_DIR}/glfw/lib/glfw3dll.lib" CACHE STRING "glfw lib path")
  ELSEIF (CMAKE_HOST_APPLE)
    SET (GLFW3_LIBRARY "${CMAKE_BINARY_DIR}/glfw/lib/libglfw.dylib" CACHE STRING "glfw lib path")
  ELSE ()
    SET (GLFW3_LIBRARY "${CMAKE_BINARY_DIR}/glfw/lib/libglfw.so" CACHE STRING "glfw lib path")
  ENDIF ()
ENDIF ()


## GLM is used for matrices, vectors and camera handling
#FIND_PACKAGE (glm)
#IF (NOT glm_FOUND)
#  MESSAGE (STATUS "Ignore the previous message about glm, we will use our own.")
#  EXTERNALPROJECT_ADD (
#    glm
#    PREFIX "glm"
#    GIT_REPOSITORY "https://github.com/g-truc/glm"
#    GIT_TAG 0.9.7.1
#    BUILD_COMMAND ""
#    INSTALL_COMMAND ""
#  )
#  SET (USE_SYSTEM_GLM FALSE)
#  SET (glm_FOUND TRUE CACHE BOOL "Whether glm was found")
#  SET (GLM_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/glm/src/glm" CACHE STRING "glm include dir")
#ENDIF ()


# ImGui is used for the windowing system inside the application
EXTERNALPROJECT_ADD (
  imgui
  PREFIX "imgui"
  GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
  GIT_TAG v1.46
  BINARY_DIR "imgui/build"
  UPDATE_COMMAND ${CMAKE_COMMAND} -E copy
      ${PROJECT_SOURCE_DIR}/imgui_CMakeLists.txt
      ${CMAKE_BINARY_DIR}/imgui/src/imgui/CMakeLists.txt
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
SET (IMGUI_INCLUDE_DIR "${CMAKE_BINARY_DIR}/imgui/include")
IF (CMAKE_HOST_WIN32)
  SET (IMGUI_static_LIBRARY "${CMAKE_BINARY_DIR}/imgui/lib/imguidll.lib")
  SET (IMGUI_LIBRARY "${CMAKE_BINARY_DIR}/imgui/lib/imgui.dll")
ELSEIF (CMAKE_HOST_APPLE)
  SET (IMGUI_LIBRARY "${CMAKE_BINARY_DIR}/imgui/lib/libimgui.dylib")
ELSE ()
  SET (IMGUI_LIBRARY "${CMAKE_BINARY_DIR}/imgui/lib/libimgui.so")
ENDIF ()


SET (USE_SYSTEM_GLFW ${USE_SYSTEM_GLFW} CACHE BOOL "Whether to use the system version of GLFW")
#SET (USE_SYSTEM_GLM ${USE_SYSTEM_GLM} CACHE BOOL "Whether to use the system version of GLFW")


# Configure *C++ Environment Variables*
SET (SHADERS_DIR "${PROJECT_SOURCE_DIR}/shaders")
SET (RESOURCES_DIR "${PROJECT_SOURCE_DIR}/res")
CONFIGURE_FILE ("${PROJECT_SOURCE_DIR}/src/config.hpp.in" "${PROJECT_BINARY_DIR}/config.hpp")


# Define groups of files for IDEs
SET (PATH_SEP \\)
SOURCE_GROUP (
  Shaders${PATH_SEP}SimpleDraw
  
  FILES
  ${SHADERS_DIR}/ArrayToMs.vert
  ${SHADERS_DIR}/ArrayToMs.frag
  ${SHADERS_DIR}/SimpleTextured.vert
  ${SHADERS_DIR}/SimpleTextured.frag
  ${SHADERS_DIR}/SimpleTexturedArray.frag
  ${SHADERS_DIR}/SimpleTexturedMS.frag
)
SOURCE_GROUP (
  Shaders
  
  FILES
  ${SHADERS_DIR}/fill_gbuffer.vert
  ${SHADERS_DIR}/fill_gbuffer.frag
  ${SHADERS_DIR}/fill_shadowmap.vert
  ${SHADERS_DIR}/fill_shadowmap.frag
  ${SHADERS_DIR}/accumulate_lights.vert
  ${SHADERS_DIR}/accumulate_lights.frag
  ${SHADERS_DIR}/resolve_deferred.vert
  ${SHADERS_DIR}/resolve_deferred.frag
  ${SHADERS_DIR}/Simple.vert
  ${SHADERS_DIR}/Simple.frag
)
SOURCE_GROUP (Source\ Files FILES ${PROJECT_SOURCE_DIR}/src/Deferred.cpp)
SOURCE_GROUP (Source\ Files${PATH_SEP}External${PATH_SEP}lodepng FILES ${PROJECT_SOURCE_DIR}/src/lodepng.cpp ${PROJECT_SOURCE_DIR}/src/lodepng.h)
SOURCE_GROUP (Source\ Files${PATH_SEP}External${PATH_SEP}GLFW FILES ${PROJECT_SOURCE_DIR}/src/imgui_impl_glfw_gl3.cpp ${PROJECT_SOURCE_DIR}/src/imgui_impl_glfw_gl3.h)
SOURCE_GROUP (Source\ Files${PATH_SEP}External${PATH_SEP}GL3W FILES ${PROJECT_SOURCE_DIR}/src/gl3w.cpp)


# Include directories
#INCLUDE_DIRECTORIES (${OPENGL_INCLUDE_DIR})
INCLUDE_DIRECTORIES (${GLFW3_INCLUDE_DIR})
#INCLUDE_DIRECTORIES (${GLM_INCLUDE_DIRS})
INCLUDE_DIRECTORIES (${IMGUI_INCLUDE_DIR})
INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES (${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES ("${PROJECT_SOURCE_DIR}/inc")


# Compile the project and link the libraries
SET (
  CPP_SOURCES

  src/Bonobo.cpp
  src/Deferred.cpp
  src/GLB.cpp
  src/GLBdecl.cpp
  src/GLStateInspection.cpp
  src/GLStateInspectionView.cpp
  src/InputHandler.cpp
  src/lodepng.cpp
  src/Log.cpp
  src/LogView.cpp
  src/Main.cpp
  src/Misc.cpp
  src/ObjReader.cpp
  src/Profiler.cpp
  src/ProfilerView.cpp
  src/RenderState.cpp
  src/SimpleDraw.cpp
  src/SimpleText.cpp
  src/StructDesc.cpp
  src/Types.cpp
  src/VectorMath.cpp
  src/Window.cpp

  src/gl3w.cpp
  src/imgui_impl_glfw_gl3.cpp
)

ADD_EXECUTABLE (${PROJECT_NAME} ${CPP_SOURCES})

IF (NOT USE_SYSTEM_GLFW)
  ADD_DEPENDENCIES (${PROJECT_NAME} glfw)
ENDIF ()
#IF (NOT USE_SYSTEM_GLM)
#  ADD_DEPENDENCIES (${PROJECT_NAME} glm)
#ENDIF ()
ADD_DEPENDENCIES (${PROJECT_NAME} imgui)

IF (WIN32)
  FILE (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
  FILE (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
  ADD_CUSTOM_COMMAND (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${GLFW3_LIBRARY} ${CMAKE_BINARY_DIR}/Debug)
  ADD_CUSTOM_COMMAND (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${GLFW3_LIBRARY} ${CMAKE_BINARY_DIR}/Release)
ENDIF ()


IF (APPLE)
  FIND_LIBRARY (CORE_FOUNDATION_LIBRARY CoreFoundation)
  SET (EXTRA_LIBS ${CORE_FOUNDATION_LIBRARY})
ELSEIF (UNIX)
  SET (EXTRA_LIBS dl)
ENDIF ()


IF (UNIX)
  TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${OPENGL_gl_LIBRARY} ${GLFW3_LIBRARY} ${IMGUI_LIBRARY} ${EXTRA_LIBS})
ELSE ()
  TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${OPENGL_gl_LIBRARY} ${GLFW3_static_LIBRARY} ${IMGUI_static_LIBRARY})
ENDIF ()
