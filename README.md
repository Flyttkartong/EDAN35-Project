# EDAN35-Project
####EDAN35 Terrain Generation Project Using the Bonobo Framework Provided by LTH

We have implemented a version of the Marching Cubes algorithm described in GPU Gems 3, chapter 1: http://http.developer.nvidia.com/GPUGems3/gpugems3_ch01.html

The author of this chapter has a webpage where the Look Up tables can be downloaded: 
http://www.geisswerks.com/about_terrain.html

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

####Note:
In Visual Studio, the shaders are located in "Resource Files/Shaders/Assignment/", whereas the source code can be found in "Source Files/Project".

A minor change has been made in bonobo::loadFrameBufferObject() in GLB.cpp, where glFramebufferTexture2D was replaced by glFramebufferTexture in order for it to work with our code.
