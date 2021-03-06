#include "stdafx.h"

#if defined(CLASSICFORMAT)

#include "ClassicFormatReader.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "../geometry/Matrix4.h"

std::wstring folder;

bool proceedMesh(aiMesh* mesh,
				const aiScene* scene,
				gaia3d::Matrix4& transform,
				std::vector<gaia3d::TrianglePolyhedron*>& container,
				std::map<std::wstring, std::wstring>& textureContainer)
{
	if (mesh->mNumVertices < 3)
	{
		return false;
	}
	// aiMesh == TrianglePolyhedron
	gaia3d::TrianglePolyhedron* polyhedron = new gaia3d::TrianglePolyhedron;

	// check if texture exists.
	bool textureExistsForMesh = false;
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	if (mesh->mMaterialIndex == 291)
		int x = 0;
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0 && mesh->mTextureCoords[0] != NULL)
	{
		textureExistsForMesh = true;
		polyhedron->setHasTextureCoordinates(true);

		// collect texture info
		aiString texturePath;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
		int neededLength = MultiByteToWideChar(CP_UTF8, 0, texturePath.C_Str(), -1, NULL, 0);
		if (neededLength > 0)
		{
			wchar_t* receiver = new wchar_t[neededLength];
			memset(receiver, 0x00, sizeof(wchar_t)*neededLength);
			MultiByteToWideChar(CP_UTF8, 0, texturePath.C_Str(), -1, receiver, neededLength);
			std::wstring fullPath = folder + L"/" + std::wstring(receiver);
			delete[] receiver; receiver = NULL;
			size_t lastSlashIndex = fullPath.find_last_of(L"\\/");
			std::wstring fileName;
			if (lastSlashIndex == std::wstring::npos)
				fileName = fullPath;
			else
			{
				size_t fileNameLength = fullPath.length() - lastSlashIndex - 1;
				fileName = fullPath.substr(lastSlashIndex + 1, fileNameLength);
			}

			if (textureContainer.find(fileName) == textureContainer.end())
			{
				textureContainer.insert(std::map<std::wstring, std::wstring>::value_type(fileName, fullPath));
			}

			polyhedron->addStringAttribute(std::wstring(TextureName), fileName);
		}
	}

	// check if color info exists.
	gaia3d::ColorMode colorMode = gaia3d::NoColor;
	if (!textureExistsForMesh)
	{
		aiReturn result;
		aiColor3D colorDiffuse(0.f, 0.f, 0.f);
		result = material->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse);
		if (result == aiReturn_SUCCESS)
		{
			colorMode = gaia3d::SingleColor;
			polyhedron->setSingleColor(MakeColorU4((unsigned char)(colorDiffuse.r * 255),
				(unsigned char)(colorDiffuse.g * 255),
				(unsigned char)(colorDiffuse.b * 255)));
		}
		else
		{
			if (mesh->mColors[0] != NULL)
				colorMode = gaia3d::ColorsOnVertices;
		}
	}
	polyhedron->setColorMode(colorMode);

	// check if vertex normals
	if (mesh->HasNormals())
		polyhedron->setHasNormals(true);

	// access to vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		gaia3d::Vertex* vertex = new gaia3d::Vertex;

		vertex->position.x = mesh->mVertices[i].x;
		vertex->position.y = mesh->mVertices[i].y;
		vertex->position.z = mesh->mVertices[i].z;

		vertex->position = transform * vertex->position;

		if (mesh->HasNormals())
		{
			vertex->normal.x = mesh->mNormals[i].x;
			vertex->normal.y = mesh->mNormals[i].y;
			vertex->normal.z = mesh->mNormals[i].z;

			transform.applyOnlyRotationOnPoint(vertex->normal);

			vertex->normal.normalize();
		}

		if (textureExistsForMesh)
		{
			vertex->textureCoordinate[0] = mesh->mTextureCoords[0][i].x;
			vertex->textureCoordinate[1] = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			if (colorMode == gaia3d::ColorsOnVertices)
			{
				vertex->color = MakeColorU4((unsigned char)(mesh->mColors[0][i].r * 255),
											(unsigned char)(mesh->mColors[0][i].g * 255),
											(unsigned char)(mesh->mColors[0][i].b * 255));
			}
		}

		polyhedron->getVertices().push_back(vertex);
	}

	// assimp model has 2 level hierarchy but TrianglePolyhedron has 3 level of it
	// so add 1 Surface to polyhedron
	gaia3d::Surface* surface = new gaia3d::Surface;
	polyhedron->getSurfaces().push_back(surface);

	// make triangles and assign vertices to each triangle
	gaia3d::Triangle* triangle;
	bool wrongIndex;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		if (mesh->mFaces[i].mNumIndices != 3)
			continue;

		triangle = new gaia3d::Triangle;

		wrongIndex = false;
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			if (mesh->mFaces[i].mIndices[j] >= polyhedron->getVertices().size())
			{
				wrongIndex = true;
				break;
			}
		}
		
		if (wrongIndex)
			continue;

		triangle->setVertexIndices(mesh->mFaces[i].mIndices[0],
									mesh->mFaces[i].mIndices[1],
									mesh->mFaces[i].mIndices[2]);

		triangle->setVertices(polyhedron->getVertices()[triangle->getVertexIndices()[0]],
			polyhedron->getVertices()[triangle->getVertexIndices()[1]],
			polyhedron->getVertices()[triangle->getVertexIndices()[2]]);

		surface->getTriangles().push_back(triangle);
	}

	polyhedron->setId(container.size());

	container.push_back(polyhedron);
	
	return true;
}

bool proceedNode(aiNode* node,
				const aiScene* scene,
				gaia3d::Matrix4& parentMatrix,
				std::vector<gaia3d::TrianglePolyhedron*>& container,
				std::map<std::wstring, std::wstring>& textureContainer)
{
	gaia3d::Matrix4 thisMatrix;

	thisMatrix.set(node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1, node->mTransformation.d1,
		node->mTransformation.a2, node->mTransformation.b2, node->mTransformation.c2, node->mTransformation.d2,
		node->mTransformation.a3, node->mTransformation.b3, node->mTransformation.c3, node->mTransformation.d3,
		node->mTransformation.a4, node->mTransformation.b4, node->mTransformation.c4, node->mTransformation.d4);

	thisMatrix = thisMatrix * parentMatrix;

	aiMesh *mesh = NULL;
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		mesh = scene->mMeshes[node->mMeshes[i]];
		proceedMesh(mesh, scene, thisMatrix, container, textureContainer);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		proceedNode(node->mChildren[i], scene, thisMatrix, container, textureContainer);
	}
	return true;
}

ClassicFormatReader::ClassicFormatReader()
{
	unitScaleFactor = 1.0;
}


ClassicFormatReader::~ClassicFormatReader()
{
}

bool ClassicFormatReader::readRawDataFile(std::wstring& filePath)
{
	Assimp::Importer importer;

	std::string singleCharFilePath = std::string(CW2A(filePath.c_str()));

	const aiScene* scene = importer.ReadFile(singleCharFilePath,
		//aiProcess_SplitLargeMeshes |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (scene == NULL)
		return false;

	if (scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE)
		return false;

	if (scene->mRootNode == NULL)
		return false;

	size_t slashPosition = filePath.find_last_of(L"\\/");
	if (slashPosition == std::wstring::npos)
		folder = filePath;
	else
		folder = filePath.substr(0, slashPosition);

	gaia3d::Matrix4 scaleMatrix;
	scaleMatrix.set(scaleMatrix.m[0][0]*unitScaleFactor, scaleMatrix.m[0][1], scaleMatrix.m[0][2], scaleMatrix.m[0][3],
					scaleMatrix.m[1][0], scaleMatrix.m[1][1] * unitScaleFactor, scaleMatrix.m[1][2], scaleMatrix.m[1][3],
					scaleMatrix.m[2][0], scaleMatrix.m[2][1], scaleMatrix.m[2][2] * unitScaleFactor, scaleMatrix.m[2][3],
					scaleMatrix.m[3][0], scaleMatrix.m[3][1], scaleMatrix.m[3][2], scaleMatrix.m[3][3]);
	if (!proceedNode(scene->mRootNode, scene, scaleMatrix, container, textureContainer))
		return false;

	return true;
}

void ClassicFormatReader::clear()
{
	container.clear();

	textureContainer.clear();
}



#endif