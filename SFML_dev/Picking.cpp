#include "Picking.hpp"
#include "Camera.hpp"
#include <iostream>

#define EPSILON 1.0e-25F

namespace px
{
	glm::vec3 Picking::m_direction;
	glm::vec3 Picking::m_origin;

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
		m_origin = camera->GetPosition();
		m_direction = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
		m_direction = glm::normalize(m_direction);
	}

	bool Picking::RayCast(float distance, btRigidBody * body)
	{
		//Raycast with bullet for intersection testing
		if (Physics::m_dynamicsWorld == nullptr)
			return false;

		glm::vec3 rayEnd = m_origin + m_direction * distance;
		btCollisionWorld::ClosestRayResultCallback RayCallback(Physics::ToBulletVector(m_origin), Physics::ToBulletVector(rayEnd));
		Physics::m_dynamicsWorld->rayTest(Physics::ToBulletVector(m_origin), Physics::ToBulletVector(rayEnd), RayCallback);

		if (RayCallback.hasHit())
		{
			btRigidBody* pickedBody = (btRigidBody*)btRigidBody::upcast(RayCallback.m_collisionObject);
			if(pickedBody == body && body->getActivationState() == DISABLE_SIMULATION)
				return true;
		}
		else
			return false;

		return false;
	}


	glm::vec3 Picking::GetPickingRay()
	{
		return m_direction;
	}
}
