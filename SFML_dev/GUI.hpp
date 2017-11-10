#pragma once

namespace px
{
	namespace gui
	{
		class GUI
		{
		public:
			GUI();
			~GUI();

		public:
			void DisplayMenu();

		public:
			static bool m_showGrid;
			static bool m_showFPS;
			static bool m_showCameraPosition;
		};
	}
}

