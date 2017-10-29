#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"

#include <memory>
#include <vector>
#include <array>

namespace px
{
	class Camera;

	class Grid
	{
	public:
		Grid(std::shared_ptr<Camera> & camera);
		~Grid();

	public:
		void Draw(Shaders::ID id);

	private:
		void SetupGrid();

	private:
		std::shared_ptr<Camera> m_camera;
		unsigned int m_VAO, m_VBO, m_EBO;
		unsigned int m_width, m_height;
		unsigned int m_vertexCount, m_indexCount;
	};
}

