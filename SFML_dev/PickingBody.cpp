#include "PickingBody.hpp"
#include <iostream>

namespace px
{
	PickingBody::PickingBody(RigidBodyType::ID id) : m_pickingType(id)
	{
		switch (id)
		{
		case px::RigidBodyType::Box:
			m_shape = new btBoxShape(Physics::ToBulletVector(glm::vec3(1.f)));
			CreateBody();
			break;
		case px::RigidBodyType::Sphere:
			m_shape = new btSphereShape(1); //Radius
			CreateBody();
			break;
		case px::RigidBodyType::Capsule: //A bit weird at the moment, don't know the exact dimensions conversion
			m_shape = new btCapsuleShape(1, 1); //Radius and height
			CreateBody();
			break;
		case px::RigidBodyType::Cylinder:
			m_shape = new btCylinderShape(Physics::ToBulletVector(glm::vec3(1.f)));
			CreateBody();
			break;
		default:
			break;
		}
	}

	//Note: this function always creates a rigidbody at the origin because of
	//the local scaling problem associated with the collision body
	//Thus, you will call setTransform() afterwards
	void PickingBody::CreateBody()
	{
		m_motionState = new btDefaultMotionState(btTransform(Physics::ToBulletQuaternion(glm::quat()), Physics::ToBulletVector(glm::vec3())));
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
		delete m_shape;
	}

	void PickingBody::SetTransform(glm::vec3 position, glm::vec3 scale, glm::quat orientation)
	{
		btTransform trans;
		trans.setOrigin(Physics::ToBulletVector(position));
		trans.setRotation(Physics::ToBulletQuaternion(orientation));
		m_rigidBody->getCollisionShape()->setLocalScaling(Physics::ToBulletVector(scale));
		m_rigidBody->setWorldTransform(trans);
	}

	RigidBodyType::ID PickingBody::GetPickingType() const
	{
		return m_pickingType;
	}

	btRigidBody * PickingBody::GetRigidBody() const
	{
		return m_rigidBody;
	}
}