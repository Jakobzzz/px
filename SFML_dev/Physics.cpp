#include "Physics.hpp"


namespace px
{
	btDiscreteDynamicsWorld* Physics::m_dynamicsWorld;
	BulletDebugDraw* Physics::m_debugDraw;
	btBroadphaseInterface* Physics::m_broadphase;
	btDefaultCollisionConfiguration* Physics::m_collisionConfiguration;
	btCollisionDispatcher* Physics::m_dispatcher;
	btSequentialImpulseConstraintSolver* Physics::m_solver;

	void Physics::Init(std::shared_ptr<Camera> & camera)
	{
		//Build the broadphase
		m_broadphase = new btDbvtBroadphase();

		//Set up the collision configuration and dispatcher
		m_collisionConfiguration = new btDefaultCollisionConfiguration();
		m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

		//The actual physics solver
		m_solver = new btSequentialImpulseConstraintSolver;

		//The world.
		m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
		m_dynamicsWorld->setGravity(btVector3(0, -9.82, 0));

		//Debug draw
		m_debugDraw = new BulletDebugDraw(camera);
		m_debugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

		m_dynamicsWorld->setDebugDrawer(m_debugDraw);
	}

	void Physics::Update()
	{
		m_dynamicsWorld->stepSimulation(1.f / 60.f, 7);

		//If the user presses the play button all rigidbodies (not picking related)
		//will simply be activated enabled in the simulation?
	}

	void Physics::Release()
	{
		delete m_broadphase;
		delete m_collisionConfiguration;
		delete m_debugDraw;
		delete m_dispatcher;
		delete m_solver;
		delete m_dynamicsWorld;
	}

	void Physics::DrawDebug()
	{
		m_dynamicsWorld->debugDrawWorld();
	}

	BulletDebugDraw * Physics::GetDebugDraw()
	{
		return m_debugDraw;
	}

	btScalar Physics::ToBulletScalar(float & scalar)
	{
		return *(btScalar*)&scalar;
	}

	float Physics::ToStandardFloat(btScalar & scalar)
	{
		return (float)scalar;
	}

	btVector3 Physics::ToBulletVector(glm::vec3 & vector)
	{
		return *(btVector3*)&vector;
	}

	glm::vec3 Physics::ToVector3(btVector3 & vector)
	{
		return glm::vec3(vector.x(), vector.y(), vector.z());
	}

	btQuaternion Physics::ToBulletQuaternion(glm::quat & quaternion)
	{
		return *(btQuaternion*)&quaternion;
	}

	glm::quat Physics::ToQuaternion(btQuaternion & quaternion)
	{
		//Flipped coordinate system
		return glm::quat(-quaternion.z(), quaternion.y(), -quaternion.x(), quaternion.w());
	}
}
