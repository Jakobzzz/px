#include "Game.hpp"
#include "imguidock.h"
#include "imgui_impl_glfw_gl3.h"
#include "imgui_log.h"
#include "imgui_console.h"

#include <assert.h>
#include <iostream>
#include <functional>

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

namespace px
{
	//Static functions
	float Game::m_lastX = (float)WINDOW_WIDTH / 2.f;
	float Game::m_lastY = (float)WINDOW_HEIGHT / 2.f;
	bool Game::m_hovered = false;
	bool Game::m_showGrid = true;
	bool Game::m_showFPS = false;
	bool Game::m_showCameraPosition = true;
	bool Game::m_picked = false;
	bool Game::m_showDiagnostics = false;
	int Game::m_selectedEntity = 0;
	std::vector<char> Game::m_nameChanger;
	glm::vec3 Game::m_rotationAngles;
	glm::vec3 Game::m_position;
	glm::vec3 Game::m_scale;
	glm::vec3 Game::m_color;
	std::string Game::m_pickedName;
	std::unique_ptr<Scene> Game::m_scene;

	Game::Game() : m_frameTime(0.f), m_cubeCreationCounter(0)
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

		//Callbacks
		glfwSetFramebufferSizeCallback(m_window, OnFrameBufferResizeCallback);
		glfwSetCursorPosCallback(m_window, OnMouseCallback);

		//ImGUI initialize
		InitImGuiStyle(true, 0.9f);
		ImGui_ImplGlfwGL3_Init(m_window, true);

		//Override imgui mouse button callback
		glfwSetMouseButtonCallback(m_window, OnMouseButtonCallback);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);

		InitScene();

		//Lua functions
		gameConsole.lua.set_function("setCamera", [](float x, float y, float z) { m_scene->GetCamera()->SetPosition(glm::vec3(x, y, z)); });
		gameConsole.lua.set_function("print", [] { gameConsole.AddLog("Printed"); });
	}

	Game::~Game()
	{
		m_scene->WriteSceneData();
		m_scene->DestroyScene();

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

		m_scene = std::make_unique<Scene>();
		m_scene->LoadScene(m_models);
		m_frameBuffer = std::make_unique<RenderTexture>();
		m_grid = std::make_unique<Grid>(m_scene->GetCamera());

		//Lightning
		m_lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f); m_ambient = 0.3f; m_specular = 0.2f;
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
		Shader::SetMatrix4x4(Shaders::Phong, "projection", m_scene->GetCamera()->GetProjectionMatrix());
		Shader::SetMatrix4x4(Shaders::Phong, "view", m_scene->GetCamera()->GetViewMatrix());
		Shader::SetFloat3v(Shaders::Phong, "viewpos", m_scene->GetCamera()->GetPosition());
		Shader::SetFloat3v(Shaders::Phong, "direction", m_lightDirection);
		Shader::SetFloat(Shaders::Phong, "ambientStrength", m_ambient);
		Shader::SetFloat(Shaders::Phong, "specularStrength", m_specular);

		//Update systems
		m_scene->UpdateSystems(dt);
		
		m_frameBuffer->BlitMultiSampledBuffer();
		m_frameBuffer->UnbindFrameBuffer();
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Game::Update(float dt)
	{
		//Consider using a struct object as parameter instead?
		m_scene->UpdatePickedEntity(m_pickedName, m_position, m_rotationAngles, m_scale, m_color, m_picked);

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
			m_scene->GetCamera()->SetWidth((unsigned int)size.x);
			m_scene->GetCamera()->SetHeight((unsigned int)size.y);
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
				if (ImGui::MenuItem("Quit", "Escape")) { glfwSetWindowShouldClose(m_window, true); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Show Grid", NULL, &m_showGrid)) {}
				if (ImGui::MenuItem("Show FPS", NULL, &m_showFPS)) {}
				if (ImGui::MenuItem("Show Position", NULL, &m_showCameraPosition)) {}
				if (ImGui::MenuItem("Show Diagnostics", NULL, &m_showDiagnostics)) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::BeginMenu("3D Object"))
				{
					if(ImGui::MenuItem("Cube"))
					{
						std::string name = "Cube" + std::to_string(m_cubeCreationCounter);

						ComponentHandle<Transformable> transform;
						ComponentHandle<Renderable> renderable;

						for (Entity & entity : m_scene->GetEntities().entities_with_components(transform, renderable))
						{
							if (name != renderable->object->GetName())
								name = name;
							else
							{
								m_cubeCreationCounter++;
								name = "Cube" + std::to_string(m_cubeCreationCounter);
							}
						}

						m_scene->CreateEntity(m_models, Models::Cube, name);
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		//Open popup if delete is pressed && picked
		if (m_picked && glfwGetKey(m_window, GLFW_KEY_DELETE) == GLFW_PRESS)
		{
			ImGui::OpenPopup("Delete?");
		}

		//Remove the picked entity if the user agrees
		if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("Are you sure you want to delete this item?\nThis action can't be undone.\n\n");
			ImGui::Separator();

			if (ImGui::Button("Yes", ImVec2(120, 0))) 
			{ 
				m_scene->DestroyEntity(m_pickedName);
				m_cubeCreationCounter = 0;
				m_picked = false;
				ImGui::CloseCurrentPopup(); 
			}
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
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

			ImGui::Text("(%.3f, %.3f, %.3f)      ", m_scene->GetCamera()->GetPosition().x, m_scene->GetCamera()->GetPosition().y, m_scene->GetCamera()->GetPosition().z);
			ImGui::End();		
		}
		
		//Diagnostics overlay on left of the screen
		if (m_showDiagnostics)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 1370, WINDOW_HEIGHT - 840));
			if (!ImGui::Begin("Diagnostics overlay", &m_showDiagnostics, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
			{
				ImGui::End();
				return;
			}

			ImGui::TextColored(ImVec4(0.f, 1.0f, 0.0f, 1.0f), "OpenGL context:\nVersion: %s\nGLSL Version: %s\nVendor: %s\nRenderer: %s\n",
				glGetString(GL_VERSION),
				glGetString(GL_SHADING_LANGUAGE_VERSION),
				glGetString(GL_VENDOR),
				glGetString(GL_RENDERER)
			);

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
					//TODO: make sure that entities can't have the same name!
					//Change name of entity upon completion
					if (ImGui::InputText("Name", m_nameChanger.data(), m_nameChanger.size(), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						m_scene->ChangeEntityName(m_pickedName, m_nameChanger.data());
					}
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
					ImGui::Spacing();

					ImGui::SetNextTreeNodeOpen(true, 2);
					if (ImGui::CollapsingHeader("Material"))
					{
						ImGui::Spacing();
						ImGui::ColorEdit3("Color", &m_color[0]);
					}
				}
			}
			ImGui::EndDock();
			
			ImGui::SetNextDock(ImGuiDockSlot_Left);
			if (ImGui::BeginDock("Hierarchy"))
			{
				ImGui::BeginChild("Entities");

				unsigned int i = 0;
				ComponentHandle<Transformable> transform;
				ComponentHandle<Renderable> renderable;

				for (Entity & entity : m_scene->GetEntities().entities_with_components(transform, renderable))
				{
					char label[128];
					sprintf(label, renderable->object->GetName().c_str());
					if (ImGui::Selectable(label, m_selectedEntity == i))
					{
						//Give information to GUI about picked object
						m_pickedName = renderable->object->GetName();
						m_color = renderable->object->GetColor();

						//Copy the name to the char vector
						m_nameChanger.clear(); m_nameChanger.resize(50);
						for (unsigned int p = 0; p < m_pickedName.size(); p++)
							m_nameChanger[p] = m_pickedName[p];

						//Give information to GUI about picked object
						m_scale = transform->transform->GetScale();
						m_position = transform->transform->GetPosition();
						m_rotationAngles = transform->transform->GetRotationAngles();
						m_selectedEntity = i;
						m_picked = true;
					}
					i++;
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

			ImGui::SetNextDock(ImGuiDockSlot_Tab);
			if (ImGui::BeginDock("Assets"))
			{
				if (ImGui::TreeNode("Materials")) 
				{
					if (ImGui::TreeNode("Material001"))
					{
						glm::vec3 color = glm::vec3(0.5f, 0.f, 0.f);
						ImGui::ColorEdit3("Color", &color[0]);
						ImGui::TreePop();
					}
					ImGui::TreePop(); 
				}   
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
			m_scene->GetCamera()->ProcessKeyboard(FORWARD, dt);
		if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
			m_scene->GetCamera()->ProcessKeyboard(BACKWARD, dt);
		if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
			m_scene->GetCamera()->ProcessKeyboard(RIGHT, dt);
		if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
			m_scene->GetCamera()->ProcessKeyboard(LEFT, dt);
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
			if (m_scene->GetCamera()->GetFirstMouse())
			{
				m_lastX = (float)xpos;
				m_lastY = (float)ypos;
			}

			float xoffset = (float)xpos - m_lastX;
			float yoffset = m_lastY - (float)ypos;

			m_lastX = (float)xpos;
			m_lastY = (float)ypos;
			
			m_scene->GetCamera()->ProcessMouseMovement(window, xoffset, yoffset);
		}
	}

	void Game::OnMouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
	{
		if ((button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS))
		{
			//Width formula for new resize -> (window_width - new_width) / 2
			//Height formula (approximation) -> (new_height / 8)

			//Picking: project 2D-position to 3D and check intersection with OBB
			if (m_hovered)
			{
				Picking::PerformMousePicking(m_scene->GetCamera(), m_lastX - 16, m_lastY - 50);

				ComponentHandle<Transformable> transform;
				ComponentHandle<Renderable> renderable;

				unsigned int i = 0;
				for (Entity & entity : m_scene->GetEntities().entities_with_components(transform, renderable))
				{
					if (Picking::RayOBBIntersection(glm::vec3(1.f), transform->transform->GetTransform()))
					{
						//Give information to GUI about picked object
						m_selectedEntity = i;
						m_pickedName = renderable->object->GetName();
						m_color = renderable->object->GetColor();
						m_scale = transform->transform->GetScale();
						m_position = transform->transform->GetPosition();
						m_rotationAngles = transform->transform->GetRotationAngles();
						m_picked = true;

						//Copy the name to the char vector
						m_nameChanger.clear(); m_nameChanger.resize(50);
						for (unsigned int p = 0; p < m_pickedName.size(); p++)
							m_nameChanger[p] = m_pickedName[p];

						gameLog.Print("Picked\n");
						break;
					}
					else
						m_picked = false;

					i++;
				}
			}
		}
	}

	//void Game::OnMouseScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
	//{
	//	float fov = m_scene->GetCamera()->GetFov();

	//	if (m_hovered)
	//	{
	//		if (fov >= 1.0f && fov <= 90.0f)
	//		{
	//			fov -= (float)(5.0 * yoffset);
	//			m_scene->GetCamera()->SetFov(fov);
	//		}

	//		if (fov <= 1.0f)
	//		{
	//			fov = 1.0f;
	//			m_scene->GetCamera()->SetFov(fov);
	//		}

	//		if (fov >= 90.0f)
	//		{
	//			fov = 90.0f;
	//			m_scene->GetCamera()->SetFov(fov);
	//		}
	//	}
	//}
}
