#pragma once
#include "Physics.hpp"

namespace px
{
	namespace PickingType
	{
		enum ID
		{
			Box,
			Sphere,
			Capsule,
			Cylinder
		};
	}

	class PickingBody
	{
	public:
		PickingBody(PickingType::ID id);
		
	public:
		void DestroyBody();

	public:
		void SetTransform(glm::vec3 position, glm::vec3 scale, glm::quat orientation);

	public:
		btRigidBody* GetRigidBody() const;
		PickingType::ID GetPickingType() const;

	private:
		void CreateBody();

	private:
		btCollisionShape* m_shape;
		btDefaultMotionState* m_motionState;
		btRigidBody* m_rigidBody;
		PickingType::ID m_pickingType;
	};
}

