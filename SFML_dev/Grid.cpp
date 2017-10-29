#include "Grid.hpp"
#include <glad/glad.h>
#include "Camera.hpp"
#include <iostream>

namespace px
{
	Grid::Grid(std::shared_ptr<Camera> & camera) : m_camera(camera)
	{
		SetupGrid();
	}

	Grid::~Grid()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteBuffers(1, &m_EBO);
	}

	void Grid::Draw(Shaders::ID id)
	{
		Shader::Use(id);

		glm::mat4 m_world = glm::mat4();
		m_world = glm::translate(m_world, glm::vec3(-50.f, 0.f, -50.f));

		Shader::SetMatrix4x4(id, "model", m_world);
		Shader::SetMatrix4x4(id, "projection", m_camera->GetProjectionMatrix());
		Shader::SetMatrix4x4(id, "view", m_camera->GetViewMatrix());

		glBindVertexArray(m_VAO);
		glDrawElements(GL_LINES, m_indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void Grid::SetupGrid()
	{
		m_width = 100;
		m_height = 100;
		m_vertexCount = (m_width - 1) * (m_height - 1) * 8;
		m_indexCount = m_vertexCount;

		glm::vec3* vertices = new glm::vec3[m_vertexCount];
		unsigned int* indices = new unsigned int[m_indexCount];

		int index = 0;

		//Load the vertex array and index array with data.
		for (unsigned int j = 0; j < m_height - 1; j++)
		{
			for (unsigned int i = 0; i < m_width - 1; i++)
			{
				//Line 1 - Upper left.
				float positionX = (float)i;
				float positionZ = (float)(j + 1);

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 1 - Upper right.
				positionX = (float)(i + 1);
				positionZ = (float)(j + 1);

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 2 - Upper right
				positionX = (float)(i + 1);
				positionZ = (float)(j + 1);

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 2 - Bottom right.
				positionX = (float)(i + 1);
				positionZ = (float)j;

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 3 - Bottom right.
				positionX = (float)(i + 1);
				positionZ = (float)j;

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 3 - Bottom left.
				positionX = (float)i;
				positionZ = (float)j;

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 4 - Bottom left.
				positionX = (float)i;
				positionZ = (float)j;

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;

				//Line 4 - Upper left.
				positionX = (float)i;
				positionZ = (float)(j + 1);

				vertices[index] = glm::vec3(positionX, 0.0f, positionZ);
				indices[index] = index;
				index++;
			}
		}

		//Generate buffers
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

		glBufferData(GL_ARRAY_BUFFER, m_vertexCount * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

		//Positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);

		//Release arrays
		delete[] vertices;
		delete[] indices;

		vertices = 0;
		indices = 0;
	}
}
