#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace px
{
	class Camera;
	//This picking is solely based on the fact that the graphical representation
	//of the mesh has almost the same shape/size as the rigidbody of the object (which is why Bullet is used)
	class Picking
	{
	public:
		static void PerformMousePicking(std::shared_ptr<Camera> & camera, float x, float y);

	public:
		static glm::vec3 GetPickingRay();

	private:
		static glm::vec3 m_pickedRay;
	};
}