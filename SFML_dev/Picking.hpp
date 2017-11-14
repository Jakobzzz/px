#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Physics.hpp"

namespace px
{
	class Camera;

	class Picking
	{
	public:
		static void PerformMousePicking(std::shared_ptr<Camera> & camera, float x, float y);
		static bool RayCast(float distance, btRigidBody* body);

	public:
		static glm::vec3 GetPickingRay();

	private:
		static glm::vec3 m_direction;
		static glm::vec3 m_origin;
	};
}