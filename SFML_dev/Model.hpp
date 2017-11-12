#pragma once
#include "Mesh.hpp"

#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace px
{
	template <typename Identifier>
	class Model
	{
	public:
		void LoadModel(Identifier id, std::string const & path);
		void Draw(Identifier id, Shaders::ID shaderID);
		void Destroy(Identifier id);

	public:
		void SetColor(Identifier id, glm::vec3 color);

	public:
		glm::vec3 GetColor(Identifier id);

	private:
		void ProcessNode(Identifier id, aiNode* node, const aiScene* scene);
		std::unique_ptr<Mesh> ProcessMesh(Identifier id, aiMesh* mesh, const aiScene* scene);

	private:
		std::map<Identifier, std::vector<std::unique_ptr<Mesh>>> m_models;
		std::vector<std::unique_ptr<Mesh>> m_meshes;
		std::string m_directory;
	};

	template <typename Identifier>
	inline void Model<Identifier>::LoadModel(Identifier id, std::string const & path)
	{
		//Read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

		//Check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) //If is Not Zero
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		//Retrieve the directory path of the filepath
		m_directory = path.substr(0, path.find_last_of('/'));
		ProcessNode(id, scene->mRootNode, scene);
	}

	template <typename Identifier>
	inline void Model<Identifier>::Draw(Identifier id, Shaders::ID shaderID)
	{
		auto found = m_models.find(id);
		assert(found != m_models.end());

		for (auto & mesh : found->second)
			mesh->Draw(shaderID);
	}

	template <typename Identifier>
	inline void Model<Identifier>::SetColor(Identifier id, glm::vec3 color) //This need some kind of index for child nodes
	{
		auto found = m_models.find(id);
		assert(found != m_models.end());

		for (auto & mesh : found->second)
			mesh->SetColor(color);
	}

	template <typename Identifier>
	inline glm::vec3 Model<Identifier>::GetColor(Identifier id) //This need some kind of index for child nodes
	{
		auto found = m_models.find(id);
		assert(found != m_models.end());

		for (auto & mesh : found->second)
			return mesh->GetColor();

		return glm::vec3();
	}

	template <typename Identifier>
	inline void Model<Identifier>::Destroy(Identifier id)
	{
		auto found = m_models.find(id);
		assert(found != m_models.end());

		for (auto & mesh : found->second)
			mesh->Destroy();
	}

	template <typename Identifier>
	inline void Model<Identifier>::ProcessNode(Identifier id, aiNode * node, const aiScene * scene)
	{
		//Process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.push_back(std::move(ProcessMesh(id, mesh, scene)));
		}

		//Process children if any after the meshes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
			ProcessNode(id, node->mChildren[i], scene);

		//Add the final model to the container
		m_models.insert(std::make_pair(id, std::move(m_meshes)));
	}

	template <typename Identifier>
	inline std::unique_ptr<Mesh> Model<Identifier>::ProcessMesh(Identifier id, aiMesh * mesh, const aiScene * scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		glm::vec3 vertexColor;

		//Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			//Position
			vector = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.position = vector;

			//Normals
			vector = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			vertex.normal = vector;

			vertices.push_back(vertex);
		}

		//Go through the faces
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			//Retrieve indices
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		//Color materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		aiColor3D color(0.f, 0.f, 0.f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		vertexColor = glm::vec3(color.r, color.g, color.b);

		auto object = std::make_unique<Mesh>(vertices, indices, vertexColor);
		return object;
	}
}
