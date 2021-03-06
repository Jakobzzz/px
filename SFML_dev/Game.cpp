#include "Game.hpp"
#include "imguidock.h"
#include "imgui_impl_glfw_gl3.h"
#include "imgui_log.h"
#include "imgui_console.h"
#include "Macros.hpp"

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
	std::vector<Game::Material> Game::m_materials; //Test vector for materials
	std::unique_ptr<Scene> Game::m_scene;
	Game::EntityInformation Game::m_info;
	Game::DisplayInformation Game::m_displayInfo;

	Game::Game() : m_frameTime(0.f), m_creationCounter(0)
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

		//Init some GUI info
		m_info.picked = false;
		m_info.selectedEntity = 0;

		m_displayInfo.hovered = false;
		m_displayInfo.showGrid = true;
		m_displayInfo.showFPS = false;
		m_displayInfo.showCameraPosition = true;
		m_displayInfo.showDiagnostics = false;
		m_displayInfo.showDebugDraw = true;

		//Materials test
		Material material;
		material.name = "Cube"; material.color = glm::vec3(0.5f);
		m_materials.push_back(material);

		material.name = "Trunk"; material.color = glm::vec3(0.1f);
		m_materials.push_back(material);

		material.name = "Leaves"; material.color = glm::vec3(0.7f);
		m_materials.push_back(material);

		for (unsigned int i = 0; i < m_materials.size(); i++)
			m_materialNames.push_back(m_materials[i].name);
	}

	Game::~Game()
	{
		m_scene->WriteSceneData();
		m_scene->DestroyScene();

		//TODO: make function to destroy all loaded models?
		m_models->Destroy(Models::Cube);
		//m_models->Destroy(Models::Capsule);
		m_models->Destroy(Models::Sphere);
		m_models->Destroy(Models::Cylinder);

		Physics::Release();
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}

	void Game::LoadShaders()
	{
		Shader::LoadShaders(Shaders::Phong, "triangle.vertex", "triangle.fragment");
		Shader::LoadShaders(Shaders::Grid, "grid.vertex", "grid.fragment");
		Shader::LoadShaders(Shaders::Debug, "bulletDebug.vertex", "bulletDebug.fragment");
	}

	void Game::LoadModels()
	{
		m_models = std::make_shared<Model<Models::ID>>();

		//Standard models
		m_models->LoadModel(Models::Cube, "../res/Models/Cube/cube.obj");
		m_models->LoadModel(Models::Sphere, "../res/Models/Sphere/sphere.obj");
		m_models->LoadModel(Models::Cylinder, "../res/Models/Cylinder/cylinder.obj");
		//m_models->LoadModel(Models::Capsule, "../res/Models/Capsule/capsule.obj");
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

			Physics::Update();
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

		if (m_displayInfo.showGrid)
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
	
		if(m_displayInfo.showDebugDraw)
			Physics::DrawDebug();
		
		m_frameBuffer->BlitMultiSampledBuffer();
		m_frameBuffer->UnbindFrameBuffer();
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Game::Update(float dt)
	{
		//Consider using a struct object as parameter instead?
		m_scene->UpdatePickedEntity(m_info.pickedName, m_info.position, m_info.rotationAngles, m_info.scale,
									m_info.color, m_info.picked);

		if (m_displayInfo.hovered)
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
		m_displayInfo.hovered = ImGui::IsItemHovered();
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
				if (ImGui::MenuItem("Show Grid", NULL, &m_displayInfo.showGrid)) {}
				if (ImGui::MenuItem("Show FPS", NULL, &m_displayInfo.showFPS)) {}
				if (ImGui::MenuItem("Show Position", NULL, &m_displayInfo.showCameraPosition)) {}
				if (ImGui::MenuItem("Show Diagnostics", NULL, &m_displayInfo.showDiagnostics)) {}
				if (ImGui::MenuItem("Show Debug Shapes", NULL, &m_displayInfo.showDebugDraw)) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("GameObject"))
			{
				if (ImGui::BeginMenu("3D Object"))
				{
					if(ImGui::MenuItem("Cube"))
						m_scene->CreateEntity(m_models, Models::Cube, RigidBodyType::Box, GenerateName("Cube"));

					if (ImGui::MenuItem("Sphere"))
						m_scene->CreateEntity(m_models, Models::Sphere, RigidBodyType::Sphere, GenerateName("Sphere"));

					if (ImGui::MenuItem("Cylinder"))
						m_scene->CreateEntity(m_models, Models::Cylinder, RigidBodyType::Cylinder, GenerateName("Cylinder"));

					/*if (ImGui::MenuItem("Capsule"))
						m_scene->CreateEntity(m_models, Models::Capsule, PickingType::Capsule,  GenerateName("Capsule"));*/
					
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		//static bool delayFrame;
		//delayFrame = true;
		//if (m_info.picked && (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS))
		//{
		//	//Only default components for now
		//	if (delayFrame)
		//	{
		//		//auto entity = m_scene->GetEntityByName(m_info.pickedName);

		//		/*Models::ID id = m_scene->GetEntityByName(m_info.pickedName).component<Renderable>()->object->GetModel();
		//		PickingType::ID type = m_scene->GetEntityByName(m_info.pickedName).component<Pickable>()->object->GetPickingType();
		//		glm::vec3 pos = m_scene->GetEntityByName(m_info.pickedName).component<Transformable>()->transform->GetPosition();
		//		glm::vec3 scale = m_scene->GetEntityByName(m_info.pickedName).component<Transformable>()->transform->GetScale();
		//		glm::vec3 rotation = m_scene->GetEntityByName(m_info.pickedName).component<Transformable>()->transform->GetRotationAngles();

		//		m_scene->CreateEntity(m_models, id, type, GenerateName(m_info.pickedName + "(clone)"));*/


		//		delayFrame = false;
		//	}
		//}

		//Open popup if delete is pressed && picked
		if (m_info.picked && glfwGetKey(m_window, GLFW_KEY_DELETE) == GLFW_PRESS)
		{
			ImGui::OpenPopup("Delete?");
		}

		//Remove the picked entity if the user agrees
		if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("Are you sure you want to delete this item?\nThis action can't be undone.\n\n");
			ImGui::Separator();

			if (ImGui::Button("Yes", ImVec2(120, 0)) || glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS)
			{ 
				m_scene->DestroyEntity(m_info.pickedName);
				m_creationCounter = 0;
				m_info.picked = false;
				ImGui::CloseCurrentPopup(); 
			}
			ImGui::SameLine();

			if (ImGui::Button("No", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

		//Camera position overlay at the bottom of the scene
		if (m_displayInfo.showCameraPosition)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 215, WINDOW_HEIGHT - 480));
			if (!ImGui::Begin("Camera overlay", &m_displayInfo.showCameraPosition, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
																						   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
			{
				ImGui::End();
				return;
			}

			ImGui::Text("(%.3f, %.3f, %.3f)      ", m_scene->GetCamera()->GetPosition().x, m_scene->GetCamera()->GetPosition().y, m_scene->GetCamera()->GetPosition().z);
			ImGui::End();		
		}
		
		//Diagnostics overlay on left of the screen
		if (m_displayInfo.showDiagnostics)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 1370, WINDOW_HEIGHT - 840));
			if (!ImGui::Begin("Diagnostics overlay", &m_displayInfo.showDiagnostics, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
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
		if (m_displayInfo.showFPS)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 230, WINDOW_HEIGHT - 840));
			if (!ImGui::Begin("FPS overlay", &m_displayInfo.showFPS, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
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
				if (m_info.picked)
				{
					//TODO: make sure that entities can't have the same name!
					//Change name of entity upon completion
					if (ImGui::InputText("Name", m_info.nameChanger.data(), m_info.nameChanger.size(), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						m_scene->ChangeEntityName(m_info.pickedName, m_info.nameChanger.data());
					}
					ImGui::Spacing();

					ImGui::SetNextTreeNodeOpen(true, 2);
					if (ImGui::CollapsingHeader("Transform"))
					{
						ImGui::Spacing();
						ImGui::InputFloat3("Position", &m_info.position[0], floatPrecision);
						ImGui::Spacing();
						ImGui::InputFloat3("Rotation", &m_info.rotationAngles[0], floatPrecision);
						ImGui::Spacing();
						ImGui::InputFloat3("Scale", &m_info.scale[0], floatPrecision);
					}
					ImGui::Spacing();
				
					ImGui::SetNextTreeNodeOpen(true, 2);
					if (ImGui::CollapsingHeader("Material"))
					{
						ImGui::Spacing();

						//TODO: Number of combos should depend on the number of materials for a model
						static int item2 = 0;
						ImGui::Combo("1", &item2, m_materialNames.data(), m_materialNames.size()); //Callback function upon picking an item?

						//For now...
						//Assign color from picked material
						m_info.color = m_materials[item2].color;
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
					if (ImGui::Selectable(label, m_info.selectedEntity == i))
					{
						//Give information to GUI about picked object
						m_info.pickedName = renderable->object->GetName();
						m_info.color = renderable->object->GetColor();

						//Copy the name to the char vector
						m_info.nameChanger.clear(); m_info.nameChanger.resize(50);
						for (unsigned int p = 0; p < m_info.pickedName.size(); p++)
							m_info.nameChanger[p] = m_info.pickedName[p];

						//Give information to GUI about picked object
						m_info.scale = transform->transform->GetScale();
						m_info.position = transform->transform->GetPosition();
						m_info.rotationAngles = transform->transform->GetRotationAngles();
						m_info.selectedEntity = i;
						m_info.picked = true;
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
					for (unsigned int i = 0; i < m_materials.size(); i++)
					{
						if (ImGui::TreeNode(m_materials[i].name))
						{
							ImGui::ColorEdit3("Color", &m_materials[i].color[0]);
							ImGui::TreePop();
						}
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

	std::string Game::GenerateName(std::string nameType)
	{
		//Generates a new name based on a type (sphere, cube, etc)
		//This function is not safe as the name can sometimes already be taken
		std::string name = nameType + std::to_string(m_creationCounter);
		ComponentHandle<Renderable> renderable;

		for (Entity & entity : m_scene->GetEntities().entities_with_components(renderable))
		{
			if (name != renderable->object->GetName())
				name = name;
			else
			{
				m_creationCounter++;
				name = nameType + std::to_string(m_creationCounter);
			}
		}

		return name;
	}

	//*** Callbacks ***
	void Game::OnFrameBufferResizeCallback(GLFWwindow * window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void Game::OnMouseCallback(GLFWwindow * window, double xpos, double ypos)
	{
		if (m_displayInfo.hovered)
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
			if (m_displayInfo.hovered)
			{
				Picking::PerformMousePicking(m_scene->GetCamera(), m_lastX - 16, m_lastY - 50);

				ComponentHandle<Transformable> transform;
				ComponentHandle<Renderable> renderable;
				ComponentHandle<Pickable> pickable;

				unsigned int i = 0;
				for (Entity & entity : m_scene->GetEntities().entities_with_components(transform, renderable, pickable))
				{
					if (Picking::RayCast(FAR_PLANE, pickable->object->GetRigidBody()))
					{
						//Give information to GUI about picked object
						m_info.selectedEntity = i;
						m_info.pickedName = renderable->object->GetName();
						m_info.color = renderable->object->GetColor();
						m_info.scale = transform->transform->GetScale();
						m_info.position = transform->transform->GetPosition();
						m_info.rotationAngles = transform->transform->GetRotationAngles();
						m_info.picked = true;

						//Copy the name to the char vector
						m_info.nameChanger.clear(); m_info.nameChanger.resize(50);
						for (unsigned int p = 0; p < m_info.pickedName.size(); p++)
							m_info.nameChanger[p] = m_info.pickedName[p];

						gameLog.Print("Picked\n");
						break;
					}
					else
						m_info.picked = false;

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
