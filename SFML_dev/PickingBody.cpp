#include "PickingBody.hpp"

namespace px
{
	PickingBody::PickingBody(PickingType::ID id, glm::vec3 position, glm::vec3 scale, glm::quat orientation)
	{
		//Only box type supported at the moment
		if (id == PickingType::Box)
		{
			m_shape = new btBoxShape(Physics::ToBulletVector(scale));
			m_motionState = new btDefaultMotionState(btTransform(Physics::ToBulletQuaternion(orientation), Physics::ToBulletVector(position)));
			btRigidBody::btRigidBodyConstructionInfo CI(0, m_motionState, m_shape);
			m_rigidBody = new btRigidBody(CI);
			m_rigidBody->setActivationState(DISABLE_SIMULATION); //Should only perform picking on entities with this flag
			Physics::m_dynamicsWorld->addRigidBody(m_rigidBody);
		}

		//...
	}

	void PickingBody::DestroyBody()
	{
		Physics::m_dynamicsWorld->removeRigidBody(m_rigidBody);
		delete m_rigidBody->getMotionState();
		delete m_rigidBody;
	}

	void PickingBody::SetTransform(glm::vec3 position, glm::vec3 scale, glm::quat orientation)
	{
		btTransform trans;
		trans.setOrigin(Physics::ToBulletVector(position));
		trans.setRotation(Physics::ToBulletQuaternion(orientation));
		m_rigidBody->getCollisionShape()->setLocalScaling(Physics::ToBulletVector(scale));

		m_rigidBody->setWorldTransform(trans);
	}

	btRigidBody * PickingBody::GetRigidBody()
	{
		return m_rigidBody;
	}
}