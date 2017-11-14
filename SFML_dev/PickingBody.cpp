#include "PickingBody.hpp"

namespace px
{
	PickingBody::PickingBody(PickingType::ID id, glm::vec3 position, glm::vec3 scale, glm::quat orientation) : m_pickingType(id)
	{
		switch (id)
		{
		case px::PickingType::Box:
			m_shape = new btBoxShape(Physics::ToBulletVector(scale));
			CreateBody(position, orientation);
			break;
		case px::PickingType::Sphere:
			m_shape = new btSphereShape(Physics::ToBulletScalar(scale.x)); //Radius
			CreateBody(position, orientation);
			break;
		case px::PickingType::Capsule:
			m_shape = new btCapsuleShape(Physics::ToBulletScalar(scale.x), Physics::ToBulletScalar(scale.y)); //Radius and height
			CreateBody(position, orientation);
			break;
		case px::PickingType::Cylinder:
			m_shape = new btCylinderShape(Physics::ToBulletVector(scale));
			CreateBody(position, orientation);
			break;
		default:
			break;
		}
	}

	void PickingBody::CreateBody(glm::vec3 position, glm::quat orientation)
	{
		//Create rigidbody from position and rotation
		m_motionState = new btDefaultMotionState(btTransform(Physics::ToBulletQuaternion(orientation), Physics::ToBulletVector(position)));
		btRigidBody::btRigidBodyConstructionInfo CI(0, m_motionState, m_shape);
		m_rigidBody = new btRigidBody(CI);
		m_rigidBody->setActivationState(DISABLE_SIMULATION); //We don't want the pickshape to be active
		Physics::m_dynamicsWorld->addRigidBody(m_rigidBody);
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

	PickingType::ID PickingBody::GetPickingType() const
	{
		return m_pickingType;
	}

	btRigidBody * PickingBody::GetRigidBody() const
	{
		return m_rigidBody;
	}
}