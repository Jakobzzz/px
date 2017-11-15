#include "Scene.hpp"
#include "Converters.hpp"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

//TODO: modify this class for pickable component

namespace px
{
	Scene::Scene() : m_entities(m_events), m_systems(m_entities, m_events)
	{
	}

	void Scene::LoadScene(ModelHolder models)
	{
		//Read scene data from json file
		std::ifstream i("Scripts/Json/scene.json");
		json reader; i >> reader; i.close();

		//Camera
		if (reader["Camera"]["count"] == 1) //Prevent crash if the scene file doesn't have camera data
		{
			glm::vec3 cameraPos = utils::FromVec3Json(reader["Camera"]["position"]);
			m_camera = std::make_shared<Camera>(cameraPos, reader["Camera"]["yaw"], reader["Camera"]["pitch"]);
		}
		else
			m_camera = std::make_shared<Camera>();

		//Physics
		Physics::Init(m_camera);

		//Entities
		for (unsigned int i = 0; i < reader["Scene"]["count"]; i++)
		{
			std::string name = reader["Scene"]["names"][i];

			//Transform component
			auto entity = m_entities.create();
			auto transform = std::make_unique<Transform>();
			transform->SetPosition(utils::FromVec3Json(reader[name]["position"]));
			transform->SetRotationOnAllAxis(utils::FromVec3Json(reader[name]["rotation"]));
			transform->SetScale(utils::FromVec3Json(reader[name]["scale"]));

			//Picking component
			PickingType::ID id = reader[name]["pickingType"];
			auto pickable = std::make_unique<px::PickingBody>(id);
			pickable->SetTransform(utils::FromVec3Json(reader[name]["position"]), utils::FromVec3Json(reader[name]["scale"]),
			transform->GetOrientation());

			//Render component
			auto render = std::make_unique<px::Render>(models, reader[name]["model"], Shaders::Phong, name); //Only cube models right now;

			entity.assign<Transformable>(transform);
			entity.assign<Renderable>(render);
			entity.assign<Pickable>(pickable);
		}

		//Systems
		m_systems.add<RenderSystem>();
		m_systems.configure();
	}

	void Scene::ChangeEntityName(std::string name, std::string newName)
	{
		ComponentHandle<Renderable> renderable;

		//TODO: make sure that the chosen name doesn't already exist!
		for (Entity & entity : m_entities.entities_with_components(renderable))
		{
			if (name == renderable->object->GetName())
			{
				renderable->object->SetName(newName);
				break;
			}
		}
	}

	void Scene::CreateEntity(ModelHolder models, Models::ID modelID, PickingType::ID pickShape, std::string name)
	{
		//Create entity at the origin
		auto entity = m_entities.create();
		auto transform = std::make_unique<Transform>();
		auto render = std::make_unique<px::Render>(models, modelID, Shaders::Phong, name); //One shader right now
		auto pickable = std::make_unique<px::PickingBody>(pickShape);

		entity.assign<Transformable>(transform);
		entity.assign<Renderable>(render);
		entity.assign<Pickable>(pickable);
	}

	void Scene::DestroyEntity(std::string name)
	{
		ComponentHandle<Renderable> renderable;
		ComponentHandle<Pickable> pickable;

		//Remove entity which corresponds to the name
		for (Entity & entity : m_entities.entities_with_components(renderable, pickable))
		{
			if (name == renderable->object->GetName())
			{
				pickable->object->DestroyBody();
				m_entities.destroy(entity.id());
				break;
			}
		}
	}

	void Scene::UpdatePickedEntity(std::string name, glm::vec3 & position, glm::vec3 & rotation, glm::vec3 & scale, glm::vec3 & color, bool & picked)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;
		ComponentHandle<Pickable> pickable;

		//Update entities transformation
		for (Entity & entity : m_entities.entities_with_components(transform, renderable, pickable))
		{
			if (name == renderable->object->GetName() && picked)
			{
				//Apply changes from GUI to picked object
				renderable->object->SetColor(color);
				transform->transform->SetPosition(position);
				transform->transform->SetRotationOnAllAxis(rotation);
				transform->transform->SetScale(scale);
				pickable->object->SetTransform(position, scale, transform->transform->GetOrientation());
			}
			else
				transform->transform->SetTransform();
		}
	}

	void Scene::UpdateSystems(double dt)
	{
		m_systems.update<RenderSystem>(dt);
	}

	void Scene::WriteSceneData()
	{
		//In the future, this function will also write the modelID/Shaders
		//Write scene information to json file
		json data;
		data["Scene"]["count"] = m_entities.size();

		data["Camera"]["count"] = 1;
		data["Camera"]["position"] = utils::ToVec3Json(m_camera->GetPosition());
		data["Camera"]["yaw"] = m_camera->GetYaw();
		data["Camera"]["pitch"] = m_camera->GetPitch();

		int i = 0;
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;
		ComponentHandle<Pickable> pickable;

		for (Entity & entity : m_entities.entities_with_components(transform, renderable, pickable))
		{
			data["Scene"]["names"][i] = renderable->object->GetName();
			data[renderable->object->GetName()]["pickingType"] = pickable->object->GetPickingType();
			data[renderable->object->GetName()]["model"] = renderable->object->GetModel();
			data[renderable->object->GetName()]["position"] = utils::ToVec3Json(transform->transform->GetPosition());
			data[renderable->object->GetName()]["rotation"] = utils::ToVec3Json(transform->transform->GetRotationAngles());
			data[renderable->object->GetName()]["scale"] = utils::ToVec3Json(transform->transform->GetScale());
			i++;
		}

		//Dump information
		std::ofstream o("Scripts/Json/scene.json");
		o << std::setw(3) << data << std::endl;
	}

	void Scene::DestroyScene()
	{
		ComponentHandle<Pickable> pickable;

		for (Entity & entity : m_entities.entities_with_components(pickable))
		{
			pickable->object->DestroyBody();
			entity.destroy();
		}
	}

	std::shared_ptr<Camera> Scene::GetCamera()
	{
		return m_camera;
	}

	unsigned int Scene::GetEntityCount()
	{
		return m_entities.size();
	}

	EntityManager & Scene::GetEntities()
	{
		return m_entities;
	}
}
