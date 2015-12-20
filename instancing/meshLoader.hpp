/*
* Simple wrapper for getting an index buffer and vertices out of an assimp mesh
*
* Copyright (C) 2015 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <stdlib.h>
#include <string>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#endif

#include <assimp\Importer.hpp> 
#include <assimp\scene.h>     
#include <assimp\postprocess.h>
#include <assimp\cimport.h>

#include <glm\glm.hpp>

class MeshLoader {
private:

	struct Vertex
	{
		glm::vec3 m_pos;
		glm::vec2 m_tex;
		glm::vec3 m_normal;
		glm::vec3 m_color;
		glm::vec3 m_tangent;
		glm::vec3 m_binormal;

		Vertex() {}

		Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec3& bitangent, const glm::vec3& color)
		{
			m_pos = pos;
			m_tex = tex;
			m_normal = normal;
			m_color = color;
			m_tangent = tangent;
			m_binormal = bitangent;
		}
	};

	struct MeshEntry {
		unsigned int NumIndices;
		unsigned int MaterialIndex;
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};

public:
	std::vector<MeshEntry> m_Entries;

	struct Dimension 
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
		glm::vec3 size;
	} dim;

	~MeshLoader()
	{
		m_Entries.clear();
	}

	bool LoadMesh(const std::string& Filename) // TODO : Flag param, maybe with overload (tangent space)
	{
		bool Ret = false;
		Assimp::Importer Importer;

		int flags = aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace;
		const aiScene* pScene = Importer.ReadFile(Filename.c_str(), flags);

		if (pScene) {
			Ret = InitFromScene(pScene, Filename);
		}
		else {
			printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
		}

		return Ret;
	}

	bool InitFromScene(const aiScene* pScene, const std::string& Filename)
	{
		m_Entries.resize(pScene->mNumMeshes);

		// Initialize the meshes in the scene one by one
		for (unsigned int i = 0; i < m_Entries.size(); i++) {
			const aiMesh* paiMesh = pScene->mMeshes[i];
			InitMesh(i, paiMesh, pScene);
		}

		return true;
	}

	void InitMesh(unsigned int Index, const aiMesh* paiMesh, const aiScene* pScene)
	{
		m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

		aiColor3D pColor(0.f, 0.f, 0.f);
		pScene->mMaterials[paiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);

		aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

		for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
			aiVector3D* pPos = &(paiMesh->mVertices[i]);
			aiVector3D* pNormal = &(paiMesh->mNormals[i]);
			aiVector3D *pTexCoord;
			if (paiMesh->HasTextureCoords(0))
			{
				pTexCoord = &(paiMesh->mTextureCoords[0][i]);
			}
			else {
				pTexCoord = &Zero3D;
			}
			aiVector3D* pTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mTangents[i]) : &Zero3D;
			aiVector3D* pBiTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mBitangents[i]) : &Zero3D;

			Vertex v(glm::vec3(pPos->x, pPos->y, pPos->z),
				glm::vec2(pTexCoord->x, pTexCoord->y),
				glm::vec3(pNormal->x, pNormal->y, pNormal->z),
				glm::vec3(pTangent->x, pTangent->y, pTangent->z),
				glm::vec3(pBiTangent->x, pBiTangent->y, pBiTangent->z),
				glm::vec3(pColor.r, pColor.g, pColor.b)
				);

			dim.max.x = fmax(pPos->x, dim.max.x);
			dim.max.y = fmax(pPos->y, dim.max.y);
			dim.max.z = fmax(pPos->z, dim.max.z);

			dim.min.x = fmin(pPos->x, dim.min.x);
			dim.min.y = fmin(pPos->y, dim.min.y);
			dim.min.z = fmin(pPos->z, dim.min.z);

			m_Entries[Index].Vertices.push_back(v);
		}

		dim.size = dim.max - dim.min;

		for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
			const aiFace& Face = paiMesh->mFaces[i];
			assert(Face.mNumIndices == 3);
			m_Entries[Index].Indices.push_back(Face.mIndices[0]);
			m_Entries[Index].Indices.push_back(Face.mIndices[1]);
			m_Entries[Index].Indices.push_back(Face.mIndices[2]);
		}

	}
};