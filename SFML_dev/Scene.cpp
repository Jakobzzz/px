#include "Scene.hpp"
#include "Converters.hpp"
#include <json.hpp>
#include <fstream>

//Systems
#include "RenderSystem.hpp"

//Components
#include "Transformable.hpp"
#include "Renderable.hpp"

using json = nlohmann::json;

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

		//Entities
		for (unsigned int i = 0; i < reader["Scene"]["count"]; i++)
		{
			std::string name = reader["Scene"]["names"][i];

			auto entity = m_entities.create();
			auto transform = std::make_unique<Transform>();
			transform->SetPosition(utils::FromVec3Json(reader[name]["position"]));
			transform->SetRotationOnAllAxis(utils::FromVec3Json(reader[name]["rotation"]));
			transform->SetScale(utils::FromVec3Json(reader[name]["scale"]));

			auto render = std::make_unique<px::Render>(models, Models::Cube, Shaders::Phong, name); //Only cube models right now
			entity.assign<Transformable>(transform);
			entity.assign<Renderable>(render);
		}

		//Systems
		m_systems.add<RenderSystem>();
		m_systems.configure();
	}

	void Scene::ChangeEntityName(std::string name, std::string newName)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;

		for (Entity & entity : m_entities.entities_with_components(transform, renderable))
		{
			if (name == renderable->object->GetName())
			{
				renderable->object->SetName(newName);
				break;
			}
		}
	}

	void Scene::CreateEntity(ModelHolder models, Models::ID modelID, std::string name)
	{
		//Create default entity with transform and render component
		auto entity = m_entities.create();
		auto transform = std::make_unique<Transform>();
		auto render = std::make_unique<px::Render>(models, modelID, Shaders::Phong, name); //One shader right now
		entity.assign<Transformable>(transform);
		entity.assign<Renderable>(render);
	}

	void Scene::DestroyEntity(std::string name)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;

		//Remove entity which corresponds to the name
		for (Entity & entity : m_entities.entities_with_components(transform, renderable))
		{
			if (name == renderable->object->GetName())
			{
				m_entities.destroy(entity.id());
				break;
			}
		}
	}

	//This approach isn't the cleanest but I can't figure a way to overcome the static need for callback functions
	//Must fix this in the future!
	void Scene::UpdatePickedEntity(std::string name, glm::vec3 & position, glm::vec3 & rotation, glm::vec3 & scale, std::vector<PickingInfo>& info)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;

		//Update entities transformation
		int i = 0;
		for (Entity & entity : m_entities.entities_with_components(transform, renderable))
		{
			if (name == renderable->object->GetName()) //&& picked
			{
				//Apply transform from GUI to picked object
				transform->transform->SetPosition(position);
				transform->transform->SetRotationOnAllAxis(rotation);
				transform->transform->SetScale(scale);
			}
			else
				transform->transform->SetTransform();

			info[i].position = transform->transform->GetPosition();
			info[i].rotationAngles = transform->transform->GetRotationAngles();
			info[i].scale = transform->transform->GetScale();
			info[i].world = transform->transform->GetTransform();
			info[i].name = renderable->object->GetName();
			i++;
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

		for (Entity & entity : m_entities.entities_with_components(transform, renderable))
		{
			data["Scene"]["names"][i] = renderable->object->GetName();
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
		for (Entity & entity : m_entities.entities_with_components<Transformable, Renderable>())
			entity.destroy();
	}

	std::shared_ptr<Camera> Scene::GetCamera()
	{
		return m_camera;
	}

	unsigned int Scene::GetEntityCount()
	{
		return m_entities.size();
	}
}
