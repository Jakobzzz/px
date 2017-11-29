#pragma once
#include "Physics.hpp"

namespace px
{
	class PickingBody
	{
	public:
		PickingBody(RigidBodyType::ID id);
		
	public:
		void DestroyBody();

	public:
		void SetTransform(glm::vec3 position, glm::vec3 scale, glm::quat orientation);

	public:
		btRigidBody* GetRigidBody() const;
		RigidBodyType::ID GetPickingType() const;

	private:
		void CreateBody();

	private:
		btCollisionShape* m_shape;
		btDefaultMotionState* m_motionState;
		btRigidBody* m_rigidBody;
		RigidBodyType::ID m_pickingType;
	};
}

