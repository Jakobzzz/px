#pragma once
#define GLFW_INCLUDE_NONE
#include <entityx\entityx.h>
#include <memory>
#include "Camera.hpp"
#include "Model.hpp"
#include "ResourceIdentifiers.hpp"

//Systems
#include "RenderSystem.hpp"

//Components
#include "Transformable.hpp"
#include "Renderable.hpp"
#include "Pickable.hpp"

using namespace entityx;

namespace px
{
	class Scene : public EntityX
	{
	public:
		Scene();

	public:
		void LoadScene(ModelHolder models);
		void ChangeEntityName(std::string name, std::string newName);
		void CreateEntity(ModelHolder models, Models::ID modelID, RigidBodyType::ID pickShape, std::string name);
		void DestroyEntity(std::string name);
		void UpdatePickedEntity(std::string name, glm::vec3 & position, glm::vec3 & rotation, glm::vec3 & scale, glm::vec3 & color, bool & picked);
		void UpdateSystems(double dt);
		void WriteSceneData();
		void DestroyScene();

	public:
		std::shared_ptr<Camera> GetCamera();
		unsigned int GetEntityCount();
		EntityManager & GetEntities();
		Entity GetEntityByName(std::string name);

	private:
		EntityManager m_entities;
		EventManager m_events;
		SystemManager m_systems;

	private:
		std::shared_ptr<Camera> m_camera;
	};
}

