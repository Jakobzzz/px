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
		std::string GenerateName(std::string nameType);

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

		//Struct for providing information about an entity to the GUI
		struct EntityInformation
		{
			int selectedEntity;
			bool picked;
			std::string pickedName;
			glm::vec3 rotationAngles;
			glm::vec3 position;
			glm::vec3 scale;
			glm::vec3 color;
			std::vector<char> nameChanger;
		};

		//Struct for managing display settings in the GUI
		struct DisplayInformation
		{
			bool showGrid;
			bool showFPS;
			bool showCameraPosition;
			bool hovered;
			bool showDiagnostics;
			bool showDebugDraw;
		};

	private:
		static std::unique_ptr<Scene> m_scene;

		//GUI related
		static DisplayInformation m_displayInfo;
		static EntityInformation m_info;
		static float m_lastX;
		static float m_lastY;;
		static std::vector<Material> m_materials;
			
	private:
		int m_creationCounter;
		bool* m_open;
		float m_frameTime;
		GLFWwindow* m_window;
		std::vector<char*> m_materialNames;
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

