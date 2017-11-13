#pragma once

#include "BulletDebugDraw.hpp"

namespace px
{
	class Physics
	{
	public:
		static void Init(std::shared_ptr<Camera> & camera);
		static void Update();
		static void Release();
		static void DrawDebug();

	public:
		static BulletDebugDraw* GetDebugDraw();

	public:
		static btScalar ToBulletScalar(float & scalar);
		static float ToStandardFloat(btScalar & scalar);
		static btVector3 ToBulletVector(glm::vec3 & vector);
		static glm::vec3 ToVector3(btVector3 & vector);
		static btQuaternion ToBulletQuaternion(glm::quat & quaternion);
		static glm::quat ToQuaternion(btQuaternion & quaternion);

	public:
		static btDiscreteDynamicsWorld* m_dynamicsWorld;

	private:
		static BulletDebugDraw* m_debugDraw;
		static btBroadphaseInterface* m_broadphase;
		static btDefaultCollisionConfiguration* m_collisionConfiguration;
		static btCollisionDispatcher* m_dispatcher;
		static btSequentialImpulseConstraintSolver* m_solver;
	};
}

