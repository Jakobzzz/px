#pragma once
#include "Physics.hpp"

namespace px
{
	namespace PickingType
	{
		enum ID
		{
			Box
		};
	}

	class PickingBody
	{
	public:
		PickingBody(PickingType::ID id, glm::vec3 position, glm::vec3 scale, glm::quat orientation);
		
	public:
		void DestroyBody();

	public:
		void SetTransform(glm::vec3 position, glm::vec3 scale, glm::quat orientation);

	public:
		btRigidBody* GetRigidBody();

	private:
		btCollisionShape* m_shape;
		btDefaultMotionState* m_motionState;
		btRigidBody* m_rigidBody;
	};
}

