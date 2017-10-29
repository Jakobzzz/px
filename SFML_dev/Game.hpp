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

	private:
		static void OnFrameBufferResizeCallback(GLFWwindow* window, int width, int height);
		static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void OnMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void OnMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	private:
		static std::shared_ptr<Camera> m_camera;

		//Picking related
		std::string m_pickedName;

	private:
		float m_frameTime;
		GLFWwindow* m_window;
		std::unique_ptr<Grid> m_grid;
		std::unique_ptr<RenderTexture> m_frameBuffer;
		static bool m_hovered;

	private:
		EntityManager m_entities;
		EventManager m_events;
		SystemManager m_systems;
		ModelHolder m_models;

	private:
		Entity m_cubeEntity;
		Entity m_planeEntity;
		glm::vec3 m_rotationAngles;
		glm::vec3 m_position;
		glm::vec3 m_scale;
		float m_colors[3];
		bool* m_open;

	private:
		static bool m_showGrid;
		static bool m_showFPS;
		static bool m_showCameraPosition;

	private:
		//Lightning variables
		glm::vec3 m_lightDirection;
		float m_ambient;
		float m_specular;

	private:
		//Callback variables
		static float m_lastX;
		static float m_lastY;
		static int m_newX;
		static int m_newY;
	};
}

