//#pragma once
//
//#include "Physics.hpp"
//
//namespace px
//{
//	//This picking is solely based on the fact that the graphical representation
//	//of the mesh has almost the same shape/size as the rigidbody of the object (which is why Bullet is used)
//	class Picking
//	{
//	public:
//		static btRigidBody* PerformMousePicking(std::shared_ptr<Camera> & camera, float x, float y);
//
//	public:
//		static glm::vec3 GetPickingRay();
//
//	private:
//		static btRigidBody* RayCast(const btVector3 &startPosition, const btVector3 &direction);
//
//	private:
//		static glm::vec3 m_pickedRay;
//	};
//}