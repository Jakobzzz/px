#include "Game.hpp"
#include "imguidock.h"
#include "imgui_impl_glfw_gl3.h"

#include <assert.h>
#include <iostream>
#include <functional>

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
	glm::mat4 Game::m_cubeWorld;

	Game::Game() : m_frameTime(0.f), m_entities(m_events), m_systems(m_entities, m_events), m_pickedName("Cube")
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

		//Callbacks should be placed after ImGUI is init to work properly
		//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetFramebufferSizeCallback(m_window, OnFrameBufferResizeCallback);
		glfwSetCursorPosCallback(m_window, OnMouseCallback);

		//ImGUI
		InitImGuiStyle(true, 0.5f);
		ImGui_ImplGlfwGL3_Init(m_window, true);

		//Override imgui mouse button callback
		//No issues so far...
		//glfwSetScrollCallback(m_window, OnMouseScrollCallback); //Use this when the render to texture works?
		glfwSetMouseButtonCallback(m_window, OnMouseButtonCallback);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);

		InitScene();
	}

	Game::~Game()
	{
		for (Entity entity : m_entities.entities_with_components<Transformable, Renderable>())
			entity.destroy();

		/*glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);*/

		m_models->Destroy(Models::Cube);
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}

	void Game::LoadShaders()
	{
		Shader::LoadShaders(Shaders::Phong, "triangle.vertex", "triangle.fragment");
		Shader::LoadShaders(Shaders::Debug, "lineDebug.vertex", "lineDebug.fragment");
		Shader::LoadShaders(Shaders::Grid, "grid.vertex", "grid.fragment");
	}

	void Game::LoadModels()
	{
		m_models = std::make_shared<Model<Models::ID>>();
		m_models->LoadModel(Models::Cube, "../res/Models/Cube/cube.obj"); //Check the mesh vector when more objects are added -> clear() or local vector?
	}

	void Game::InitScene()
	{
		m_camera = std::make_shared<Camera>(glm::vec3(-0.164f, 10.694f, 32.12f));

		LoadShaders();
		LoadModels();

		//Render textures
		m_frameBuffer = std::make_unique<RenderTexture>();

		//Grid
		m_grid = std::make_unique<Grid>(m_camera);

		//Entites
		InitEntities();

		//Lines
		//m_lines.push_back({ glm::vec3(0.f, 0.f, 0.f) }); //Line start
		//m_lines.push_back({ glm::vec3(20.f, 0.f, 0.f) }); //Line end

		//glGenVertexArrays(1, &m_VAO);
		//glGenBuffers(1, &m_VBO);

		//glBindVertexArray(m_VAO);

		//glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		//glBufferData(GL_ARRAY_BUFFER, m_lines.size() * sizeof(LineInfo), &m_lines[0], GL_DYNAMIC_DRAW);

		////Positions
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineInfo), (void*)0);
		//glEnableVertexAttribArray(0);

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);

		//Lightning
		m_lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
		m_ambient = 0.3f;
		m_specular = 0.2f;

		//Colors
		m_colors[0] = 0.274f;
		m_colors[1] = 0.227f;
		m_colors[2] = 0.227f;

		//Systems
		m_systems.add<RenderSystem>();
		m_systems.configure();
	}

	void Game::InitEntities()
	{
		//*** CUBE ENTITY ***
		m_cubeEntity = m_entities.create();

		auto cubeTransform = std::make_unique<Transform>();
		cubeTransform->SetPosition(glm::vec3(0.f, 10.f, 0.f));

		auto cube = std::make_unique<px::Render>(m_models, Models::Cube, Shaders::Phong, "Cube");

		//Init tweaking variables for cube object
		m_scale = cubeTransform->GetScale();
		m_position = cubeTransform->GetPosition();
		m_rotationAngles = cubeTransform->GetRotationAngles();

		m_cubeEntity.assign<Transformable>(cubeTransform);
		m_cubeEntity.assign<Renderable>(cube);

		//*** PLANE ENTITY ***
		m_planeEntity = m_entities.create();

		auto planeTransform = std::make_unique<Transform>();
		planeTransform->SetScale(glm::vec3(10.f, 0.1f, 10.f));

		auto plane = std::make_unique<px::Render>(m_models, Models::Cube, Shaders::Phong, "Plane");

		m_planeEntity.assign<Transformable>(planeTransform);
		m_planeEntity.assign<Renderable>(plane);
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
		glClearColor(m_colors[0], m_colors[1], m_colors[2], 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(m_showGrid)
			m_grid->Draw(Shaders::Grid);

		//VP matrices + Phong Shading
		Shader::Use(Shaders::Phong);
		Shader::SetMatrix4x4(Shaders::Phong, "projection", m_camera->GetProjectionMatrix());
		Shader::SetMatrix4x4(Shaders::Phong, "view", m_camera->GetViewMatrix());
		Shader::SetFloat3v(Shaders::Phong, "viewpos", m_camera->GetPosition());
		Shader::SetFloat3v(Shaders::Phong, "direction", m_lightDirection);
		Shader::SetFloat(Shaders::Phong, "ambientStrength", m_ambient);
		Shader::SetFloat(Shaders::Phong, "specularStrength", m_specular);

		//Update systems
		m_systems.update<RenderSystem>(dt);

		//Render picking line
		//Shader::Use(Shaders::Debug);

		//glm::vec3 startPos = glm::vec3(-0.164f, 10.694f, 32.12f);
		//m_lines[0].position = startPos;
		//m_lines[1].position = startPos + Picking::GetPickingRay() * FAR_PLANE;

		//glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		//glBufferData(GL_ARRAY_BUFFER, m_lines.size() * sizeof(LineInfo), &m_lines[0], GL_DYNAMIC_DRAW);

		////Positions
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineInfo), (void*)0);
		//glEnableVertexAttribArray(0);

		//Shader::SetMatrix4x4(Shaders::Debug, "model", glm::mat4());
		//Shader::SetMatrix4x4(Shaders::Debug, "projection", m_camera->GetProjectionMatrix());
		//Shader::SetMatrix4x4(Shaders::Debug, "view", m_camera->GetViewMatrix());

		//glBindVertexArray(m_VAO);
		//glDrawArrays(GL_LINES, 0, m_lines.size());
		//glBindVertexArray(0);
		
		m_frameBuffer->BlitMultiSampledBuffer();
		m_frameBuffer->UnbindFrameBuffer();
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Game::Update(float dt)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;

		//Perform OBB intersection test with picking ray
		/*for (Entity entity : m_entities.entities_with_components(transform, renderable))
		{
			m_picked = Picking::RayOBBIntersection(glm::vec3(-1.f), glm::vec3(1.f), transform->transform->GetTransform());

			if (m_picked)
				std::cout << "Intersected " << renderable->object->GetName() << std::endl;
			else
				std::cout << "Didn't intersect" << std::endl;
		}*/

		//Check which entity was picked
		//for (Entity entity : m_entities.entities_with_components(transform, renderable))
		//{
		//	if (m_pickedBody == rigidBody->body->GetRigidBody().get())
		//	{
		//		m_pickedName = renderable->object->GetName();
		//		std::cout << renderable->object->GetName() << std::endl;

		//		//Give information to GUI about object
		//		m_scale = transform->transform->GetScale();
		//		m_position = transform->transform->GetPosition();
		//		m_rotationAngles = transform->transform->GetRotationAngles();

		//		m_rigidbodySize = rigidBody->body->GetSize();
		//	}
		//}

		//Set transformation for the picked object
		for (Entity entity : m_entities.entities_with_components(transform, renderable))
		{
			if (m_pickedName == renderable->object->GetName())
			{
				//Object
				transform->transform->SetPosition(m_position);
				transform->transform->SetRotationOnAllAxis(m_rotationAngles);
				transform->transform->SetScale(m_scale);
			}
			else
				transform->transform->SetTransform(); //Apply transform to object to prevent world matrix from identity
		}

		ComponentHandle<Transformable> cubeTransform = m_cubeEntity.component<Transformable>();
		m_cubeWorld = cubeTransform->transform->GetTransform();

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

			ImGui::EndMainMenuBar();
		}

		//Camera position overlay at the bottom of the scene
		if (m_showCameraPosition)
		{
			ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 215, WINDOW_HEIGHT - 480));
			if (!ImGui::Begin("Camera overlay", &m_showCameraPosition, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
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

		//Docking tabs
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
				/*ImGui::Spacing();
				ImGui::ColorEdit3("Color", m_colors);*/
				
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
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Bottom);
			if (ImGui::BeginDock("Log"))
			{

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
			//Height formula (this is not absolute)
			//(new_height / 8)

			//Picking: project 2D-position to 3D
			if (m_hovered)
			{
				Picking::PerformMousePicking(m_camera, m_lastX - 16, m_lastY - 50);
				m_picked = Picking::RayOBBIntersection(glm::vec3(-1.f), glm::vec3(1.f), m_cubeWorld);

				if (m_picked)
					std::cout << "YES" << std::endl;
				else
					std::cout << "NO" << std::endl;
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
