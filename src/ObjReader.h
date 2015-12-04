#pragma once

#include "VectorMath.h"
#include "StructDesc.h"
#include <string>
#include <iostream>
#include <istream>
#include <unordered_map>

class OBJ {
public:
	struct Vertex {
		Vertex() : mNormal(v3f(0.0f, 0.0f, 0.0f)), mTexcoords(v2f(0.0f, 0.0f)) {}
		vec3f mPosition;
		vec3f mNormal;
        vec3f mTangent;
        vec3f mBinormal;
		vec2f mTexcoords;
	};

	struct Triangle {
		unsigned int mIdx[3];
	};

	struct Group {
		std::string mName;
		std::string mMaterial;
		unsigned int mStart, mEnd;
	};

	struct Mesh {
		std::vector<Group> mGroups;
		std::vector<Triangle> mTriangles;
		std::vector<Vertex> mVertices;
		StructDesc mVertexDesc;
		std::string mMaterialFileName;
	};

	struct Material {
		std::string mName;
		vec3f mAmbientColor, mDiffuseColor, mSpecularColor;
		std::string mAmbientTexture, mDiffuseTexture, mSpecularTexture, mBumpTexture;
		f32 mShininess, mSharpness, mTransparency, mReflectivity, mRefractionIndex;
	};

	struct MaterialMap {
		std::unordered_map<std::string, Material> mMap;
	};

public:
	static MaterialMap *ReadMTL(std::istream &input);
	static MaterialMap *ReadMTLFile(std::string filename);

	static Mesh *ReadOBJ(std::istream &input);
	static Mesh *ReadOBJFile(std::string filename);
	static Mesh *ReadCachedOBJFile(std::string filename);

};

