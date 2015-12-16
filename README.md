# EDAN35-Project
EDAN35 Terrain Generation Project Using the Bonobo Framework Provided by LTH

Files relevant to the project are the following:

####src/
* Main.cpp
* Terrain.cpp
* Terrain.h

####shaders/
* glslWater.vert
* glslWater.frag
* terrain.vert
* terrain.geo
* terrain.frag

Note: A minor change has been made in bonobo::loadFrameBufferObject() in GLB.cpp, where glFramebufferTexture2D was replaced by glFramebufferTexture in order for it to work with our code.
