#pragma once
#include "Picking.hpp"
#include "Grid.hpp"
#include "RenderTexture.hpp"
#include "Scene.hpp"

#include <GLFW/glfw3.h>
#include <memory>

namespace px
{
	class Game
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
		void UpdateGUI(double dt);
		void UpdateCamera(float dt);

	private:
		static void OnFrameBufferResizeCallback(GLFWwindow* window, int width, int height);
		static void OnMouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void OnMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		//static void OnMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	private:
		struct Material
		{
			char* name;
			glm::vec3 color;
		};

	private:
		static std::unique_ptr<Scene> m_scene;

		//GUI related
		static int m_selectedEntity;
		static float m_lastX;
		static float m_lastY;
		static bool m_picked;
		static bool m_showGrid;
		static bool m_showFPS;
		static bool m_showCameraPosition;
		static bool m_hovered;
		static bool m_showDiagnostics;
		static glm::vec3 m_rotationAngles;
		static glm::vec3 m_position;
		static glm::vec3 m_scale;
		static glm::vec3 m_color;
		static std::string m_pickedName;
		static std::vector<char> m_nameChanger;
		static std::vector<Material> m_materials;
			
	private:
		int m_cubeCreationCounter;
		bool* m_open;
		float m_frameTime;
		GLFWwindow* m_window;
		std::unique_ptr<Grid> m_grid;
		std::unique_ptr<RenderTexture> m_frameBuffer;
		ModelHolder m_models;

	private:
		//Lightning variables
		glm::vec3 m_lightDirection;
		float m_ambient;
		float m_specular;	
	};
}

