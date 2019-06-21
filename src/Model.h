#pragma once
#include <GL/glew.h>

#include "Camera.h"
#include <cassert>
#include <map>
#include <vector>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Shader.h"
#include "Texture.h"

class Model
{
  public:
	Model(bool normalize = true);

	~Model();

	/*
	 * Loads in mesh from given file.
	 */
	bool loadMesh(const std::string &Filename);

	/*
	 * Renders mesh using given shader and camera.
	 */
	void render(Shader &shader, Camera &camera);

	unsigned int NumBones() const { return m_NumBones; }

	/*
	 * Transform mesh to given time.
	 */
	bool transformBones(float TimeInSeconds);

	/*
	 * Retrieve skeleton.
	 */
	std::vector<std::tuple<std::string, glm::vec3, glm::vec3>> getSkeletalRig(std::string rootNodeName);

  private:
	struct BoneInfo
	{
		BoneInfo() = default;
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	/// Linearly interpolate from last animation position to current one, based on  keyframe time of animations
	glm::vec3 calcInterpolatedPosition(float AnimationTime, const aiNodeAnim *pNodeAnim);

	/// Spherically interpolates between quaternion rotations
	void calcInterpolatedRotation(aiQuaternion &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);

	/// Linearly interpolate from last animation scale to current one, based on  keyframe time of animations
	glm::vec3 calcInterpolatedScaling(float AnimationTime, const aiNodeAnim *pNodeAnim);

	/// Finds the last keyframe before AnimationTime
	unsigned int findScaling(float AnimationTime, const aiNodeAnim *pNodeAnim);
	unsigned int findRotation(float AnimationTime, const aiNodeAnim *pNodeAnim);
	unsigned int findPosition(float AnimationTime, const aiNodeAnim *pNodeAnim);

	// Helper functions to read Assimp data.
	const aiNodeAnim *findNodeAnim(const aiAnimation *pAnimation, const std::string NodeName);
	void readNodeHierarchy(float AnimationTime, aiNode *pNode, aiMatrix4x4 ParentTransform);

	// Model intialization functions.
	bool initFromScene(const aiScene *pScene, const std::string &Filename);
	void initMesh(unsigned int MeshIndex, const aiMesh *paiMesh, std::vector<glm::vec3> &Positions, std::vector<glm::vec3> &Normals, std::vector<glm::vec2> &TexCoords, std::vector<unsigned int> &Indices);
	bool initMaterials(const aiScene *pScene, const std::string &Filename);

	void clear();
	/*
	 * Transforms all the model's meshes to the corresponding keyframe position.
	 */
	void transformAllMeshes();

#define INVALID_MATERIAL 0xFFFFFFFF

	enum VB_TYPES
	{
		INDEX_BUFFER,
		POS_VB,
		NORMAL_VB,
		TEXCOORD_VB,
		NUM_VBs
	};

	GLuint m_VAO;
	GLuint m_Buffers[NUM_VBs];

	struct MeshEntry
	{
		MeshEntry()
		{
			NumIndices = 0;
			NumVertices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		//Number of indices in mesh
		unsigned int NumIndices;
		//Number of vertices in mesh
		unsigned int NumVertices;
		//Where the beginning vertex is in OpenGL array buffer
		unsigned int BaseVertex;
		//Where the beginning index is in OpenGL array buffer
		unsigned int BaseIndex;

		unsigned int MaterialIndex;
	};

	std::vector<MeshEntry> m_Entries;
	std::vector<glm::mat4> m_meshTransformMatrices;
	std::vector<Texture *> m_Textures;
	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int m_NumBones;
	std::vector<BoneInfo> m_BoneInfo;

	const aiScene *m_pScene;
	Assimp::Importer m_Importer;
};