#include "GUI.hpp"
#include <imgui.h>

namespace px
{
	namespace gui
	{
		bool GUI::m_showGrid = true;
		bool GUI::m_showFPS = false;
		bool GUI::m_showCameraPosition = true;

		GUI::GUI()
		{
		}

		GUI::~GUI()
		{
		}

		void GUI::DisplayMenu()
		{
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




			}


		}

	}
}
