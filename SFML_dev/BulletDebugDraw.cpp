#include "BulletDebugDraw.hpp"
#include <glad/glad.h>
#include "Camera.hpp"
#include "Shader.hpp"

namespace px
{
	BulletDebugDraw::BulletDebugDraw(std::shared_ptr<Camera> & camera) : m_camera(camera)
	{
		//Initial data
		m_lines.push_back({ glm::vec3(1.f, 0.f, 0.f) }); //Line start
		m_lines.push_back({ glm::vec3(1.f, 0.f, 0.f) }); //Line end

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);

		glBindVertexArray(m_VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_lines.size() * sizeof(LineVertex), &m_lines[0], GL_DYNAMIC_DRAW);

		//Positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
		glEnableVertexAttribArray(0);

		//Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	BulletDebugDraw::~BulletDebugDraw()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
	}

	void BulletDebugDraw::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & fromColor, const btVector3 & toColor)
	{
		Shader::Use(Shaders::Debug);

		m_lines[0].pos = glm::vec3(from.x(), from.y(), from.z());
		m_lines[0].color = glm::vec3(fromColor.x(), fromColor.y(), fromColor.z());
		m_lines[1].pos = glm::vec3(to.x(), to.y(), to.z());
		m_lines[1].color = glm::vec3(toColor.x(), toColor.y(), toColor.z());

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_lines.size() * sizeof(LineVertex), &m_lines[0], GL_DYNAMIC_DRAW);

		//Positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
		glEnableVertexAttribArray(0);

		//Colors
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));

		Shader::SetMatrix4x4(Shaders::Debug, "model", glm::mat4());
		Shader::SetMatrix4x4(Shaders::Debug, "projection", m_camera->GetProjectionMatrix());
		Shader::SetMatrix4x4(Shaders::Debug, "view", m_camera->GetViewMatrix());

		glBindVertexArray(m_VAO);
		glDrawArrays(GL_LINES, 0, m_lines.size());
		glBindVertexArray(0);
	}

	void BulletDebugDraw::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & color)
	{
		drawLine(from, to, color, color);
	}

	void BulletDebugDraw::drawSphere(const btVector3 & p, btScalar radius, const btVector3 & color)
	{

	}

	void BulletDebugDraw::drawTriangle(const btVector3 & a, const btVector3 & b, const btVector3 & c, const btVector3 & color, btScalar alpha)
	{
	}

	void BulletDebugDraw::drawContactPoint(const btVector3 & PointOnB, const btVector3 & normalOnB, btScalar distance, int lifeTime, const btVector3 & color)
	{
	}

	void BulletDebugDraw::reportErrorWarning(const char * warningString)
	{
	}

	void BulletDebugDraw::draw3dText(const btVector3 & location, const char * textString)
	{
	}

	void BulletDebugDraw::setDebugMode(int debugMode)
	{
		m_debugMode = debugMode;
	}

}