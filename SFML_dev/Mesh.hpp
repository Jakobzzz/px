#pragma once

#include "Shader.hpp"
#include <memory>

namespace px
{
	struct Vertex
	{
		//Only position and normal at this point
		glm::vec3 position;
		glm::vec3 normal;
	};

	class Mesh
	{
	public:
		Mesh(std::vector<Vertex> & vertices, std::vector<unsigned int> & indices, glm::vec3 & color);

	public:
		void Draw(Shaders::ID id);
		void Destroy();

	public:
		void SetColor(glm::vec3 color);

	public:
		glm::vec3 GetColor();

	private:
		void SetupMesh();

	private:
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		unsigned int m_VAO, m_VBO, m_EBO;
		unsigned int m_nrOfVertices, m_nrOfIndices;
		glm::vec3 m_color;
	};
}

