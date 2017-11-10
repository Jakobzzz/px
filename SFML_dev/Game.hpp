#pragma once
#include "Shader.hpp"
#include "Camera.hpp"
#include "Render.hpp"
#include "Picking.hpp"
#include "Grid.hpp"
#include <entityx\entityx.h>
#include "RenderTexture.hpp"

//Systems
#include "RenderSystem.hpp"

//Components
#include "Transformable.hpp"
#include "Renderable.hpp"

#include <GLFW/glfw3.h>
#include <memory>

using namespace entityx;

namespace px
{
	class Game : public EntityX
	{
	public:
		Game();
		~Game();

	public:
		void Run();

	private:
		void Update(float dt);
		void Render(double dt);
		void SceneGUI(double dt);
		void LoadShaders();
		void LoadModels();
		void InitScene();
		void InitEntities();
		void UpdateGUI(double dt);
		void UpdateCamera(float dt);
		void WriteSceneData();

	private:
		static void OnFrameBufferResizeCallback(GLFWwindow* window, int width, int height);
		static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void OnMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void OnMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	private:
		struct PickingInfo
		{
			glm::mat4 world;
			glm::vec3 position;
			glm::vec3 rotationAngles;
			glm::vec3 scale;
			std::string name;
		};

	private:
		static std::vector<PickingInfo> m_entityPicked;
		static std::shared_ptr<Camera> m_camera;
		static std::string m_pickedName;
		static float m_lastX;
		static float m_lastY;
		static bool m_picked;
		static bool m_showGrid;
		static bool m_showFPS;
		static bool m_showCameraPosition;
		static bool m_hovered;
		static glm::vec3 m_rotationAngles;
		static glm::vec3 m_position;
		static glm::vec3 m_scale;
		static int m_selectedEntity;
		static std::vector<char> m_nameChanger;
			
	private:
		int m_cubeCreationCounter;
		bool* m_open;
		float m_frameTime;
		//char m_nameChanger[100];
		GLFWwindow* m_window;
		std::unique_ptr<Grid> m_grid;
		std::unique_ptr<RenderTexture> m_frameBuffer;
		ModelHolder m_models;

	private:
		//Entities
		EntityManager m_entities;
		EventManager m_events;
		SystemManager m_systems;

	private:
		//Lightning variables
		glm::vec3 m_lightDirection;
		float m_ambient;
		float m_specular;	
	};
}

