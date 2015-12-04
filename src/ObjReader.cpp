#include <functional>
#include <vector>
#include <sstream>
#include <fstream>
#include "StructDesc.h"

#include "Log.h"

#include "Misc.h"
#include "ObjReader.h"

struct IVertex {
	unsigned int pos;
	unsigned int norm;
	unsigned int tex;
	bool operator==(const IVertex &other) const {
		return pos == other.pos && norm == other.norm && tex == other.tex;
	}
};

struct IVertexHash
{
	std::size_t operator()(const IVertex &i) const {
		return i.pos ^
			((i.norm << 17) | (i.norm >> 15)) ^
			((i.tex << 11)  | (i.tex << 21));
	}
};

OBJ::MaterialMap *OBJ::ReadMTLFile(std::string filename)
{
	std::ifstream ifs = std::ifstream(filename, std::ios::binary);
	if (!ifs.good() || !ifs.is_open()) {
		LogWarning("Couldn't open .mtl file %s", filename.c_str());
		return nullptr;
	}
	OBJ::MaterialMap *mm = OBJ::ReadMTL(ifs);
	ifs.close();
	return mm;
}

OBJ::MaterialMap *OBJ::ReadMTL(std::istream &input)
{
	bool ok = Param(!input.bad());
	if (!ok)
		return nullptr;
	
	char line[8192];
	int lineNbr = 0;

	OBJ::MaterialMap *map = new OBJ::MaterialMap();

	OBJ::Material *m = nullptr;
	std::string type;
	std::string name;

	//while (input.getline(line, 8192 - 1).eof()) {
	for (;;) {
		input.getline(line, 8192 - 1);
		std::stringstream ls(line);
		ls >> type;

		if (type == "#") {
			// do nothing
		}
		else if (type == "newmtl") {
			OBJ::Material mat;
			ls >> name;
			m = &(map->mMap[name] = mat);
			m->mName = name;
			// Haphazard default values
			m->mAmbientColor = vec3f::Make(0.0f, 0.0f, 0.0f);
			m->mDiffuseColor = vec3f::Make(1.0f, 1.0f, 1.0f);
			m->mSpecularColor = 0.2f * vec3f::Make(1.0f, 1.0f, 1.0f);
			m->mAmbientTexture = "";
			m->mDiffuseTexture = "";
			m->mSpecularTexture = "";
			m->mBumpTexture = "";
			m->mShininess = 10.0f;
			m->mSharpness = 1.0f;
			m->mTransparency = 0.0f;
			m->mReflectivity = 0.0f;
			m->mRefractionIndex = 1.0f;
		}
		else if (type == "Ka")
			ls >> m->mAmbientColor.r >> m->mAmbientColor.g >> m->mAmbientColor.b;
		else if (type == "Kd")
			ls >> m->mDiffuseColor.r >> m->mDiffuseColor.g >> m->mDiffuseColor.b;
		else if (type == "Ks")
			ls >> m->mSpecularColor.r >> m->mSpecularColor.g >> m->mSpecularColor.b;
		else if (type == "map_Ka")
			ls >> m->mAmbientTexture;
		else if (type == "map_Kd")
			ls >> m->mDiffuseTexture;
		else if (type == "map_Ks")
			ls >> m->mSpecularTexture;
		else if ((type == "map_bump" || type == "bump") && m->mBumpTexture == "")
			ls >> m->mBumpTexture;
		else if (type == "Ns")
			ls >> m->mShininess;
		else if (type == "sharpness")
			ls >> m->mSharpness;
		else if (type == "d")
			ls >> m->mTransparency;
		else if (type == "r")
			ls >> m->mReflectivity;
		else if (type == "Ni")
			ls >> m->mRefractionIndex;

		if (input.eof())
			break;
	}

	return map;
}

OBJ::Mesh *OBJ::ReadOBJFile(std::string filename)
{
	std::ifstream ifs = std::ifstream(filename, std::ios::binary);
	if (!ifs.good() || !ifs.is_open()) {
		LogError("Couldn't open .obj file %s", filename.c_str());
		return nullptr;
	}
	OBJ::Mesh *mesh = OBJ::ReadOBJ(ifs);
	ifs.close();
	return mesh;
}

OBJ::Mesh *OBJ::ReadOBJ(std::istream &input)
{
	bool ok = Param(!input.bad());
	if (!ok)
		return nullptr;

	auto startTime = StartTimer();

	OBJ::Mesh *mesh = new OBJ::Mesh();
	mesh->mMaterialFileName = "";

	std::vector<vec3f> vPos;
    std::vector<vec3f> vNorm;
    std::vector<vec3f> vTangent;
    std::vector<vec3f> vBinormal;
	std::vector<vec2f> vTex;
	std::unordered_map<IVertex, unsigned int, IVertexHash> vhash;
	std::vector<IVertex> indices;

	vPos.reserve(16384);
    vNorm.reserve(16384);
    vTangent.reserve(16384);
    vBinormal.reserve(16384);
	vTex.reserve(16384);
	vPos.push_back(vec3f::Make(0.0f, 0.0f, 0.0f));
    vNorm.push_back(vec3f::Make(1.0f, 0.0f, 0.0f));
    vTangent.push_back(vec3f::Make(0.0f, 0.0f, 1.0f));
    vBinormal.push_back(vec3f::Make(0.0f, 1.0f, 0.0f));
	vTex.push_back(vec2f::Make(0.0f, 0.0f));

	bool mtlFound = false;
	bool newGroup = true;
	std::string groupName = "UnnamedGroup";
	std::string materialName = "";
	unsigned int nFaces = 0;
	unsigned int nPolys = 0;
	unsigned int lineNbr = 0;
	std::string type;
	unsigned int index = 0;
	unsigned int start = 0;

	unsigned int duplicates = 0;

	bool sync_state = std::ios_base::sync_with_stdio(false);

	char line[8192];
	for (;;) {
		input.getline(line, 8192 - 1);
		std::stringstream ls(line);
		ls >> type;
		if (type == "f") {
			IVertex vtx[3];
			memset(vtx, 0, sizeof(IVertex) * 3);
			unsigned int n = 0, m = 0;
			nPolys++;
			for (;;) {
				std::string word;
				ls >> word;
				if (word == "")
					break;
				std::stringstream ws(word);
				ws >> vtx[n].pos;
				if (ws.peek() != '/')
					goto incr;
				ws.get();
				if (ws.peek() == '/') {
					ws.get();
					ws >> vtx[n].norm;
					goto incr;
				}
				ws >> vtx[n].tex;
				if (ws.peek() == '/') {
					ws.get();
					ws >> vtx[n].norm;
				}
				incr:
				n++;
				m++;
				if (n == 3) {
					Triangle tri;
					for (unsigned int k = 0; k < 3; k++) {

						//auto it = vhash.find(vtx[k]);
						//if (it == vhash.end()) {
							vhash[vtx[k]] = index;
							tri.mIdx[k] = index++;
							indices.push_back(vtx[k]);
						//} else {
						//	tri.idx[k] = it->second;
						//	duplicates++;
						//}
					}
					mesh->mTriangles.push_back(tri);
					vtx[1] = vtx[2];
					n = 2;
				}
				newGroup = false;
				if (ls.eof())
					break;
			}
			if (n < 2)
				LogLocOnce(Log::TYPE_INFO, "Too few vertices found in OBJ file (%u). Error on line ~= %u", n, lineNbr);
		} else if (type == "v") {
			vec3f pos;
			ls >> pos.x >> pos.y >> pos.z;
			vPos.push_back(pos);
		} else if (type == "vn") {
			vec3f nrm;
			ls >> nrm.x >> nrm.y >> nrm.z;
			vNorm.push_back(nrm);
		} else if (type == "vt") {
			vec2f tex;
			ls >> tex.x >> tex.y;
			vTex.push_back(tex);
		} else if (type == "g" || type == "usemtl") {
			if (!newGroup) {
				Group g;
				g.mStart = start;
				g.mEnd = (unsigned int) mesh->mTriangles.size();
				g.mName = groupName;
				g.mMaterial = materialName;
				mesh->mGroups.push_back(g);
				start = g.mEnd;
			}
			if (type == "g")
				ls >> groupName;
			else
				ls >> materialName;
			newGroup = true;
		} else if (type == "mtllib")
			ls >> mesh->mMaterialFileName;

		lineNbr++;
		if (input.eof())
			break;
	}
	std::ios_base::sync_with_stdio(sync_state);


	//LogTrivia("Found %u duplicate indices in .obj-file", duplicates);
	
	if (!newGroup) {
		Group g;
		g.mStart = start;
		g.mEnd = (unsigned int) mesh->mTriangles.size();
		g.mName = groupName;
		g.mMaterial = materialName;
		mesh->mGroups.push_back(g);
    }

    // Generate binormal and tangent
    for (u32 i = 0; i < vNorm.size(); ++i) {
        vBinormal.push_back(v3f(0.0f, 0.0f, 0.0f));
        vTangent.push_back(v3f(0.0f, 0.0f, 0.0f));
    }
    for (auto const& t : mesh->mTriangles) {
        vec2f t1 = vTex[indices[t.mIdx[1]].tex] - vTex[indices[t.mIdx[0]].tex];
        vec2f t2 = vTex[indices[t.mIdx[2]].tex] - vTex[indices[t.mIdx[0]].tex];
        vec3f e1 = vPos[indices[t.mIdx[1]].pos] - vPos[indices[t.mIdx[0]].pos];
        vec3f e2 = vPos[indices[t.mIdx[2]].pos] - vPos[indices[t.mIdx[0]].pos];

        vec3f normal = cross(e1, e2);
        f32 len = length(normal);
        f32 area = len * 0.5f;

        vec3f tangent, binormal;

        f32 c = 1.0f / (t1.u * t2.v - t1.v * t2.u);
        tangent.x = (t2.v * e1.x - t1.v * e2.x) * c;
        tangent.y = (t2.v * e1.y - t1.v * e2.y) * c;
        tangent.z = (t2.v * e1.z - t1.v * e2.z) * c;
        binormal.x = (t2.u * e1.x - t1.u * e2.x) * c;
        binormal.y = (t2.u * e1.y - t1.u * e2.y) * c;
        binormal.z = (t2.u * e1.z - t1.u * e2.z) * c;

        normalize(tangent);
        normalize(binormal);

        vec3f wbn = area * binormal;
        vec3f wtn = area * tangent;

        for (int j = 0; j < 3; ++j) {
            i32 ni = indices[t.mIdx[j]].norm;
            vBinormal[ni] += wbn;
            vTangent[ni] += wtn;
        }
    }
    for (u32 i = 0; i < vNorm.size(); ++i) {
        normalize(vBinormal[i]);
        normalize(vTangent[i]);
    }

	OBJ::Vertex vtx;
	for (auto it = indices.begin(); it != indices.end(); ++it) {
		IVertex v = *it;
		vtx.mPosition = vPos[v.pos];
        vtx.mNormal = vNorm[v.norm];
        vtx.mTangent = vTangent[v.norm];
        vtx.mBinormal = vBinormal[v.norm];
		vtx.mTexcoords = vTex[v.tex];
		mesh->mVertices.push_back(vtx);
	}

	LogTrivia("Found %u triangles in .obj-file", mesh->mTriangles.size());

	/*
	OBJ::Vertex vtx;
	for (auto it = vPos.begin(); it != vPos.end(); ++it) {
		vtx.position = *it;
		mesh->vertices.push_back(vtx);
	}
	auto it = mesh->vertices.begin();
	for (auto nit = vNorm.begin(); nit != vNorm.end(); ++nit, ++it)
		it->normal = *nit;
	it = mesh->vertices.begin();
	for (auto nit = vTex.begin(); nit != vTex.end(); ++nit, ++it)
		it->texcoords = *nit;
	*/

	StructDescEntry entries[] = {
		DescEntry<vec3f>("Vertex"),
        DescEntry<vec3f>("Normal"),
        DescEntry<vec3f>("Tangent"),
        DescEntry<vec3f>("Binormal"),
		DescEntry<vec2f>("Texcoord")
	};
	mesh->mVertexDesc.SetEntries(entries, 5);

	LogTrivia("Loading mesh took %u ms", EndTimerMilliseconds(startTime));

	return mesh;
}

OBJ::Mesh *OBJ::ReadCachedOBJFile(std::string filename)
{
	std::ostringstream oss;
	oss << filename << ".cached";

	bool doCache = false;

	std::string fn = std::string(oss.str());
	std::ifstream ifs = std::ifstream(fn, std::ios::binary);
	if (!ifs.is_open()) {
		OBJ::Mesh *mesh = ReadOBJFile(filename);
		if (mesh == nullptr)
			return nullptr;
		std::ofstream ofs = std::ofstream(oss.str(), std::ios::binary);
		if (!ofs.good() || !ofs.is_open()) {
			LogInfo("Couldn't cache .obj file %s", filename.c_str());
			return mesh;
		}
		// Cache obj file
		WriteData(ofs, mesh->mVertexDesc, (u8 *) &mesh->mVertices[0], mesh->mVertices.size());
		StructDescEntry idxEntries[] = {DescEntry<vec3i>("Index")};
		StructDesc idxDesc(idxEntries, 1);
		WriteData(ofs, idxDesc, (u8 *) &mesh->mTriangles[0], mesh->mTriangles.size());
		ofs << (mesh->mMaterialFileName.empty() ? 0 : 1) << " ";
		if (!mesh->mMaterialFileName.empty())
			ofs << mesh->mMaterialFileName << " ";
		ofs << mesh->mGroups.size() << " ";
		for (auto it : mesh->mGroups) {
			ofs << it.mName << " ";
			ofs << (it.mMaterial.empty() ? 0 : 1) << " ";
			if (!it.mMaterial.empty())
				ofs << it.mMaterial << " ";
			ofs << it.mStart << " ";
			ofs << it.mEnd << " ";
		}
		ofs.close();
		LogTrivia("Cached .obj file %s", filename.c_str());
		return mesh;
	}
	auto startTime = StartTimer();
	OBJ::Mesh *mesh = new OBJ::Mesh();

	// Load cached obj file
	u8 *data;
	int n;
	ReadData(ifs, mesh->mVertexDesc, &data, n);
	Vertex *vtx = (Vertex *) data;
	for (int i = 0; i < n; i++)
		mesh->mVertices.push_back(vtx[i]);
	delete [] data;
	StructDesc idxDesc;
	ReadData(ifs, idxDesc, &data, n);
	Triangle *idx = (Triangle *) data;
	for (int i = 0; i < n; i++)
		mesh->mTriangles.push_back(idx[i]);
	delete [] data;
	size_t nGroups;
	short hasMaterialFileName;
	ifs >> hasMaterialFileName;
	if (hasMaterialFileName == 1)
		ifs >> mesh->mMaterialFileName;
	ifs >> nGroups;
	for (size_t i = 0; i < nGroups; i++) {
		Group g;
		short hasMaterial;
		ifs >> g.mName;
		ifs >> hasMaterial;
		if (hasMaterial == 1)
			ifs >> g.mMaterial;
		ifs >> g.mStart;
		ifs >> g.mEnd;
		mesh->mGroups.push_back(g);
	}
	ifs.close();

	LogTrivia("Loading mesh took %u ms", EndTimerMilliseconds(startTime));

	return mesh;
}
