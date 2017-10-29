#include "Picking.hpp"
#include "Camera.hpp"
#include <iostream>

namespace px
{
	glm::vec3 Picking::m_pickedRay;

	void Picking::PerformMousePicking(std::shared_ptr<Camera>& camera, float x, float y)
	{
		//Normalize mouse coordinates
		float normX = (2.0f * x) / (float)camera->GetWidth() - 1.f;
		float normY = (2.0f * y) / (float)camera->GetHeight() - 1.f;
		normY = -normY;

		//Translate to clip space
		glm::vec4 clipCoords = glm::vec4(normX, normY, -1.0f, 1.0f);

		//Translate to eye space
		glm::mat4 invertedProjection = glm::inverse(camera->GetProjectionMatrix());
		glm::vec4 eyeCoords = invertedProjection * clipCoords;
		glm::vec4 result = glm::vec4(eyeCoords.x, eyeCoords.y, -1.f, 0.f);

		//Translate to world space
		glm::mat4 invertedView = glm::inverse(camera->GetViewMatrix());
		glm::vec4 rayWorld = invertedView * result;

		//Store picked ray
		m_pickedRay = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
		m_pickedRay = glm::normalize(m_pickedRay);
	}

	//btRigidBody* Picking::RayCast(const btVector3 & startPosition, const btVector3 & direction)
	//{		
	//	Get the picking ray from where we clicked
	//	btVector3 rayFrom = startPosition;
	//	btVector3 rayTo = rayFrom + direction * FAR_PLANE;

	//	Create our raycast callback object
	//	btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);

	//	Perform the raycast
	//	Physics::m_dynamicsWorld->rayTest(rayFrom, rayTo, rayCallback);

	//	if (rayCallback.hasHit())
	//	{
	//		btRigidBody* pBody = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
	//		return pBody;
	//	}

	//	return nullptr;
	//}

	glm::vec3 Picking::GetPickingRay()
	{
		return m_pickedRay;
	}
}
