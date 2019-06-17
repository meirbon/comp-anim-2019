#include <GL/glew.h>

#include "Model.h"

#include "Camera.h"
#include "utils/Logger.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <assimp/cimport.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum UniformLocations
{
	POSITION = 0,
	TEXCOORD = 1,
	NORMAL = 2,
	BONE_ID = 3,
	BONE_WEIGHT = 4
};

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4

void ai_to_glm(const aiMatrix4x4 &from, glm::mat4 &to)
{
	to[0][0] = from[0][0];
	to[0][1] = from[0][1];
	to[0][2] = from[0][2];
	to[0][3] = from[0][3];

	to[1][0] = from[1][0];
	to[1][1] = from[1][1];
	to[1][2] = from[1][2];
	to[1][3] = from[1][3];

	to[2][0] = from[2][0];
	to[2][1] = from[2][1];
	to[2][2] = from[2][2];
	to[2][3] = from[2][3];

	to[3][0] = from[3][0];
	to[3][1] = from[3][1];
	to[3][2] = from[3][2];
	to[3][3] = from[3][3];
}

void glm_to_ai(const glm::mat4 &from, aiMatrix4x4 &to)
{
	glm::mat4 newMat = glm::transpose(from);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			to[i][j] = newMat[i][j];
		}
	}
}

Model::Model(bool normalize)
{
	m_Importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, normalize);
	m_VAO = 0;
	memset(m_Buffers, 0, sizeof(m_Buffers));
	m_NumBones = 0;
	m_pScene = nullptr;
}

Model::~Model()
{
	clear();
}

void Model::clear()
{
	for (unsigned int i = 0; i < m_Textures.size(); i++)
	{
		if (m_Textures[i])
		{
			delete m_Textures[i];
			m_Textures[i] = nullptr;
		}
	}

	if (m_Buffers[0] != 0)
	{
		glDeleteBuffers(sizeof(m_Buffers) / sizeof(m_Buffers[0]), m_Buffers);
	}

	if (m_VAO != 0)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}
void getGlobalNodeTransform(aiMatrix4x4 *result, aiNode *node)
{
	if (node->mParent)
	{
		getGlobalNodeTransform(result, node->mParent);
		aiMultiplyMatrix4(result, &node->mTransformation);
	}
	else
	{
		*result = node->mTransformation;
	}
}
void Model::transformAllMeshes()
{
	//Loop through all meshes
	for (int meshIdx = 0; meshIdx < m_pScene->mNumMeshes; ++meshIdx)
	{
		aiMesh *assMesh = m_pScene->mMeshes[meshIdx];

		//Only skinned meshes are processed.
		if (assMesh->mNumBones == 0)
			continue;

		//Create positions vector which we will pass to OpenGL
		const auto &meshEntry = m_Entries[meshIdx];
		const auto numVertices = m_pScene->mMeshes[meshIdx]->mNumVertices;
		std::vector<glm::vec3> newPositions(numVertices);
		std::vector<glm::vec3> newNormals(numVertices);

		//For every mesh, loop through all affected bones
		for (int boneIdx = 0; boneIdx < assMesh->mNumBones; boneIdx++)
		{
			//We transform vertex positions with a 4x4 position since we can do translation
			//But normals are only transformed using Rotation + Scale (3x3 matrix)
			aiMatrix4x4 skin4x4;
			aiMatrix3x3 skin3x3;
			aiIdentityMatrix4(&skin4x4);
			aiIdentityMatrix3(&skin3x3);

			aiBone *bone = assMesh->mBones[boneIdx];
			//Find the bone node in the hierarchy.
			aiNode *node = m_pScene->mRootNode->FindNode(bone->mName);
			assert(node != nullptr);
			//Get the transformation of that bone
			getGlobalNodeTransform(&skin4x4, node);
			aiMultiplyMatrix4(&skin4x4, &bone->mOffsetMatrix);

			//Now, for every bone we must loop through every vertex that bone affects in the mesh
			for (int i = 0; i < bone->mNumWeights; i++)
			{
				int v = bone->mWeights[i].mVertexId;
				float w = bone->mWeights[i].mWeight;

				aiVector3D aiPos = assMesh->mVertices[v];
				aiTransformVecByMatrix4(&aiPos, &skin4x4);

				newPositions[v] += glm::vec3(aiPos.x * w, aiPos.y * w, aiPos.z * w);

				aiVector3D normal = assMesh->mNormals[v];
				aiTransformVecByMatrix3(&normal, &skin3x3);
				newNormals[v] += glm::vec3(normal.x * w, normal.y * w, normal.z * w);
			}
		}
		// Finished processing all bones and vertices affected by bones of this mesh
		// Give new position and normals to OpenGL
		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
		glBufferSubData(GL_ARRAY_BUFFER, meshEntry.BaseVertex * sizeof(float) * 3, numVertices * sizeof(float) * 3, newPositions.data());

		glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
		glBufferSubData(GL_ARRAY_BUFFER, meshEntry.BaseVertex * sizeof(float) * 3, numVertices * sizeof(float) * 3, newNormals.data());
	}
}

bool Model::loadMesh(const std::string &Filename)
{
	clear();

	glCreateVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glCreateBuffers(sizeof(m_Buffers) / sizeof(m_Buffers[0]), m_Buffers);

	bool Ret = false;

	m_pScene = m_Importer.ReadFile(
		Filename.c_str(),
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	if (m_pScene)
	{
		Ret = initFromScene(m_pScene, Filename);
	}
	else
	{
		WARNING("Error parsing: %s, error: %s", Filename.c_str(), m_Importer.GetErrorString());
	}

	// Make sure the VAO is not changed from the outside
	glBindVertexArray(0);

	return Ret;
}

bool Model::initFromScene(const aiScene *pScene, const std::string &Filename)
{
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);
	m_meshTransformMatrices.resize(pScene->mNumMeshes);
	for (auto &m : m_meshTransformMatrices)
		m = glm::identity<glm::mat4>();

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec2> TexCoords;
	std::vector<unsigned int> Indices;

	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	// Count the number of vertices and indices
	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		m_Entries[i].BaseVertex = NumVertices;
		m_Entries[i].BaseIndex = NumIndices;
		m_Entries[i].NumVertices = pScene->mMeshes[i]->mNumVertices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += m_Entries[i].NumIndices;
	}

	// Reserve space in the std::vectors for the vertex attributes and indices
	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Indices.reserve(NumIndices);

	// Initialize the meshes in the scene one by one
	for (unsigned int i = 0; i < m_Entries.size(); i++)
	{
		const aiMesh *paiMesh = pScene->mMeshes[i];
		initMesh(i, paiMesh, Positions, Normals, TexCoords, Indices);
	}

	if (!initMaterials(pScene, Filename))
	{
		return false;
	}

	// Generate and populate the buffers with vertex attributes and the indices
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(UniformLocations::POSITION);
	glVertexAttribPointer(UniformLocations::POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(UniformLocations::TEXCOORD);
	glVertexAttribPointer(UniformLocations::TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(UniformLocations::NORMAL);
	glVertexAttribPointer(UniformLocations::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	return static_cast<bool>(glGetError());
}

void Model::initMesh(unsigned int MeshIndex, const aiMesh *paiMesh, std::vector<glm::vec3> &Positions, std::vector<glm::vec3> &Normals, std::vector<glm::vec2> &TexCoords, std::vector<unsigned int> &Indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	// Populate the vertex attribute std::vectors
	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
	{
		const aiVector3D *pPos = &(paiMesh->mVertices[i]);
		const aiVector3D *pNormal = &(paiMesh->mNormals[i]);
		const aiVector3D *pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
		Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
	}

	//LoadBones(MeshIndex, paiMesh, Bones);

	// Populate the index buffer
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
	{
		const aiFace &Face = paiMesh->mFaces[i];
		assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
}

std::tuple<aiNode *, glm::mat4> getNodeByName(std::string nodeName, glm::mat4 parentTransform, aiNode *parent)
{
	if (parent->mName.C_Str() == nodeName) return std::make_tuple(parent, parentTransform);

	for (int i = 0; i < parent->mNumChildren; i++)
	{
		aiNode *child = parent->mChildren[i];

		glm::mat4 transform;
		ai_to_glm(child->mTransformation, transform);

		transform = transform * parentTransform;

		auto foundInChildren = getNodeByName(nodeName, transform, child);

		bool found = std::get<0>(foundInChildren);

		if (found) return foundInChildren;
	}

	return std::make_tuple(nullptr, parentTransform);
}

glm::vec3 transformToPosition(aiMatrix4x4 aiTransform)
{
	glm::mat4 transform;

	ai_to_glm(aiTransform, transform);

	auto result = glm::vec4(0, 0, 0, 1) * transform;

	return (glm::vec3)result;
}

void findBonesRecursive(aiNode *parent, glm::mat4 parentTransform, std::vector<std::string> boneNames, std::vector<std::tuple<std::string, glm::vec3, glm::vec3>> *bones)
{
	glm::vec3 parentPosition = glm::vec4(0, 0, 0, 1) * parentTransform;

	for (int i = 0; i < parent->mNumChildren; i++)
	{
		aiNode *child = parent->mChildren[i];

		glm::mat4 transform;
		ai_to_glm(child->mTransformation, transform);

		transform = transform * parentTransform;

		glm::vec3 childPosition = glm::vec4(0, 0, 0, 1) * transform;

		bones->push_back(std::tuple<std::string, glm::vec3, glm::vec3>(child->mName.C_Str(), parentPosition, childPosition));

		findBonesRecursive(child, transform, boneNames, bones); // Recursively iterate over children
	}
}

std::vector<std::tuple<std::string, glm::vec3, glm::vec3>> Model::getSkeletalRig(std::string rootNodeName)
{
	auto identity = glm::identity<glm::mat4>();

	std::tuple<aiNode *, glm::mat4> foundNode = getNodeByName(rootNodeName, identity, m_pScene->mRootNode);

	aiNode *skeletalRoot;
	glm::mat4 rootTransform;
	std::tie(skeletalRoot, rootTransform) = foundNode;

	std::vector<std::tuple<std::string, glm::vec3, glm::vec3>> bones;

	std::vector<std::string> boneNames;

	for (const auto &map : m_BoneMapping)
		boneNames.push_back(map.first);

	findBonesRecursive(skeletalRoot, rootTransform, boneNames, &bones);

	return bones;
}

bool Model::initMaterials(const aiScene *pScene, const std::string &Filename)
{
	// Extract the directory part from the file name
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos)
	{
		Dir = ".";
	}
	else if (SlashIndex == 0)
	{
		Dir = "/";
	}
	else
	{
		Dir = Filename.substr(0, SlashIndex);
	}

	bool Ret = true;

	// Initialize the materials
	for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
	{
		const aiMaterial *pMaterial = pScene->mMaterials[i];

		m_Textures[i] = NULL;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string p(Path.data);

				if (p.substr(0, 2) == ".\\")
				{
					p = p.substr(2, p.size() - 2);
				}

				std::string FullPath = Dir + "/" + p;

				m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

				if (!m_Textures[i]->load())
					WARNING("Error loading texture '%s'", FullPath.c_str());
				else
					DEBUG("%d - loaded texture '%s'", i, FullPath.c_str());
			}
		}
	}

	return Ret;
}

void Model::render(Shader &shader, Camera &camera)
{
	using namespace glm;
	glBindVertexArray(m_VAO);

	for (int i = 0; i < m_Entries.size(); ++i)
	{
		const auto &entry = m_Entries[i];

		const auto modifyModel = rotate(translate(mat4(1.0f), vec3(0, 2, 0)), radians(180.0f), vec3(0, 0, 1));
		auto model = glm::transpose(m_meshTransformMatrices[i]) * glm::mat4(1.0f);
		model = modifyModel * model;

		const auto matrix = camera.getCombinedMatrix(model);

		shader.setUniformFloat("gWorld", inverse(model), true);
		shader.setUniformFloat("MVP", matrix);
		shader.setUniformInt("texture0", 0);

		//const auto model = glm::mat4(1.0f);
		//const auto matrix = camera.getCombinedMatrix(model);

		//if (m_pScene->mMeshes[i]->mNumBones == 0) continue;

		assert(entry.MaterialIndex < m_Textures.size());

		if (m_Textures[entry.MaterialIndex])
			m_Textures[entry.MaterialIndex]->bind(GL_TEXTURE0);

		glDrawElementsBaseVertex(GL_TRIANGLES,
								 entry.NumIndices,
								 GL_UNSIGNED_INT,
								 (void *)(sizeof(unsigned int) * entry.BaseIndex),
								 entry.BaseVertex);
	}

	glBindVertexArray(0);
}

unsigned int Model::findPosition(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
		{
			return i;
		}
	}

	assert(0);
	return 0;
}

unsigned int Model::findRotation(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
		{
			return i;
		}
	}

	assert(0);

	return 0;
}

unsigned int Model::findScaling(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
		{
			return i;
		}
	}

	assert(0);

	return 0;
}

glm::vec3 Model::calcInterpolatedPosition(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1)
	{
		return glm::make_vec3((float *)&pNodeAnim->mPositionKeys[0].mValue);
	}

	unsigned int PositionIndex = findPosition(AnimationTime, pNodeAnim);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D &Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D &End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;

	aiVector3D Delta = Start + Factor * (End - Start);
	return glm::make_vec3((float *)&Delta);
}

void Model::calcInterpolatedRotation(aiQuaternion &Out, float AnimationTime, const aiNodeAnim *pNodeAnim)
{
	if (pNodeAnim->mNumRotationKeys == 1)
	{
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = findRotation(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion &StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion &EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

glm::vec3 Model::calcInterpolatedScaling(float AnimationTime, const aiNodeAnim *pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1)
	{
		return glm::make_vec3((float *)&pNodeAnim->mScalingKeys[0].mValue);
	}

	unsigned int ScalingIndex = findScaling(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D &Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D &End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = Start + Factor * (End - Start);

	return glm::make_vec3((float *)&Delta);
}

void Model::readNodeHierarchy(float AnimationTime, aiNode *pNode, aiMatrix4x4 worldTransform)
{
	const std::string NodeName(pNode->mName.data);

	const aiAnimation *pAnimation = m_pScene->mAnimations[0];

	aiMultiplyMatrix4(&worldTransform, &pNode->mTransformation);

	for (int i = 0; i < pNode->mNumMeshes; i++)
	{
		if (m_pScene->mMeshes[pNode->mMeshes[i]]->mNumBones == 0)
		{
			ai_to_glm(worldTransform, m_meshTransformMatrices[pNode->mMeshes[i]]);
		}
	}

	//Enable animation
#if 1
	const aiNodeAnim *pNodeAnim = findNodeAnim(pAnimation, NodeName);

	if (pNodeAnim)
	{
		aiQuaternion RotationKey;
		calcInterpolatedRotation(RotationKey, AnimationTime, pNodeAnim);

		glm::vec3 t = calcInterpolatedPosition(AnimationTime, pNodeAnim);
		glm::quat q(RotationKey.w, RotationKey.x, RotationKey.y, RotationKey.z);
		glm::vec3 s = calcInterpolatedScaling(AnimationTime, pNodeAnim);

		glm::mat4 S = glm::scale(glm::mat4(1), s);
		glm::mat4 R = glm::mat4_cast(q);
		glm::mat4 T = glm::translate(glm::mat4(1), t);

		glm_to_ai(T * R * S, pNode->mTransformation);
	}
#endif

	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		readNodeHierarchy(AnimationTime, pNode->mChildren[i], worldTransform);
	}
}

bool Model::transformBones(float TimeInSeconds)
{
	//TODO(Dan): Investigate if this is needed?
	//if (m_NumBones <= 0) return;

	aiMatrix4x4 Identity;
	aiIdentityMatrix4(&Identity);

	const float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
	const float TimeInTicks = TimeInSeconds * TicksPerSecond;
	const float AnimationTime = fmod(TimeInTicks, (float)m_pScene->mAnimations[0]->mDuration);

	transformAllMeshes();

	readNodeHierarchy(AnimationTime, m_pScene->mRootNode, Identity);

	return TimeInTicks > static_cast<float>(m_pScene->mAnimations[0]->mDuration);
}

const aiNodeAnim *Model::findNodeAnim(const aiAnimation *pAnimation, const std::string NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++)
	{
		const aiNodeAnim *pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName)
		{
			return pNodeAnim;
		}
	}

	return nullptr;
}