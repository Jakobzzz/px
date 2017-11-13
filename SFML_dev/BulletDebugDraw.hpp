#pragma once
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>

namespace px
{
	class Camera;
	class Shader;

	class BulletDebugDraw : public btIDebugDraw
	{
	public:
		BulletDebugDraw(std::shared_ptr<Camera> & camera);
		~BulletDebugDraw();

	public:
		//Override Bullets debug draw functions
		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor);
		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
		virtual void drawSphere(const btVector3& p, btScalar radius, const btVector3& color);
		virtual void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha);
		virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
		virtual void reportErrorWarning(const char* warningString);
		virtual void draw3dText(const btVector3& location, const char* textString);
		virtual void setDebugMode(int debugMode);
		virtual int getDebugMode() const { return m_debugMode; }

	private:
		struct LineVertex
		{
			glm::vec3 pos;
			glm::vec3 color;
		};

		unsigned int m_VAO, m_VBO;
		int m_debugMode;
		std::vector<LineVertex> m_lines;
		std::shared_ptr<Camera> m_camera;
	};

}
