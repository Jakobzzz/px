#include "Game.hpp"
#include "imguidock.h"
#include "imgui_impl_glfw_gl3.h"
#include "imgui_log.h"
#include "imgui_console.h"

#include <assert.h>
#include <iostream>
#include <functional>
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace px
{
	std::shared_ptr<Camera> Game::m_camera;
	float Game::m_lastX = (float)WINDOW_WIDTH / 2.f;
	float Game::m_lastY = (float)WINDOW_HEIGHT / 2.f;
	bool Game::m_hovered = false;
	bool Game::m_showGrid = true;
	bool Game::m_showFPS = false;
	bool Game::m_showCameraPosition = true;
	bool Game::m_picked = false;
	glm::vec3 Game::m_rotationAngles;
	glm::vec3 Game::m_position;
	glm::vec3 Game::m_scale;
	std::string Game::m_pickedName;
	std::vector<Game::PickingInfo> Game::m_entityPicked;

	Game::Game() : m_frameTime(0.f), m_entities(m_events), m_systems(m_entities, m_events), m_cubeCreationCounter(0)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pixel Engine", nullptr, nullptr);
		assert(m_window != nullptr);

		glfwSetWindowPos(m_window, 100, 75);
		glfwMakeContextCurrent(m_window);

		assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

		//Make sure ImGui doesn't already have the callbacks when adding new ones
		glfwSetFramebufferSizeCallback(m_window, OnFrameBufferResizeCallback);
		glfwSetCursorPosCallback(m_window, OnMouseCallback);

		//ImGUI initialize
		InitImGuiStyle(true, 0.5f);
		ImGui_ImplGlfwGL3_Init(m_window, true);

		//Override imgui mouse button callback
		glfwSetMouseButtonCallback(m_window, OnMouseButtonCallback);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);

		InitScene();

		//Lua functions
		gameConsole.lua.set_function("setCamera", [](float x, float y, float z) { m_camera->SetPosition(glm::vec3(x, y, z)); });
		gameConsole.lua.set_function("print", [] { gameConsole.AddLog("Printed"); });
	}

	Game::~Game()
	{
		for (Entity entity : m_entities.entities_with_components<Transformable, Renderable>())
			entity.destroy();

		WriteSceneData();

		m_models->Destroy(Models::Cube);
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}

	void Game::LoadShaders()
	{
		Shader::LoadShaders(Shaders::Phong, "triangle.vertex", "triangle.fragment");
		Shader::LoadShaders(Shaders::Grid, "grid.vertex", "grid.fragment");
	}

	void Game::LoadModels()
	{
		m_models = std::make_shared<Model<Models::ID>>();
		m_models->LoadModel(Models::Cube, "../res/Models/Cube/cube.obj"); //Check the mesh vector when more objects are added -> clear() or local vector?
	}

	void Game::InitScene()
	{
		LoadShaders();
		LoadModels();

		//Render textures
		m_frameBuffer = std::make_unique<RenderTexture>();

		//Entites
		InitEntities();

		//Grid
		m_grid = std::make_unique<Grid>(m_camera);

		//Lightning
		m_lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
		m_ambient = 0.3f;
		m_specular = 0.2f;

		//Systems
		m_systems.add<RenderSystem>();
		m_systems.configure();
	}

	void Game::InitEntities()
	{
		//Read values from scene json file
		std::ifstream i("Scripts/Json/scene.json");
		json reader; i >> reader; i.close();

		//Camera
		glm::vec3 cameraPos = FromVec3Json(reader["Camera"]["position"]);
		m_camera = std::make_shared<Camera>(cameraPos, reader["Camera"]["yaw"], reader["Camera"]["pitch"]);
	
		//Restore scene data from file
		for (unsigned int i = 0; i < reader["Scene"]["Count"]; i++)
		{
			std::string name = reader["Scene"]["Names"][i];

			auto entity = m_entities.create();
			auto transform = std::make_unique<Transform>();
			transform->SetPosition(FromVec3Json(reader[name]["position"]));
			transform->SetRotationOnAllAxis(FromVec3Json(reader[name]["rotation"]));
			transform->SetScale(FromVec3Json(reader[name]["scale"]));

			auto render = std::make_unique<px::Render>(m_models, Models::Cube, Shaders::Phong, name);
			entity.assign<Transformable>(transform);
			entity.assign<Renderable>(render);
		}
	}

	void Game::Run()
	{
		int frameCount = 0;
		double lastTime = glfwGetTime();
		double deltaTime = 0.0;
		double lastFrame = 0.0;

		while (!glfwWindowShouldClose(m_window))
		{
			double currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			frameCount++;

			if (currentFrame - lastTime >= 1.0)
			{
				m_frameTime = (float)(1000.0 / (double)frameCount);
				frameCount = 0;
				lastTime += 1.0;
			}

			glfwPollEvents();

			UpdateGUI(deltaTime);			
			Update((float)deltaTime);

			//Render IMGUI last
			ImGui::Render();

			glfwSwapBuffers(m_window);
		}
	}

	void Game::Render(double dt)
	{
		//Draw scene as normally to a color texture
		m_frameBuffer->BindFrameBuffer(); 
		glClearColor(0.274f, 0.227f, 0.227f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_showGrid)
			m_grid->Draw(Shaders::Grid);

		Shader::Use(Shaders::Phong);
		Shader::SetMatrix4x4(Shaders::Phong, "projection", m_camera->GetProjectionMatrix());
		Shader::SetMatrix4x4(Shaders::Phong, "view", m_camera->GetViewMatrix());
		Shader::SetFloat3v(Shaders::Phong, "viewpos", m_camera->GetPosition());
		Shader::SetFloat3v(Shaders::Phong, "direction", m_lightDirection);
		Shader::SetFloat(Shaders::Phong, "ambientStrength", m_ambient);
		Shader::SetFloat(Shaders::Phong, "specularStrength", m_specular);

		//Update systems
		m_systems.update<RenderSystem>(dt);
		
		m_frameBuffer->BlitMultiSampledBuffer();
		m_frameBuffer->UnbindFrameBuffer();
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Game::Update(float dt)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;

		//Update entities transformation
		int i = 0;
		m_entityPicked.resize(m_entities.size());
		for (Entity & entity : m_entities.entities_with_components(transform, renderable))
		{
			if (m_pickedName == renderable->object->GetName() && m_picked)
			{
				//Set transform from GUI with picked object
				transform->transform->SetPosition(m_position);
				transform->transform->SetRotationOnAllAxis(m_rotationAngles);
				transform->transform->SetScale(m_scale);
			}
			else
				transform->transform->SetTransform();

			m_entityPicked[i].position = transform->transform->GetPosition();
			m_entityPicked[i].rotationAngles = transform->transform->GetRotationAngles();
			m_entityPicked[i].scale = transform->transform->GetScale();
			m_entityPicked[i].world = transform->transform->GetTransform();
			m_entityPicked[i].name = renderable->object->GetName();
			i++;
		}

		if (m_hovered)
			UpdateCamera(dt);
	}

	void Game::SceneGUI(double dt)
	{
		ImVec2 size = ImGui::GetContentRegionAvail();
		unsigned int width = m_frameBuffer->GetWidth();
		unsigned int height = m_frameBuffer->GetHeight();

		if (width != size.x || height != size.y)
		{
			m_camera->SetWidth((unsigned int)size.x);
			m_camera->SetHeight((unsigned int)size.y);
			m_frameBuffer->ResizeBuffer((unsigned int)size.x, (unsigned int)size.y);
		}

		Render(dt);

		//Draw the image/texture, filling the whole dock window
		ImGui::Image(reinterpret_cast<ImTextureID>(m_frameBuffer->GetTexture()), size, ImVec2(0, 0), ImVec2(1, -1));
		m_hovered = ImGui::IsItemHovered();
	}

	void Game::UpdateGUI(double dt)
	{
		int floatPrecision = 3;

		ImGui_ImplGlfwGL3_NewFrame();

		//Placeholder menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New")) {}
				if (ImGui::MenuItem("Open", "Ctrl+O")) {}
				if (ImGui::BeginMenu("Open Recent"))
				{
					ImGui::MenuItem("fish_hat.c");
					ImGui::MenuItem("fish_hat.inl");
					ImGui::MenuItem("fish_hat.h");
					if (ImGui::BeginMenu("More.."))
					{
						ImGui::MenuItem("Hello");
						ImGui::MenuItem("Sailor");
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Disabled", false)) // Disabled
				{
					IM_ASSERT(0);
				}
				if (ImGui::MenuItem("Quit", "Alt+F4")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Show Grid", NULL, &m_showGrid)) {}
				if (ImGui::MenuItem("Show FPS", NULL, &m_showFPS)) {}
				if (ImGui::MenuItem("Show Position", NULL, &m_showCameraPosition)) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::BeginMenu("3D Object"))
				{
					if(ImGui::MenuItem("Cube"))
					{
						auto entity = m_entities.create();
						auto transform = std::make_unique<Transform>();
						auto render = std::make_unique<px::Render>(m_models, Models::Cube, Shaders::Phong, "Cube" + std::to_string(m_cubeCreationCounter));
						entity.assign<Transformable>(transform);
						entity.assign<Renderable>(render);
						m_cubeCreationCounter++;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		//Camera position overlay at the bottom of the scene
		if (m_showCameraPosition)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 215, WINDOW_HEIGHT - 480));
			if (!ImGui::Begin("Camera overlay", &m_showCameraPosition, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
																						   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
			{
				ImGui::End();
				return;
			}

			ImGui::Text("(%.3f, %.3f, %.3f)      ", m_camera->GetPosition().x, m_camera->GetPosition().y, m_camera->GetPosition().z);
			ImGui::End();		
		}
		
		//FPS overlay on right of the screen
		if (m_showFPS)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 230, WINDOW_HEIGHT - 840));
			if (!ImGui::Begin("FPS overlay", &m_showFPS, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
																			 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
			{
				ImGui::End();
				return;
			}

			ImGui::TextColored(ImVec4(0.f, 1.0f, 0.0f, 1.0f), "%.3f ms/frame (%.1f FPS)  ", m_frameTime, (1 / m_frameTime) * 1000);
			ImGui::End();
		}

		//Docking system
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		const ImGuiWindowFlags flags = (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
										ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | 
										ImGuiWindowFlags_NoTitleBar);
		const float oldWindowRounding = ImGui::GetStyle().WindowRounding; ImGui::GetStyle().WindowRounding = 0;
		const bool visible = ImGui::Begin("Pixel Engine", NULL, ImVec2(0, 0), 1.0f, flags);
		ImGui::GetStyle().WindowRounding = oldWindowRounding;
		ImGui::SetWindowPos(ImVec2(0, 10));

		if (visible) 
		{
			ImGui::BeginDockspace();

			ImGui::SetNextDock(ImGuiDockSlot_Left);
			if (ImGui::BeginDock("#Scene"))
			{
				SceneGUI(dt);
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Bottom);
			if (ImGui::BeginDock("Debug"))
			{				
				ImGui::Spacing();
				if (ImGui::CollapsingHeader("Directional Light"))
				{
					ImGui::Spacing();
					ImGui::Text("Direction");
					ImGui::InputFloat3("Direction", (float*)&m_lightDirection, floatPrecision);
					ImGui::Spacing();
					ImGui::Text("Phong Shading");
					ImGui::SliderFloat("Ambient", &m_ambient, 0.0f, 1.0f);
					ImGui::SliderFloat("Specular", &m_specular, 0.0f, 1.0f);
				}				
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Tab);
			if (ImGui::BeginDock("Inspector"))
			{
				if (m_picked)
				{
					std::string info = "Entity: " + m_pickedName;
					ImGui::Text(info.c_str());
					ImGui::Spacing();

					ImGui::SetNextTreeNodeOpen(true, 2);
					if (ImGui::CollapsingHeader("Transform"))
					{
						ImGui::Spacing();
						ImGui::InputFloat3("Position", (float*)&m_position, floatPrecision);
						ImGui::Spacing();
						ImGui::InputFloat3("Rotation", (float*)&m_rotationAngles, floatPrecision);
						ImGui::Spacing();
						ImGui::InputFloat3("Scale", (float*)&m_scale, floatPrecision);
					}
				}
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Left);
			if (ImGui::BeginDock("Entities"))
			{
				static int selected = 0;
				ImGui::BeginChild("Hierachy");
				for (unsigned int i = 0; i < m_entityPicked.size(); i++)
				{
					char label[128];
					sprintf(label, m_entityPicked[i].name.c_str());
					if (ImGui::Selectable(label, selected == i))
					{
						//Give information to GUI about picked object
						m_pickedName = m_entityPicked[i].name;
						m_scale = m_entityPicked[i].scale;
						m_position = m_entityPicked[i].position;
						m_rotationAngles = m_entityPicked[i].rotationAngles;
						m_picked = true;

						selected = i;						
					}
				}
				ImGui::EndChild();
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Bottom);
			if (ImGui::BeginDock("Log"))
			{
				gameLog.Draw();
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Tab);
			if (ImGui::BeginDock("Console"))
			{
				gameConsole.Draw();
			}
			ImGui::EndDock();

			ImGui::EndDockspace();
		}
		ImGui::End();		
	}

	void Game::UpdateCamera(float dt)
	{
		//Camera movement
		if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
			m_camera->ProcessKeyboard(FORWARD, dt);
		if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
			m_camera->ProcessKeyboard(BACKWARD, dt);
		if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
			m_camera->ProcessKeyboard(RIGHT, dt);
		if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
			m_camera->ProcessKeyboard(LEFT, dt);
	}

	//Helper functions for reading/writing json files
	glm::vec3 Game::FromVec3Json(std::vector<float> values)
	{
		glm::vec3 result = { values[0], values[1], values[2] };
		return result;
	}

	std::vector<float> Game::ToVec3Json(glm::vec3 values)
	{
		std::vector<float> result = { values[0], values[1], values[2] };
		return result;
	}

	void Game::WriteSceneData()
	{
		//Write scene information to json file
		json data;
		data["Scene"]["Count"] = m_entityPicked.size();

		for (unsigned int i = 0; i < m_entityPicked.size(); i++)
		{
			data["Scene"]["Names"][i] = m_entityPicked[i].name;
		}

		data["Camera"]["position"] = ToVec3Json(m_camera->GetPosition());
		data["Camera"]["yaw"] = m_camera->GetYaw();
		data["Camera"]["pitch"] = m_camera->GetPitch();

		for (unsigned int i = 0; i < m_entityPicked.size(); i++)
		{
			data[m_entityPicked[i].name]["position"] = ToVec3Json(m_entityPicked[i].position);
			data[m_entityPicked[i].name]["rotation"] = ToVec3Json(m_entityPicked[i].rotationAngles);
			data[m_entityPicked[i].name]["scale"] = ToVec3Json(m_entityPicked[i].scale);
		}

		std::ofstream o("Scripts/Json/scene.json");
		o << std::setw(3) << data << std::endl;
	}

	//*** Callbacks ***
	void Game::OnFrameBufferResizeCallback(GLFWwindow * window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void Game::OnMouseCallback(GLFWwindow * window, double xpos, double ypos)
	{
		if (m_hovered)
		{
			if (m_camera->GetFirstMouse())
			{
				m_lastX = (float)xpos;
				m_lastY = (float)ypos;
			}

			float xoffset = (float)xpos - m_lastX;
			float yoffset = m_lastY - (float)ypos;

			m_lastX = (float)xpos;
			m_lastY = (float)ypos;
			
			m_camera->ProcessMouseMovement(window, xoffset, yoffset);
		}
	}

	void Game::OnMouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
	{
		if ((button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS))
		{
			//Width formula for new resize 
			//(window_width - new_width) / 2
			//Height formula (approximation)
			//(new_height / 8)

			//Picking: project 2D-position to 3D and check intersection with OBB
			if (m_hovered)
			{
				Picking::PerformMousePicking(m_camera, m_lastX - 16, m_lastY - 50);

				//TODO: change so the hierachy list also updates when picking an object!
				for (unsigned int i = 0; i < m_entityPicked.size(); i++)
				{
					if (Picking::RayOBBIntersection(glm::vec3(-1.f), glm::vec3(1.f), m_entityPicked[i].world))
					{
						//Give information to GUI about picked object
						m_pickedName = m_entityPicked[i].name;
						m_scale = m_entityPicked[i].scale;
						m_position = m_entityPicked[i].position;
						m_rotationAngles = m_entityPicked[i].rotationAngles;
						m_picked = true;
						gameLog.Print("Picked\n");
						break;
					}
					else
						m_picked = false;
				}
			}
		}
	}

	void Game::OnMouseScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
	{
		float fov = m_camera->GetFov();

		if (m_hovered)
		{
			if (fov >= 1.0f && fov <= 90.0f)
			{
				fov -= (float)(5.0 * yoffset);
				m_camera->SetFov(fov);
			}

			if (fov <= 1.0f)
			{
				fov = 1.0f;
				m_camera->SetFov(fov);
			}

			if (fov >= 90.0f)
			{
				fov = 90.0f;
				m_camera->SetFov(fov);
			}
		}
	}
}
